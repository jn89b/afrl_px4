/****************************************************************************
 *
 *   Copyright (c) 2012-2015 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file param.c
 *
 * Global parameter store.
 *
 * The implementation utilizes 2 arrays: a constant, auto-generated array
 * with all parameters (param_info_base) and a dynamically resized array
 * with non-default values (param_values). Both of them are sorted.
 */

//#include <debug.h>
#include <px4_defines.h>
#include <px4_posix.h>
#include <px4_config.h>
#include <px4_spi.h>
#include <string.h>
#include <stdbool.h>
#include <float.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <systemlib/err.h>
#include <errno.h>
#include <px4_atomic.h>
#include <px4_sem.h>
#include <math.h>

#include <sys/stat.h>

#include <drivers/drv_hrt.h>

#include "systemlib/param/param.h"
#include "systemlib/uthash/utarray.h"
#include "systemlib/bson/tinybson.h"

#if !defined(PARAM_NO_ORB)
# include "uORB/uORB.h"
# include "uORB/topics/parameter_update.h"
#endif

#if !defined(FLASH_BASED_PARAMS)
#  define FLASH_PARAMS_EXPOSE
#else
#  include "systemlib/flashparams/flashparams.h"
#endif

#include "px4_parameters.h"
#include <crc32.h>


#if 0
# define debug(fmt, args...)		do { warnx(fmt, ##args); } while(0)
#else
# define debug(fmt, args...)		do { } while(0)
#endif

#ifdef __PX4_QURT
#define PARAM_OPEN	px4_open
#define PARAM_CLOSE	px4_close
#else
#define PARAM_OPEN	open
#define PARAM_CLOSE	close
#endif

/**
 * Array of static parameter info.
 */
#ifdef _UNIT_TEST
extern struct param_info_s	param_array[];
extern struct param_info_s	*param_info_base;
extern struct param_info_s	*param_info_limit;
#define param_info_count	(param_info_limit - param_info_base)
#else
static const struct param_info_s *param_info_base = (const struct param_info_s *) &px4_parameters;
#define	param_info_count		px4_parameters.param_count
#endif /* _UNIT_TEST */

/**
 * Storage for modified parameters.
 */
struct param_wbuf_s {
	union param_value_u	val;
	param_t			param;
	bool			unsaved;
};


uint8_t  *param_changed_storage = NULL;
int size_param_changed_storage_bytes = 0;
const int bits_per_allocation_unit  = (sizeof(*param_changed_storage) * 8);


static inline unsigned
get_param_info_count(void)
{
	if (!param_changed_storage) { // there was an allocation failure
		return 0;
	}

	return param_info_count;
}

/** flexible array holding modified parameter values. The first element is only used for
 * the array size (stored in param). This simplifies atomic updates.
 */
//FLASH_PARAMS_EXPOSE UT_array        *param_values; // TODO: change...
FLASH_PARAMS_EXPOSE atomic_ptr param_values = (atomic_ptr)NULL; ///< type is struct param_wbuf_s *


#if !defined(PARAM_NO_ORB)
/** parameter update topic handle */
static orb_advert_t param_topic = NULL;
#endif

static void param_set_used_internal(param_t param);

static param_t param_find_internal(const char *name, bool notification);

/* parameter locking uses RCU: readers don't need to lock, they just increase an atomic counter
 * for the duration of the access, which makes reading wait-free and thus very efficient.
 * Writers use a semaphore to protect against concurrent writes. At the same time they need
 * to ensure that a reader sees a consistent state at **all** times (to simplify this, there
 * is only a single pointer that is updated (param_values) which is visible to the reader).
 */
static px4_sem_t
param_sem_writer; ///< this protects against concurrent write access to param_values and param import/export
static atomic_int param_reader_counter; ///< atomic counter which a reader increases during a read access.
///< a non-zero counter signals a writer that the param_values array is still in use

