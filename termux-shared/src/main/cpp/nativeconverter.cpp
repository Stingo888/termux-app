#include <string>
#include <vector>
#include <unicode/ucnv.h>
#include "include/scoped_utf_chars.h"
#include "include/toStringArray.h"
#include "include/IcuUtilities.h"

static const char* getICUCanonicalName(const char* name) {
  UErrorCode error = U_ZERO_ERROR;
  const char* canonicalName = NULL;
  if ((canonicalName = ucnv_getCanonicalName(name, "MIME", &error)) != NULL) {
    return canonicalName;
  } else if ((canonicalName = ucnv_getCanonicalName(name, "IANA", &error)) != NULL) {
    return canonicalName;
  } else if ((canonicalName = ucnv_getCanonicalName(name, "", &error)) != NULL) {
    return canonicalName;
  } else if ((canonicalName = ucnv_getAlias(name, 0, &error)) != NULL) {
    // We have some aliases in the form x-blah .. match those first.
    return canonicalName;
  } else if (strstr(name, "x-") == name) {
    // Check if the converter can be opened with the name given.
    error = U_ZERO_ERROR;
    icu::LocalUConverterPointer cnv(ucnv_open(name + 2, &error));
    if (U_SUCCESS(error)) {
      return name + 2;
    }
  }
  return NULL;
}

static jstring getJavaCanonicalName(JNIEnv* env, const char* icuCanonicalName) {
  UErrorCode status = U_ZERO_ERROR;

  // Check to see if this is a well-known MIME or IANA name.
  const char* cName = NULL;
  if ((cName = ucnv_getStandardName(icuCanonicalName, "MIME", &status)) != NULL) {
    return env->NewStringUTF(cName);
  } else if ((cName = ucnv_getStandardName(icuCanonicalName, "IANA", &status)) != NULL) {
    return env->NewStringUTF(cName);
  }

  // Check to see if an alias already exists with "x-" prefix, if yes then
  // make that the canonical name.
  int32_t aliasCount = ucnv_countAliases(icuCanonicalName, &status);
  for (int i = 0; i < aliasCount; ++i) {
    const char* name = ucnv_getAlias(icuCanonicalName, i, &status);
    if (name != NULL && name[0] == 'x' && name[1] == '-') {
      return env->NewStringUTF(name);
    }
  }

  // As a last resort, prepend "x-" to any alias and make that the canonical name.
  status = U_ZERO_ERROR;
  const char* name = ucnv_getStandardName(icuCanonicalName, "UTR22", &status);
  if (name == NULL && strchr(icuCanonicalName, ',') != NULL) {
    name = ucnv_getAlias(icuCanonicalName, 1, &status);
  }
  // If there is no UTR22 canonical name then just return the original name.
  if (name == NULL) {
    name = icuCanonicalName;
  }
  std::unique_ptr<char[]> result(new char[2 + strlen(name) + 1]);
  strcpy(&result[0], "x-");
  strcat(&result[0], name);
  return env->NewStringUTF(&result[0]);
}

static bool collectStandardNames(JNIEnv* env, const char* canonicalName, const char* standard, std::vector<std::string>& result) {
  UErrorCode status = U_ZERO_ERROR;
  icu::UStringEnumeration e(ucnv_openStandardNames(canonicalName, standard, &status));
  if (maybeThrowIcuException(env, "ucnv_openStandardNames", status)) {
    return false;
  }

  const int32_t count = e.count(status);
  if (maybeThrowIcuException(env, "StringEnumeration::count", status)) {
    return false;
  }

  for (int32_t i = 0; i < count; ++i) {
    const icu::UnicodeString* string = e.snext(status);
    if (maybeThrowIcuException(env, "StringEnumeration::snext", status)) {
      return false;
    }
    std::string s;
    string->toUTF8String(s);
    if (s.find_first_of("+,") == std::string::npos) {
      result.push_back(s);
    }
  }
  return true;
}

extern "C"
JNIEXPORT jobject JNICALL Java_NativeConverter_charsetForName
  (JNIEnv* env, jclass, jstring charsetName) {
    ScopedUtfChars charsetNameChars(env, charsetName);
    if (charsetNameChars.c_str() == NULL) {
        return NULL;
    }

    // Get ICU's canonical name for this charset.
    const char* icuCanonicalName = getICUCanonicalName(charsetNameChars.c_str());
    if (icuCanonicalName == NULL) {
        return NULL;
    }

    // Get Java's canonical name for this charset.
    jstring javaCanonicalName = getJavaCanonicalName(env, icuCanonicalName);
    if (env->ExceptionCheck()) {
        return NULL;
    }

    // Check that this charset is supported.
    {
        // ICU doesn't offer any "isSupported", so we just open and immediately close.
        UErrorCode error = U_ZERO_ERROR;
        icu::LocalUConverterPointer cnv(ucnv_open(icuCanonicalName, &error));
        if (!U_SUCCESS(error)) {
            return NULL;
        }
    }

    // Get the aliases for this charset.
    std::vector<std::string> aliases;
    if (!collectStandardNames(env, icuCanonicalName, "IANA", aliases)) {
        return NULL;
    }
    if (!collectStandardNames(env, icuCanonicalName, "MIME", aliases)) {
        return NULL;
    }
    if (!collectStandardNames(env, icuCanonicalName, "JAVA", aliases)) {
        return NULL;
    }
    if (!collectStandardNames(env, icuCanonicalName, "WINDOWS", aliases)) {
        return NULL;
    }
    jobjectArray javaAliases = toStringArray(env, aliases);
    if (env->ExceptionCheck()) {
        return NULL;
    }

    // Construct the CharsetICU object.
    static jmethodID charsetConstructor = env->GetMethodID(JniConstants::GetCharsetICUClass(env), "<init>",
            "(Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;)V");
    if (env->ExceptionCheck()) {
        return NULL;
    }
    return env->NewObject(JniConstants::GetCharsetICUClass(env), charsetConstructor,
            javaCanonicalName, env->NewStringUTF(icuCanonicalName), javaAliases);
}
