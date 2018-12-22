#pragma once

#include <sys/types.h>
#include <time.h>
#include <pthread.h>

#if defined(__PX4_APPLE_LEGACY)
#define clockid_t unsigned
#endif

#if defined(__PX4_QURT)

#include <sys/timespec.h>

__BEGIN_DECLS

int px4_clock_gettime(clockid_t clk_id, struct timespec *tp);
int px4_clock_settime(clockid_t clk_id, struct timespec *tp);

__EXPORT unsigned int sleep(unsigned int sec);

__END_DECLS

#elif defined(__PX4_POSIX)

__BEGIN_DECLS
__EXPORT int px4_clock_gettime(clockid_t clk_id, struct timespec *tp);
__EXPORT int px4_clock_settime(clockid_t clk_id, const struct timespec *tp);

__EXPORT int px4_usleep(useconds_t usec);
__EXPORT unsigned int px4_sleep(unsigned int seconds);
__EXPORT int px4_pthread_cond_timedwait(pthread_cond_t *cond,
					pthread_mutex_t *mutex,
					const struct timespec *abstime);
__END_DECLS

#elif defined(__PX4_NUTTX)

#define px4_clock_gettime system_clock_gettime
#define px4_clock_settime system_clock_settime
#define px4_usleep system_usleep
#define px4_sleep system_sleep

#endif