/** lock the parameter store (write access) */
static void
param_lock_writer(void)
{
	do {} while (px4_sem_wait(&param_sem_writer) != 0);
}

/** unlock the parameter store (write access) */
static void
param_unlock_writer(void)
{
	px4_sem_post(&param_sem_writer);
}

/** assert that the parameter store is held by a reader */
static void
param_assert_locked_reader(void)
{
	// TODO: debug only
	int reader_counter = atomic_int_load(&param_reader_counter);

	if (reader_counter <= 0) {
		//PX4_ERR("wrong reader counter!");
		// param show -c triggers this!
	}
}

void
param_init(void)
{
	px4_sem_init(&param_sem_writer, 0, 1);
	atomic_int_store(&param_reader_counter, 0);

	/* Singleton creation of an array of bits to track changed values */
	size_param_changed_storage_bytes  = (param_info_count / bits_per_allocation_unit) + 1;
	param_changed_storage = calloc(size_param_changed_storage_bytes, 1);
}

/**
 * Test whether a param_t is value.
 *
 * @param param			The parameter handle to test.
 * @return			True if the handle is valid.
 */
static inline bool
handle_in_range(param_t param)
{
	unsigned count = get_param_info_count();
	return (count && param < count);
}

/**
 * Compare two modifid parameter structures to determine ordering.
 *
 * This function is suitable for passing to qsort or bsearch.
 */
static int
param_compare_values(const void *a, const void *b)
{
	struct param_wbuf_s *pa = (struct param_wbuf_s *)a;
	struct param_wbuf_s *pb = (struct param_wbuf_s *)b;

	if (pa->param < pb->param) {
		return -1;
	}

	if (pa->param > pb->param) {
		return 1;
	}

	return 0;
}

/**
 * Locate the modified parameter structure for a parameter, if it exists.
 *
 * @param param			The parameter being searched.
 * @return			The structure holding the modified value, or
 *				NULL if the parameter has not been modified.
 */
static struct param_wbuf_s *
param_find_changed(param_t param)
{
	struct param_wbuf_s	*s = NULL;

	param_assert_locked_reader();

	struct param_wbuf_s *param_values_ptr = (struct param_wbuf_s *)atomic_ptr_load(&param_values);

	if (param_values_ptr != NULL) {
		struct param_wbuf_s key;
		key.param = param;
		s = bsearch(&key, param_values_ptr + 1, param_values_ptr[0].param, sizeof(struct param_wbuf_s), param_compare_values);
	}

	return s;
}

static void
_param_notify_changes(bool is_saved)
{
#if !defined(PARAM_NO_ORB)
	struct parameter_update_s pup = {
		.timestamp = hrt_absolute_time(),
		.saved = is_saved
	};

	/*
	 * If we don't have a handle to our topic, create one now; otherwise
	 * just publish.
	 *
	 * We have a race condition here: it can happen that we call orb_advertise multiple times.
	 * But it will return the same handle, thus we can live with it.
	 */
	if (param_topic == NULL) {
		param_topic = orb_advertise(ORB_ID(parameter_update), &pup);

	} else {
		orb_publish(ORB_ID(parameter_update), param_topic, &pup);
	}

#endif
}

void
param_notify_changes(void)
{
	_param_notify_changes(true);
}

param_t
param_find_internal(const char *name, bool notification)
{
	param_t middle;
	param_t front = 0;
	param_t last = get_param_info_count();

	/* perform a binary search of the known parameters */

	while (front <= last) {
		middle = front + (last - front) / 2;
		int ret = strcmp(name, param_info_base[middle].name);

		if (ret == 0) {
			if (notification) {
				param_set_used_internal(middle);
			}

			return middle;

		} else if (middle == front) {
			/* An end point has been hit, but there has been no match */
			break;

		} else if (ret < 0) {
			last = middle;

		} else {
			front = middle;
		}
	}

	/* not found */
	return PARAM_INVALID;
}

param_t
param_find(const char *name)
{
	return param_find_internal(name, true);
}

