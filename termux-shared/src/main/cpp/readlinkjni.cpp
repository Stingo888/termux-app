#include <stdio.h>
#include <string>
#include <nativehelper/scoped_utf_chars.h>
#include "readlink.h"
#include "readlinkjni.h"

JNIEXPORT jstring JNICALL Java_ReadlinkTest_readlink
  (JNIEnv *env, jclass, jstring javaPath) {

    ScopedUtfChars path(env, javaPath);
    if (path.c_str() == NULL) { return NULL; }

    std::string result;
    if (!readlink(path.c_str(), result)) { return NULL; }

    return env->NewStringUTF(result.c_str());
}
