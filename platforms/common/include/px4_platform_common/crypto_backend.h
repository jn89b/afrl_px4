/****************************************************************************
 *
 *   Copyright (c) 2021 Technology Innovation Institute. All rights reserved.
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

#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <px4_platform_common/crypto_algorithms.h>
#include "crypto_backend_definitions.h"

/*
 * Keystore access functions
 */

void keystore_init(void);

/*
 * Open a session for accessing security keys
 */

keystore_session_handle_t keystore_open(void);

/*
 * Close the keystore session
 * handle: handle to the session to be closed
 */

void keystore_close(keystore_session_handle_t *handle);

/*
 * Get a key from keystore
 * handle: a handle to an open keystore
 * idx: key index in keystore
 * key_buf: output buffer for the key
 * key_buf_size: size of the provided output buffer
 * returns the size of the key in keystore
 */
size_t keystore_get_key(keystore_session_handle_t handle, uint8_t idx, uint8_t *key_buf, size_t key_buf_size);

/*
 * Architecture specific PX4 Crypto API functions
 */

/*
 * Initialize hw level crypto
 * This has to be called before any other crypto operations
 */

void crypto_init(void);

/*
 * Open a session for performing crypto functions
 * algorithm: The crypto algorithm to be used in this session
 */

crypto_session_handle_t crypto_open(px4_crypto_algorithm_t algorithm);

/*
 * Close the session for performing crypto operations
 * handle: handle to the session to be closed
 */

void crypto_close(crypto_session_handle_t *handle);

/*
 * Perform signature check using an open session to crypto
 * handle: session handle, returned by open
 * key_index: index to the key used for signature check
 * message: pointer to the data to be checked
 * message_size: size of the data
 */

bool crypto_signature_check(crypto_session_handle_t handle,
			    uint8_t  key_index,
			    const uint8_t  *signature,
			    const uint8_t *message,
			    size_t message_size);

#if defined(__cplusplus)
} // extern "C"
#endif