param_t
param_find_no_notification(const char *name)
{
	return param_find_internal(name, false);
}

unsigned
param_count(void)
{
	return get_param_info_count();
}

unsigned
param_count_used(void)
{
	unsigned count = 0;

	// ensure the allocation has been done
	if (get_param_info_count()) {

		for (unsigned i = 0; i < size_param_changed_storage_bytes; i++) {
			for (unsigned j = 0; j < bits_per_allocation_unit; j++) {
				if (param_changed_storage[i] & (1 << j)) {
					count++;
				}
			}
		}
	}

	return count;
}

param_t
param_for_index(unsigned index)
{
	unsigned count = get_param_info_count();

	if (count && index < count) {
		return (param_t)index;
	}

	return PARAM_INVALID;
}

param_t
param_for_used_index(unsigned index)
{
	int count = get_param_info_count();

	if (count && index < count) {
		/* walk all params and count used params */
		unsigned used_count = 0;

		for (unsigned i = 0; i < (unsigned)size_param_changed_storage_bytes; i++) {
			for (unsigned j = 0; j < bits_per_allocation_unit; j++) {
				if (param_changed_storage[i] & (1 << j)) {

					/* we found the right used count,
					 * return the param value
					 */
					if (index == used_count) {
						return (param_t)(i * bits_per_allocation_unit + j);
					}

					used_count++;
				}
			}
		}
	}

	return PARAM_INVALID;
}

int
param_get_index(param_t param)
{
	if (handle_in_range(param)) {
		return (unsigned)param;
	}

	return -1;
}

int
param_get_used_index(param_t param)
{
	/* this tests for out of bounds and does a constant time lookup */
	if (!param_used(param)) {
		return -1;
	}

	/* walk all params and count, now knowing that it has a valid index */
	int used_count = 0;

	for (unsigned i = 0; i < (unsigned)size_param_changed_storage_bytes; i++) {
		for (unsigned j = 0; j < bits_per_allocation_unit; j++) {
			if (param_changed_storage[i] & (1 << j)) {

				if ((unsigned)param == i * bits_per_allocation_unit + j) {
					return used_count;
				}

				used_count++;
			}
		}
	}

	return -1;
}

const char *
param_name(param_t param)
{
	return handle_in_range(param) ? param_info_base[param].name : NULL;
}

bool
param_value_is_default(param_t param)
{
	struct param_wbuf_s *s;
	atomic_int_fetch_and_add(&param_reader_counter, 1);
	s = param_find_changed(param);
	atomic_int_fetch_and_sub(&param_reader_counter, 1);
	return s ? false : true;
}

bool
param_value_unsaved(param_t param)
{
	struct param_wbuf_s *s;
	atomic_int_fetch_and_add(&param_reader_counter, 1);
	s = param_find_changed(param);
	bool ret = s && s->unsaved;
	atomic_int_fetch_and_sub(&param_reader_counter, 1);
	return ret;
}

enum param_type_e
param_type(param_t param) {
	return handle_in_range(param) ? param_info_base[param].type : PARAM_TYPE_UNKNOWN;
}

size_t
param_size(param_t param)
{
	if (handle_in_range(param)) {

		switch (param_type(param)) {

		case PARAM_TYPE_INT32:
		case PARAM_TYPE_FLOAT:
			return 4;

		case PARAM_TYPE_STRUCT ... PARAM_TYPE_STRUCT_MAX:
			/* decode structure size from type value */
			return param_type(param) - PARAM_TYPE_STRUCT;

		default:
			return 0;
		}
	}

	return 0;
}


/**
 * Obtain a pointer to the storage allocated for a parameter.
 *
 * @param param			The parameter whose storage is sought.
 * @return			A pointer to the parameter value, or NULL
 *				if the parameter does not exist.
 */
