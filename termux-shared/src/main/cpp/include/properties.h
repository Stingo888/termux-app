/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once
#include <sys/cdefs.h>
#include <stddef.h>
#include <stdint.h>
#if __has_include(<sys/system_properties.h>)
#include <sys/system_properties.h>
#else
#define PROP_VALUE_MAX 92
#endif
#ifdef __cplusplus
extern "C" {
#endif
//
// Deprecated.
//
// See <android-base/properties.h> for better API.
//
#define PROPERTY_KEY_MAX PROP_NAME_MAX
#define PROPERTY_VALUE_MAX PROP_VALUE_MAX
/* property_get: returns the length of the value which will never be
** greater than PROPERTY_VALUE_MAX - 1 and will always be zero terminated.
** (the length does not include the terminating zero).
**
** If the property read fails or returns an empty value, the default
** value is used (if nonnull).
*/
int property_get(const char* key, char* value, const char* default_value);
/* property_get_bool: returns the value of key coerced into a
** boolean. If the property is not set, then the default value is returned.
**
* The following is considered to be true (1):
**   "1", "true", "y", "yes", "on"
**
** The following is considered to be false (0):
**   "0", "false", "n", "no", "off"
**
** The conversion is whitespace-sensitive (e.g. " off" will not be false).
**
** If no property with this key is set (or the key is NULL) or the boolean
** conversion fails, the default value is returned.
**/
int8_t property_get_bool(const char *key, int8_t default_value);
/* property_get_int64: returns the value of key truncated and coerced into a
** int64_t. If the property is not set, then the default value is used.
**
** The numeric conversion is identical to strtoimax with the base inferred:
** - All digits up to the first non-digit characters are read
** - The longest consecutive prefix of digits is converted to a long
**
** Valid strings of digits are:
** - An optional sign character + or -
** - An optional prefix indicating the base (otherwise base 10 is assumed)
**   -- 0 prefix is octal
**   -- 0x / 0X prefix is hex
**
** Leading/trailing whitespace is ignored. Overflow/underflow will cause
** numeric conversion to fail.
**
** If no property with this key is set (or the key is NULL) or the numeric
** conversion fails, the default value is returned.
**/
int64_t property_get_int64(const char *key, int64_t default_value);
/* property_get_int32: returns the value of key truncated and coerced into an
** int32_t. If the property is not set, then the default value is used.
**
** The numeric conversion is identical to strtoimax with the base inferred:
** - All digits up to the first non-digit characters are read
** - The longest consecutive prefix of digits is converted to a long
**
** Valid strings of digits are:
** - An optional sign character + or -
** - An optional prefix indicating the base (otherwise base 10 is assumed)
**   -- 0 prefix is octal
**   -- 0x / 0X prefix is hex
**
** Leading/trailing whitespace is ignored. Overflow/underflow will cause
** numeric conversion to fail.
**
** If no property with this key is set (or the key is NULL) or the numeric
** conversion fails, the default value is returned.
**/
int32_t property_get_int32(const char *key, int32_t default_value);
/* property_set: returns 0 on success, < 0 on failure
*/
int property_set(const char *key, const char *value);
int property_list(void (*propfn)(const char *key, const char *value, void *cookie), void *cookie);
#if defined(__BIONIC_FORTIFY)
#define __property_get_err_str "property_get() called with too small of a buffer"
#if defined(__clang__)
/* Some projects use -Weverything; diagnose_if is clang-specific. */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgcc-compat"
int property_get(const char* key, char* value, const char* default_value)
    __clang_error_if(__bos(value) != __BIONIC_FORTIFY_UNKNOWN_SIZE &&
                         __bos(value) < PROPERTY_VALUE_MAX,
                     __property_get_err_str);
#pragma clang diagnostic pop
#else /* defined(__clang__) */
extern int __property_get_real(const char *, char *, const char *)
    __asm__(__USER_LABEL_PREFIX__ "property_get");
__errordecl(__property_get_too_small_error, __property_get_err_str);
__BIONIC_FORTIFY_INLINE
int property_get(const char *key, char *value, const char *default_value) {
    size_t bos = __bos(value);
    if (bos < PROPERTY_VALUE_MAX) {
        __property_get_too_small_error();
    }
    return __property_get_real(key, value, default_value);
}
#endif /* defined(__clang__) */
#undef __property_get_err_str
#endif /* defined(__BIONIC_FORTIFY) */
#ifdef __cplusplus
}
#endif

#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int8_t property_get_bool(const char* key, int8_t default_value) {
    if (!key) return default_value;
    int8_t result = default_value;
    char buf[PROPERTY_VALUE_MAX] = {};
    int len = property_get(key, buf, "");
    if (len == 1) {
        char ch = buf[0];
        if (ch == '0' || ch == 'n') {
            result = false;
        } else if (ch == '1' || ch == 'y') {
            result = true;
        }
    } else if (len > 1) {
        if (!strcmp(buf, "no") || !strcmp(buf, "false") || !strcmp(buf, "off")) {
            result = false;
        } else if (!strcmp(buf, "yes") || !strcmp(buf, "true") || !strcmp(buf, "on")) {
            result = true;
        }
    }
    return result;
}
template <typename T>
static T property_get_int(const char* key, T default_value) {
    if (!key) return default_value;
    char value[PROPERTY_VALUE_MAX] = {};
    if (property_get(key, value, "") < 1) return default_value;
    // libcutils unwisely allows octal, which libbase doesn't.
    T result = default_value;
    int saved_errno = errno;
    errno = 0;
    char* end = nullptr;
    intmax_t v = strtoimax(value, &end, 0);
    if (errno != ERANGE && end != value && v >= std::numeric_limits<T>::min() &&
        v <= std::numeric_limits<T>::max()) {
        result = v;
    }
    errno = saved_errno;
    return result;
}
int64_t property_get_int64(const char* key, int64_t default_value) {
    return property_get_int<int64_t>(key, default_value);
}
int32_t property_get_int32(const char* key, int32_t default_value) {
    return property_get_int<int32_t>(key, default_value);
}
int property_set(const char* key, const char* value) {
    return __system_property_set(key, value);
}
int property_get(const char* key, char* value, const char* default_value) {
    int len = __system_property_get(key, value);
    if (len < 1 && default_value) {
        snprintf(value, PROPERTY_VALUE_MAX, "%s", default_value);
        return strlen(value);
    }
    return len;
}
