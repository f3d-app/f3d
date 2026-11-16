#include "F3DJavaBindings.h"

#include <app_f3d_F3D_Log.h>

#include <log.h>

static jobject g_callback_ref = nullptr;
static JavaVM* g_jvm = nullptr;

f3d::log::VerboseLevel JavaVerboseLevelToNative(JNIEnv* env, jobject level)
{
  const JniLocalRef<jclass> enumClass(env, env->GetObjectClass(level));
  jmethodID ordinalMethod = env->GetMethodID(enumClass, "ordinal", "()I");
  const jint ordinal = env->CallIntMethod(level, ordinalMethod);

  switch (ordinal)
  {
    case 0:
      return f3d::log::VerboseLevel::DEBUG;
    case 1:
      return f3d::log::VerboseLevel::INFO;
    case 2:
      return f3d::log::VerboseLevel::WARN;
    case 3:
      return f3d::log::VerboseLevel::ERROR;
    default:
      return f3d::log::VerboseLevel::QUIET;
  }
}

jobject NativeVerboseLevelToJava(JNIEnv* env, const f3d::log::VerboseLevel level)
{
  const JniLocalRef<jclass> enumClass(env, env->FindClass("app/f3d/F3D/Log$VerboseLevel"));
  jfieldID fieldID;

  switch (level)
  {
    case f3d::log::VerboseLevel::DEBUG:
      fieldID = env->GetStaticFieldID(enumClass, "DEBUG", "Lapp/f3d/F3D/Log$VerboseLevel;");
      break;
    case f3d::log::VerboseLevel::INFO:
      fieldID = env->GetStaticFieldID(enumClass, "INFO", "Lapp/f3d/F3D/Log$VerboseLevel;");
      break;
    case f3d::log::VerboseLevel::WARN:
      fieldID = env->GetStaticFieldID(enumClass, "WARN", "Lapp/f3d/F3D/Log$VerboseLevel;");
      break;
    case f3d::log::VerboseLevel::ERROR:
      fieldID = env->GetStaticFieldID(enumClass, "ERROR", "Lapp/f3d/F3D/Log$VerboseLevel;");
      break;
    default:
      fieldID = env->envGetStaticFieldID(enumClass, "QUIET", "Lapp/f3d/F3D/Log$VerboseLevel;");
      break;
  }
  return env->GetStaticObjectField(enumClass, fieldID);
}

static void cpp_log_forwarder(f3d::log::VerboseLevel level, const std::string& message)
{
  if (g_callback_ref == nullptr || g_jvm == nullptr)
  {
    return;
  }

  JNIEnv* env;
  bool needsDetach = false;
  const int envStatus = g_jvm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);

  if (envStatus == JNI_EDETACHED)
  {
#ifdef __ANDROID__
    if (g_jvm->AttachCurrentThread(&env, nullptr) != 0)
#else
    if (g_jvm->AttachCurrentThread(reinterpret_cast<void**>(&env), nullptr) != 0)
#endif
    {
      return;
    }
    needsDetach = true;
  }
  else if (envStatus != JNI_OK)
  {
    return;
  }

  const JniLocalRef<jobject> jLevel(env, NativeVerboseLevelToJava(env, level));

  const JniLocalRef<jstring> jMessage(env, env->NewStringUTF(message.c_str()));

  const JniLocalRef<jclass> callbackClass(env, env->GetObjectClass(g_callback_ref));
  jmethodID onLogMessageMethod = env->GetMethodID(
    callbackClass, "onLogMessage", "(Lapp/f3d/F3D/Log$VerboseLevel;Ljava/lang/String;)V");

  env->CallVoidMethod(g_callback_ref, onLogMessageMethod, jLevel.get(), jMessage.get());

  if (needsDetach)
  {
    g_jvm->DetachCurrentThread();
  }
}

extern "C"
{
  JNIEXPORT void JAVA_BIND(Log, print)(JNIEnv* env, jclass, jobject level, jstring message)
  {
    if (!level || !message)
    {
      return;
    }

    const JniUTFString messageStr(env, message);

    const f3d::log::VerboseLevel cppLevel = JavaVerboseLevelToNative(env, level);
    f3d::log::print(cppLevel, messageStr.c_str());
  }

  JNIEXPORT void JAVA_BIND(Log, debug)(JNIEnv* env, jclass, jstring message)
  {
    if (!message)
    {
      return;
    }

    const JniUTFString messageStr(env, message);
    f3d::log::debug(messageStr.c_str());
  }

  JNIEXPORT void JAVA_BIND(Log, info)(JNIEnv* env, jclass, jstring message)
  {
    if (!message)
    {
      return;
    }

    const JniUTFString messageStr(env, message);
    f3d::log::info(messageStr.c_str());
  }

  JNIEXPORT void JAVA_BIND(Log, warn)(JNIEnv* env, jclass, jstring message)
  {
    if (!message)
    {
      return;
    }

    const JniUTFString messageStr(env, message);
    f3d::log::warn(messageStr.c_str());
  }

  JNIEXPORT void JAVA_BIND(Log, error)(JNIEnv* env, jclass, jstring message)
  {
    if (!message)
    {
      return;
    }

    const JniUTFString messageStr(env, message);
    f3d::log::error(messageStr.c_str());
  }

  JNIEXPORT void JAVA_BIND(Log, setUseColoring)(JNIEnv*, jclass, jboolean use)
  {
    f3d::log::setUseColoring(use != 0);
  }

  JNIEXPORT void JAVA_BIND(Log, setVerboseLevel)(
    JNIEnv* env, jclass, jobject level, jboolean forceStdErr)
  {
    if (!level)
    {
      return;
    }

    const f3d::log::VerboseLevel cppLevel = JavaVerboseLevelToNative(env, level);
    f3d::log::setVerboseLevel(cppLevel, forceStdErr != 0);
  }

  JNIEXPORT jobject JAVA_BIND(Log, getVerboseLevel)(JNIEnv* env, jclass)
  {
    const f3d::log::VerboseLevel cppLevel = f3d::log::getVerboseLevel();

    return NativeVerboseLevelToJava(env, cppLevel);
  }

  JNIEXPORT void JAVA_BIND(Log, forward)(JNIEnv* env, jclass, jobject callback)
  {
    if (g_callback_ref != nullptr)
    {
      env->DeleteGlobalRef(g_callback_ref);
      g_callback_ref = nullptr;
    }

    if (callback != nullptr)
    {
      if (g_jvm == nullptr)
      {
        env->GetJavaVM(&g_jvm);
      }

      g_callback_ref = env->NewGlobalRef(callback);

      f3d::log::forward(cpp_log_forwarder);
    }
    else
    {
      f3d::log::forward(nullptr);
      g_jvm = nullptr;
    }
  }
}