static const void *
param_get_value_ptr(param_t param)
{
	const void *result = NULL;

	param_assert_locked_reader();

	if (handle_in_range(param)) {

		const union param_value_u *v;

		/* work out whether we're fetching the default or a written value */
		struct param_wbuf_s *s = param_find_changed(param);

		if (s != NULL) {
			v = &s->val;

		} else {
			v = &param_info_base[param].val;
		}

		if (param_type(param) >= PARAM_TYPE_STRUCT &&
		    param_type(param) <= PARAM_TYPE_STRUCT_MAX) {

			result = v->p;

		} else {
			result = v;
		}
	}

	return result;
}

int
param_get(param_t param, void *val)
{
	int result = -1;

	atomic_int_fetch_and_add(&param_reader_counter, 1);

	const void *v = param_get_value_ptr(param);

	if (val && v) {
		memcpy(val, v, param_size(param));
		result = 0;
	}

	atomic_int_fetch_and_sub(&param_reader_counter, 1);

	return result;
}

static int
param_set_internal(param_t param, const void *val, bool mark_saved, bool notify_changes, bool is_saved)
{
	int result = -1;
	bool params_changed = false;

	param_lock_writer();

	if (handle_in_range(param)) {

		atomic_int_fetch_and_add(&param_reader_counter, 1); // only needed for the assertion...
		struct param_wbuf_s *s = param_find_changed(param);
		atomic_int_fetch_and_sub(&param_reader_counter, 1);

		if (s == NULL) {

			params_changed = true;

			// the following is tricky: we need to insert a new element into a sorted array,
			// and guarantee that readers see a consistent state at all times.
			// This is where RCU comes into play: we do that by making a copy of the array,
			// update the copy, then atomically replace the array. Finally we wait until we
			// are sure that no reader accesses the previous array, so that it's safe to delete it.

			struct param_wbuf_s *param_values_ptr = (struct param_wbuf_s *)atomic_ptr_load(&param_values);
			int prev_param_count = 0;

			if (param_values_ptr) {
				prev_param_count = param_values_ptr[0].param;
			}

			struct param_wbuf_s *new_param_values = (struct param_wbuf_s *)malloc(sizeof(struct param_wbuf_s) *
								(prev_param_count + 2));

			if (new_param_values == NULL) {
				PX4_ERR("alloc failed");
				goto out;
			}

			if (param_values_ptr) {
				memcpy(new_param_values, param_values_ptr, sizeof(struct param_wbuf_s) * (prev_param_count + 1));
			}

			new_param_values[0].param = prev_param_count + 1; // update the count

			// add the new element
			int new_element_index = prev_param_count + 1;
			new_param_values[new_element_index].param = param;
			new_param_values[new_element_index].val.p = NULL;
			new_param_values[new_element_index].unsaved = false;

			// move it to the correct place by swapping neighbors (the rest of the array is already sorted)
			while (new_element_index > 1 &&
			       param_compare_values(&new_param_values[new_element_index - 1], &new_param_values[new_element_index]) == 1) {
				struct param_wbuf_s tmp;
				memcpy(&tmp, &new_param_values[new_element_index], sizeof(struct param_wbuf_s));
				memcpy(&new_param_values[new_element_index], &new_param_values[new_element_index - 1], sizeof(struct param_wbuf_s));
				memcpy(&new_param_values[new_element_index - 1], &tmp, sizeof(struct param_wbuf_s));
				// TODO: only a single memcpy (keep separate new element buffer. test with param set CBRK_BUZZER ...
				--new_element_index;
			}

			s = &new_param_values[new_element_index];

#if 1 // debug: check correctness of ordering

			for (int test_index = 0; test_index < new_param_values[0].param - 1; ++test_index) {
				if (new_param_values[test_index + 1].param >= new_param_values[test_index + 2].param) {
					PX4_ERR("wrong order (%i %i)", (int)new_param_values[test_index + 1].param,
						(int)new_param_values[test_index + 2].param);
				}
			}

			if (s->param != param) {
				PX4_ERR("s is set wrong");
			}

#endif


			/* add it to the array and sort */
//			utarray_push_back(param_values, &buf);
//			utarray_sort(param_values, param_compare_values); // inefficient: N log N
			// qsort((param_values)->d = base pointer, (param_values)->i = num elements, sizeof(struct param_wbuf_s), param_compare_values);

			/* find it after sorting */
//			s = param_find_changed(param); //inefficient too...



			// the order is important: first apply the new parameter array to make it visible to new readers.
			// then wait until there is no reader, which means it's safe to delete the previous array.
			atomic_ptr_store(&param_values, new_param_values);

			if (param_values_ptr) {
				while (atomic_int_load(&param_reader_counter) > 0) {
					// usually we don't get here, since reader accesses are quick and do not happen that often.
					usleep(1);
				}

				free(param_values_ptr);
			}

		}

		/* update the changed value */
		switch (param_type(param)) {

		case PARAM_TYPE_INT32:
			params_changed = params_changed || s->val.i != *(int32_t *)val;
			s->val.i = *(int32_t *)val; // TODO: this should be atomic (atomic read above too)
			/// -> change the type
			// http://stackoverflow.com/questions/35226128/are-c-c-fundamental-types-atomic
			break;

		case PARAM_TYPE_FLOAT:
			params_changed = params_changed || fabsf(s->val.f - * (float *)val) > FLT_EPSILON;
			s->val.f = *(float *)val;
			// need assert: sizeof(float) == sizeof(int32) ?
			break;

		case PARAM_TYPE_STRUCT ... PARAM_TYPE_STRUCT_MAX:
			if (s->val.p == NULL) {
				s->val.p = malloc(param_size(param));

				if (s->val.p == NULL) {
					debug("failed to allocate parameter storage");
					goto out;
				}
			}

			// FIXME: this update is not atomic. we could do the memcpy into a separate buffer and then
			// atomically replace the pointer. But since PARAM_TYPE_STRUCT is not used currently, we leave
			// it as is.
			memcpy(s->val.p, val, param_size(param));
			params_changed = true;
			break;

		default:
			goto out;
		}

		s->unsaved = !mark_saved;
		result = 0;
	}

out:
	param_unlock_writer();

	/*
	 * If we set something, now that we have unlocked, go ahead and advertise that
	 * a thing has been set.
	 */
	if (params_changed && notify_changes) {
		_param_notify_changes(is_saved);
	}

	return result;
}

