/*
 * Copyright (C) 2011 The Android Open Source Project
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

#ifndef TO_STRING_ARRAY_H_included
#define TO_STRING_ARRAY_H_included

//#include "jni.h"
#include "jni_constants.h"
#include "scoped_local_ref.h"

#include <string>
#include <vector>

jobjectArray newStringArray(JNIEnv* env, size_t count);

template <typename Counter, typename Getter>
jobjectArray toStringArray(JNIEnv* env, Counter* counter, Getter* getter) {
    size_t count = (*counter)();
    jobjectArray result = newStringArray(env, count);
    if (result == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < count; ++i) {
        ScopedLocalRef<jstring> s(env, env->NewStringUTF((*getter)(i)));
        if (env->ExceptionCheck()) {
            return NULL;
        }
        env->SetObjectArrayElement(result, i, s.get());
        if (env->ExceptionCheck()) {
            return NULL;
        }
    }
    return result;
}

template <typename Counter, typename Getter>
jobjectArray toStringArray16(JNIEnv* env, Counter* counter, Getter* getter) {
    size_t count = (*counter)();
    jobjectArray result = newStringArray(env, count);
    if (result == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < count; ++i) {
        int32_t charCount;
        const jchar* chars = (*getter)(&charCount);
        ScopedLocalRef<jstring> s(env, env->NewString(chars, charCount));
        if (env->ExceptionCheck()) {
            return NULL;
        }
        env->SetObjectArrayElement(result, i, s.get());
        if (env->ExceptionCheck()) {
            return NULL;
        }
    }
    return result;
}

JNIEXPORT jobjectArray toStringArray(JNIEnv* env, const std::vector<std::string>& strings);
JNIEXPORT jobjectArray toStringArray(JNIEnv* env, const char* const* strings);

#endif  // TO_STRING_ARRAY_H_included

jobjectArray newStringArray(JNIEnv* env, size_t count) {
    return env->NewObjectArray(count, JniConstants::GetStringClass(env), NULL);
}

struct VectorCounter {
    const std::vector<std::string>& strings;
    VectorCounter(const std::vector<std::string>& strings) : strings(strings) {}
    size_t operator()() {
        return strings.size();
    }
};

struct VectorGetter {
    const std::vector<std::string>& strings;
    VectorGetter(const std::vector<std::string>& strings) : strings(strings) {}
    const char* operator()(size_t i) {
        return strings[i].c_str();
    }
};

jobjectArray toStringArray(JNIEnv* env, const std::vector<std::string>& strings) {
    VectorCounter counter(strings);
    VectorGetter getter(strings);
    return toStringArray<VectorCounter, VectorGetter>(env, &counter, &getter);
}

struct ArrayCounter {
    const char* const* strings;
    ArrayCounter(const char* const* strings) : strings(strings) {}
    size_t operator()() {
        size_t count = 0;
        while (strings[count] != NULL) {
            ++count;
        }
        return count;
    }
};

struct ArrayGetter {
    const char* const* strings;
    ArrayGetter(const char* const* strings) : strings(strings) {}
    const char* operator()(size_t i) {
        return strings[i];
    }
};

jobjectArray toStringArray(JNIEnv* env, const char* const* strings) {
    ArrayCounter counter(strings);
    ArrayGetter getter(strings);
    return toStringArray(env, &counter, &getter);
}
