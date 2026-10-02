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

    const JniUTFString strAChars(env, strA);
    const JniUTFString strBChars(env, strB);

    const unsigned int distance = f3d::utils::textDistance(strAChars.c_str(), strBChars.c_str());

    return static_cast<jint>(distance);
  }

  JNIEXPORT jobject JAVA_BIND(Utils, tokenize)(
    JNIEnv* env, jclass, jstring str, jboolean keepComments)
  {
    if (!str)
    {
      return CreateStringList(env, std::vector<std::string>());
    }

    const JniUTFString strChars(env, str);
    jobject result = nullptr;
    try
    {
      result = CreateStringList(env, f3d::utils::tokenize(strChars.c_str(), keepComments != 0));
    }
    catch (const std::exception& e)
    {
      const JniLocalRef<jclass> exceptionClass(env, env->FindClass("java/lang/RuntimeException"));
      env->ThrowNew(exceptionClass, e.what());
    }
    return result;
  }

  JNIEXPORT jstring JAVA_BIND(Utils, collapsePath)(
    JNIEnv* env, jclass, jstring path, jstring baseDirectory)
  {
    if (!path)
    {
      return env->NewStringUTF("");
    }

    const JniUTFString pathChars(env, path);
    const std::string pathStr = pathChars.c_str();

    std::filesystem::path basePath;
    if (baseDirectory)
    {
      const JniUTFString baseDirChars(env, baseDirectory);
      basePath = baseDirChars.c_str();
    }

    const std::filesystem::path result = f3d::utils::collapsePath(pathStr, basePath);
    return env->NewStringUTF(result.string().c_str());
  }

  JNIEXPORT jstring JAVA_BIND(Utils, globToRegex)(
    JNIEnv* env, jclass, jstring glob, jchar pathSeparator)
  {
    if (!glob)
    {
      return env->NewStringUTF("");
    }

    const JniUTFString globChars(env, glob);
    std::string result;
    try
    {
      result = f3d::utils::globToRegex(globChars.c_str(), static_cast<char>(pathSeparator));
    }
    catch (const std::exception& e)
    {
      const JniLocalRef<jclass> exceptionClass(env, env->FindClass("java/lang/RuntimeException"));
      env->ThrowNew(exceptionClass, e.what());
    }
    return env->ExceptionCheck() ? nullptr : env->NewStringUTF(result.c_str());
  }

  JNIEXPORT jstring JAVA_BIND(Utils, getEnv)(JNIEnv* env, jclass, jstring envVar)
  {
    if (!envVar)
    {
      return nullptr;
    }

    const JniUTFString envVarChars(env, envVar);
    std::optional<std::string> result = f3d::utils::getEnv(envVarChars.c_str());

    return result.has_value() ? env->NewStringUTF(result.value().c_str()) : nullptr;
  }

  JNIEXPORT jstring JAVA_BIND(Utils, getKnownFolder)(JNIEnv* env, jclass, jobject knownFolder)
  {
    if (!knownFolder)
    {
      return nullptr;
    }

    const JniLocalRef<jclass> enumClass(env, env->GetObjectClass(knownFolder));
    jmethodID getValueMethod = env->GetMethodID(enumClass, "getValue", "()I");
    const jint folderValue = env->CallIntMethod(knownFolder, getValueMethod);

    const f3d::utils::KnownFolder folder = static_cast<f3d::utils::KnownFolder>(folderValue);
    std::optional<std::string> result = f3d::utils::getKnownFolder(folder);

    return result.has_value() ? env->NewStringUTF(result.value().c_str()) : nullptr;
  }
}