#if defined(FLASH_BASED_PARAMS)
int param_set_external(param_t param, const void *val, bool mark_saved, bool notify_changes, bool is_saved)
{
	return param_set_internal(param, val, mark_saved, notify_changes, is_saved);
}

const void *param_get_value_ptr_external(param_t param)
{
	return param_get_value_ptr(param);
}
#endif

int
param_set(param_t param, const void *val)
{
	return param_set_internal(param, val, false, true, false);
}

int
param_set_no_autosave(param_t param, const void *val)
{
	return param_set_internal(param, val, false, true, true);
}

int
param_set_no_notification(param_t param, const void *val)
{
	return param_set_internal(param, val, false, false, false);
}

bool
param_used(param_t param)
{
	int param_index = param_get_index(param);

	if (param_index < 0) {
		return false;
	}

	return param_changed_storage[param_index / bits_per_allocation_unit] &
	       (1 << param_index % bits_per_allocation_unit);
}

void param_set_used_internal(param_t param)
{
	int param_index = param_get_index(param);

	if (param_index < 0) {
		return;
	}

	// TODO: atomic...
	param_changed_storage[param_index / bits_per_allocation_unit] |=
		(1 << param_index % bits_per_allocation_unit);
}

int
param_reset(param_t param)
{
	struct param_wbuf_s *s = NULL;
	bool param_found = false;

	param_lock_writer();

	if (handle_in_range(param)) {

		/* look for a saved value */
		atomic_int_fetch_and_add(&param_reader_counter, 1); // only needed for the assertion...
		s = param_find_changed(param);
		atomic_int_fetch_and_sub(&param_reader_counter, 1);

		// TODO: RCU
		/* if we found one, erase it */
		if (s != NULL) {
			//int pos = utarray_eltidx(param_values, s);
			//utarray_erase(param_values, pos, 1);
			// -> use memmove
		}

		param_found = true;
	}

	param_unlock_writer();

	if (s != NULL) {
		_param_notify_changes(false);
	}

	return (!param_found);
}

