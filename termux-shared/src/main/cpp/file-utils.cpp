#include <stdio.h>
#include <unistd.h>
#include <string>
#include "scoped_utf_chars.h"
#include "readlink.h"
#include "file-utils.h"

JNIEXPORT jstring JNICALL Java_com_termux_shared_file_FileUtils_readlink
  (JNIEnv *env, jclass, jstring javaPath) {

    ScopedUtfChars path(env, javaPath);
    if (path.c_str() == NULL) { return NULL; }

    std::string result;
    if (!readlink(path.c_str(), result)) { return NULL; }

    return env->NewStringUTF(result.c_str());
}

JNIEXPORT void JNICALL Java_com_termux_shared_file_FileUtils_symlink
  (JNIEnv *env, jclass, jstring javaOldPath, jstring javaNewPath) {

    ScopedUtfChars oldPath(env, javaOldPath);
    if (oldPath.c_str() == NULL) { return; }

    ScopedUtfChars newPath(env, javaNewPath);
    if (newPath.c_str() == NULL) { return; }

    symlink(oldPath.c_str(), newPath.c_str());
}

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    return JNI_VERSION_10;
}
