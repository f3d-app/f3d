#include "F3DJavaBindings.h"

#include <app_f3d_F3D_Utils.h>

#include <utils.h>

extern "C"
{
  JNIEXPORT jint JAVA_BIND(Utils, textDistance)(JNIEnv* env, jclass, jstring strA, jstring strB)
  {
    if (!strA || !strB)
    {
      return 0;
    }

    const char* strAChars = env->GetStringUTFChars(strA, nullptr);
    const char* strBChars = env->GetStringUTFChars(strB, nullptr);

    unsigned int distance = f3d::utils::textDistance(strAChars, strBChars);

    env->ReleaseStringUTFChars(strA, strAChars);
    env->ReleaseStringUTFChars(strB, strBChars);

    return static_cast<jint>(distance);
  }

  JNIEXPORT jobject JAVA_BIND(Utils, tokenize)(
    JNIEnv* env, jclass, jstring str, jboolean keepComments)
  {
    if (!str)
    {
      return CreateStringList(env, std::vector<std::string>());
    }

    const char* strChars = env->GetStringUTFChars(str, nullptr);

    try
    {
      std::vector<std::string> tokens = f3d::utils::tokenize(strChars, keepComments != 0);
      env->ReleaseStringUTFChars(str, strChars);
      return CreateStringList(env, tokens);
    }
    catch (const std::exception& e)
    {
      env->ReleaseStringUTFChars(str, strChars);
      jclass exceptionClass = env->FindClass("java/lang/RuntimeException");
      env->ThrowNew(exceptionClass, e.what());
      return nullptr;
    }
  }

  JNIEXPORT jstring JAVA_BIND(Utils, collapsePath)(
    JNIEnv* env, jclass, jstring path, jstring baseDirectory)
  {
    if (!path)
    {
      return env->NewStringUTF("");
    }

    const char* pathChars = env->GetStringUTFChars(path, nullptr);
    std::string pathStr = pathChars;
    env->ReleaseStringUTFChars(path, pathChars);

    std::filesystem::path basePath;
    if (baseDirectory)
    {
      const char* baseDirChars = env->GetStringUTFChars(baseDirectory, nullptr);
      basePath = baseDirChars;
      env->ReleaseStringUTFChars(baseDirectory, baseDirChars);
    }

    std::filesystem::path result = f3d::utils::collapsePath(pathStr, basePath);
    return env->NewStringUTF(result.string().c_str());
  }

  JNIEXPORT jstring JAVA_BIND(Utils, globToRegex)(
    JNIEnv* env, jclass, jstring glob, jchar pathSeparator)
  {
    if (!glob)
    {
      return env->NewStringUTF("");
    }

    const char* globChars = env->GetStringUTFChars(glob, nullptr);

    try
    {
      std::string result = f3d::utils::globToRegex(globChars, static_cast<char>(pathSeparator));
      env->ReleaseStringUTFChars(glob, globChars);
      return env->NewStringUTF(result.c_str());
    }
    catch (const std::exception& e)
    {
      env->ReleaseStringUTFChars(glob, globChars);
      jclass exceptionClass = env->FindClass("java/lang/RuntimeException");
      env->ThrowNew(exceptionClass, e.what());
      return nullptr;
    }
  }

  JNIEXPORT jstring JAVA_BIND(Utils, getEnv)(JNIEnv* env, jclass, jstring envVar)
  {
    if (!envVar)
    {
      return nullptr;
    }

    const char* envVarChars = env->GetStringUTFChars(envVar, nullptr);
    std::optional<std::string> result = f3d::utils::getEnv(envVarChars);
    env->ReleaseStringUTFChars(envVar, envVarChars);

    return result.has_value() ? env->NewStringUTF(result.value().c_str()) : nullptr;
  }

  JNIEXPORT jstring JAVA_BIND(Utils, getKnownFolder)(JNIEnv* env, jclass, jobject knownFolder)
  {
    if (!knownFolder)
    {
      return nullptr;
    }

    jclass enumClass = env->GetObjectClass(knownFolder);
    jmethodID getValueMethod = env->GetMethodID(enumClass, "getValue", "()I");
    jint folderValue = env->CallIntMethod(knownFolder, getValueMethod);

    f3d::utils::KnownFolder folder = static_cast<f3d::utils::KnownFolder>(folderValue);
    std::optional<std::string> result = f3d::utils::getKnownFolder(folder);

    return result.has_value() ? env->NewStringUTF(result.value().c_str()) : nullptr;
  }

  JNIEXPORT jdouble JAVA_BIND(Utils, getDPIScale)(JNIEnv* env, jclass)
  {
    return static_cast<jdouble>(f3d::utils::getDPIScale());
  }
}