void
param_reset_all(void)
{
	param_lock_writer();

	struct param_wbuf_s *param_values_ptr = (struct param_wbuf_s *)atomic_ptr_load(&param_values);

	if (param_values_ptr != NULL) {
		atomic_ptr_store(&param_values, NULL);

		// TODO: create a method...
		while (atomic_int_load(&param_reader_counter) > 0) {
			// usually we don't get here, since reader accesses are quick and do not happen that often.
			usleep(1);
		}

		free(param_values_ptr);
	}

	param_unlock_writer();

	_param_notify_changes(false);
}

void
param_reset_excludes(const char *excludes[], int num_excludes)
{
	param_t	param;

	for (param = 0; handle_in_range(param); param++) {
		const char *name = param_name(param);
		bool exclude = false;

		for (int index = 0; index < num_excludes; index ++) {
			int len = strlen(excludes[index]);

			if ((excludes[index][len - 1] == '*'
			     && strncmp(name, excludes[index], len - 1) == 0)
			    || strcmp(name, excludes[index]) == 0) {
				exclude = true;
				break;
			}
		}

		if (!exclude) {
			param_reset(param);
		}
	}

	_param_notify_changes(false);
}

static const char *param_default_file = PX4_ROOTFSDIR"/eeprom/parameters";
static char *param_user_file = NULL;

int
param_set_default_file(const char *filename)
{
	if (param_user_file != NULL) {
		// we assume this is not in use by some other thread
		free(param_user_file);
		param_user_file = NULL;
	}

	if (filename) {
		param_user_file = strdup(filename);
	}

	return 0;
}

const char *
param_get_default_file(void)
{
	return (param_user_file != NULL) ? param_user_file : param_default_file;
}

int
param_save_default(void)
{
	int res;
#if !defined(FLASH_BASED_PARAMS)
	int fd;

	const char *filename = param_get_default_file();

	/* write parameters to temp file */
	fd = PARAM_OPEN(filename, O_WRONLY | O_CREAT, PX4_O_MODE_666);

	if (fd < 0) {
		warn("failed to open param file: %s", filename);
		return ERROR;
	}

	res = 1;
	int attempts = 5;

	while (res != OK && attempts > 0) {
		res = param_export(fd, false);
		attempts--;
	}

	if (res != OK) {
		warnx("failed to write parameters to file: %s", filename);
	}

	PARAM_CLOSE(fd);
#else
	param_lock_writer();
	res = flash_param_save();
	param_unlock_writer();
#endif
	return res;
}

/**
 * @return 0 on success, 1 if all params have not yet been stored, -1 if device open failed, -2 if writing parameters failed
 */
int
param_load_default(void)
{
	int res = 0;
#if !defined(FLASH_BASED_PARAMS)
	int fd_load = PARAM_OPEN(param_get_default_file(), O_RDONLY);

	if (fd_load < 0) {
		/* no parameter file is OK, otherwise this is an error */
		if (errno != ENOENT) {
			warn("open '%s' for reading failed", param_get_default_file());
			return -1;
		}

		return 1;
	}

	int result = param_load(fd_load);
	PARAM_CLOSE(fd_load);

	if (result != 0) {
		warn("error reading parameters from '%s'", param_get_default_file());
		return -2;
	}

#else
	// no need for locking
	res = flash_param_load();
#endif
	return res;
}

static void
param_bus_lock(bool lock)
{

#if defined (CONFIG_ARCH_BOARD_PX4FMU_V4)

	// FMUv4 has baro and FRAM on the same bus,
	// as this offers on average a 100% silent
	// bus for the baro operation

	// XXX this would be the preferred locking method
	// if (dev == nullptr) {
	// 	dev = px4_spibus_initialize(PX4_SPI_BUS_BARO);
	// }

	// SPI_LOCK(dev, lock);

	// we lock like this for Pixracer for now

	static irqstate_t irq_state = 0;

	if (lock) {
		irq_state = px4_enter_critical_section();

	} else {
		px4_leave_critical_section(irq_state);
	}

#endif
}

int
param_export(int fd, bool only_unsaved)
{
	struct param_wbuf_s *s = NULL;
	struct bson_encoder_s encoder;
	int	result = -1;

	param_lock_writer();

	param_bus_lock(true);
	bson_encoder_init_file(&encoder, fd);
	param_bus_lock(false);

	struct param_wbuf_s *param_values_ptr = (struct param_wbuf_s *)atomic_ptr_load(&param_values);

	/* no modified parameters -> we are done */
	if (param_values_ptr == NULL) {
		result = 0;
		goto out;
	}

	for (int param_idx = 0; param_idx < param_values_ptr[0].param; ++param_idx) {
		s = &param_values_ptr[param_idx + 1];

		int32_t	i;
		float	f;

		/*
		 * If we are only saving values changed since last save, and this
		 * one hasn't, then skip it
		 */
		if (only_unsaved && !s->unsaved) {
			continue;
		}

		s->unsaved = false;

		/* append the appropriate BSON type object */


		switch (param_type(s->param)) {

		case PARAM_TYPE_INT32: {
				i = s->val.i;
				const char *name = param_name(s->param);

				/* lock as short as possible */
				param_bus_lock(true);

				if (bson_encoder_append_int(&encoder, name, i)) {
					param_bus_lock(false);
					debug("BSON append failed for '%s'", name);
					goto out;
				}
			}
			break;

		case PARAM_TYPE_FLOAT: {

				f = s->val.f;
				const char *name = param_name(s->param);

				/* lock as short as possible */
				param_bus_lock(true);

				if (bson_encoder_append_double(&encoder, name, f)) {
					param_bus_lock(false);
					debug("BSON append failed for '%s'", name);
					goto out;
				}
			}
			break;

		case PARAM_TYPE_STRUCT ... PARAM_TYPE_STRUCT_MAX: {

				const char *name = param_name(s->param);
				const size_t size = param_size(s->param);
				const void *value_ptr = param_get_value_ptr(s->param);

				/* lock as short as possible */
				param_bus_lock(true);

				if (bson_encoder_append_binary(&encoder,
							       name,
							       BSON_BIN_BINARY,
							       size,
							       value_ptr)) {
					param_bus_lock(false);
					debug("BSON append failed for '%s'", name);
					goto out;
				}
			}
			break;

		default:
			debug("unrecognized parameter type");
			goto out;
		}

		param_bus_lock(false);

		/* allow this process to be interrupted by another process / thread */
		usleep(5);
	}

	result = 0;

out:
	param_unlock_writer();

	if (result == 0) {
		result = bson_encoder_fini(&encoder);
	}

	return result;
}

struct param_import_state {
	bool mark_saved;
};

static int
param_import_callback(bson_decoder_t decoder, void *private, bson_node_t node)
{
	float f;
	int32_t i;
	void *v, *tmp = NULL;
	int result = -1;
	struct param_import_state *state = (struct param_import_state *)private;

	/*
	 * EOO means the end of the parameter object. (Currently not supporting
	 * nested BSON objects).
	 */
	if (node->type == BSON_EOO) {
		debug("end of parameters");
		return 0;
	}

	/*
	 * Find the parameter this node represents.  If we don't know it,
	 * ignore the node.
	 */
	param_t param = param_find_no_notification(node->name);

	if (param == PARAM_INVALID) {
		debug("ignoring unrecognised parameter '%s'", node->name);
		return 1;
	}

	/*
	 * Handle setting the parameter from the node
	 */

	switch (node->type) {
	case BSON_INT32:
		if (param_type(param) != PARAM_TYPE_INT32) {
			debug("unexpected type for '%s", node->name);
			goto out;
		}

		i = node->i;
		v = &i;
		break;

	case BSON_DOUBLE:
		if (param_type(param) != PARAM_TYPE_FLOAT) {
			debug("unexpected type for '%s", node->name);
			goto out;
		}

		f = node->d;
		v = &f;
		break;

	case BSON_BINDATA:
		if (node->subtype != BSON_BIN_BINARY) {
			debug("unexpected subtype for '%s", node->name);
			goto out;
		}

		if (bson_decoder_data_pending(decoder) != param_size(param)) {
			debug("bad size for '%s'", node->name);
			goto out;
		}

		/* XXX check actual file data size? */
		tmp = malloc(param_size(param));

		if (tmp == NULL) {
			debug("failed allocating for '%s'", node->name);
			goto out;
		}

		if (bson_decoder_copy_data(decoder, tmp)) {
			debug("failed copying data for '%s'", node->name);
			goto out;
		}

		v = tmp;
		break;

	default:
		debug("unrecognised node type");
		goto out;
	}

	if (param_set_internal(param, v, state->mark_saved, true, false)) {
		debug("error setting value for '%s'", node->name);
		goto out;
	}

	if (tmp != NULL) {
		free(tmp);
		tmp = NULL;
	}

	/* don't return zero, that means EOF */
	result = 1;

out:

	if (tmp != NULL) {
		free(tmp);
	}

	return result;
}

static int
param_import_internal(int fd, bool mark_saved)
{
	struct bson_decoder_s decoder;
	int result = -1;
	struct param_import_state state;
	hrt_abstime t = hrt_absolute_time();

	param_bus_lock(true);

	if (bson_decoder_init_file(&decoder, fd, param_import_callback, &state)) {
		debug("decoder init failed");
		param_bus_lock(false);
		goto out;
	}

	param_bus_lock(false);

	state.mark_saved = mark_saved;

	do {
		param_bus_lock(true);
		result = bson_decoder_next(&decoder);
		//usleep(1);
		param_bus_lock(false);

	} while (result > 0);

out:

	PX4_WARN("load took: %i us -- ---------------", (int)hrt_elapsed_time(&t));

	if (result < 0) {
		debug("BSON error decoding parameters");
	}

	return result;
}

int
param_import(int fd)
{
#if !defined(FLASH_BASED_PARAMS)
	return param_import_internal(fd, false);
#else
	(void)fd; // unused
	// no need for locking here
	return flash_param_import();
#endif
}

int
param_load(int fd)
{
	param_reset_all();
	return param_import_internal(fd, true);
}

void
param_foreach(void (*func)(void *arg, param_t param), void *arg, bool only_changed, bool only_used)
{
	param_t	param;

	for (param = 0; handle_in_range(param); param++) {

		/* if requested, skip unchanged values */
		if (only_changed && (param_find_changed(param) == NULL)) {
			continue;
		}

		if (only_used && !param_used(param)) {
			continue;
		}

		func(arg, param);
	}
}

uint32_t param_hash_check(void)
{
	uint32_t param_hash = 0;

	atomic_int_fetch_and_add(&param_reader_counter, 1);

	/* compute the CRC32 over all string param names and 4 byte values */
	for (param_t param = 0; handle_in_range(param); param++) {
		if (!param_used(param)) {
			continue;
		}

		const char *name = param_name(param);
		const void *val = param_get_value_ptr(param);
		param_hash = crc32part((const uint8_t *)name, strlen(name), param_hash);
		param_hash = crc32part(val, param_size(param), param_hash);
	}

	atomic_int_fetch_and_sub(&param_reader_counter, 1);

	return param_hash;
}
