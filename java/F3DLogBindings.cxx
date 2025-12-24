#include "F3DJavaBindings.h"

#include <app_f3d_F3D_Log.h>

#include <log.h>

static jobject g_callback_ref = nullptr;
static JavaVM* g_jvm = nullptr;

static void cpp_log_forwarder(f3d::log::VerboseLevel level, const std::string& message)
{
  if (g_callback_ref == nullptr || g_jvm == nullptr)
  {
    return;
  }

  JNIEnv* env;
#ifdef __ANDROID__
  if (g_jvm->AttachCurrentThread(&env, nullptr) != 0)
#else
  if (g_jvm->AttachCurrentThread(reinterpret_cast<void**>(&env), nullptr) != 0)
#endif
  {
    return;
  }

  jclass enumClass = env->FindClass("app/f3d/F3D/Log$VerboseLevel");
  jmethodID fromValueMethod =
    env->GetStaticMethodID(enumClass, "fromValue", "(I)Lapp/f3d/F3D/Log$VerboseLevel;");
  jobject jLevel = env->CallStaticObjectMethod(enumClass, fromValueMethod, static_cast<int>(level));

  jstring jMessage = env->NewStringUTF(message.c_str());

  jclass callbackClass = env->GetObjectClass(g_callback_ref);
  jmethodID onLogMessageMethod = env->GetMethodID(
    callbackClass, "onLogMessage", "(Lapp/f3d/F3D/Log$VerboseLevel;Ljava/lang/String;)V");

  env->CallVoidMethod(g_callback_ref, onLogMessageMethod, jLevel, jMessage);

  env->DeleteLocalRef(jLevel);
  env->DeleteLocalRef(jMessage);

  g_jvm->DetachCurrentThread();
}

extern "C"
{
  JNIEXPORT void JAVA_BIND(Log, print)(JNIEnv* env, jclass, jobject level, jstring message)
  {
    if (!level || !message)
    {
      return;
    }

    jclass enumClass = env->GetObjectClass(level);
    jmethodID getValueMethod = env->GetMethodID(enumClass, "getValue", "()I");
    jint levelValue = env->CallIntMethod(level, getValueMethod);

    const char* messageStr = env->GetStringUTFChars(message, nullptr);

    f3d::log::VerboseLevel cppLevel = static_cast<f3d::log::VerboseLevel>(levelValue);
    f3d::log::print(cppLevel, messageStr);

    env->ReleaseStringUTFChars(message, messageStr);
  }

  JNIEXPORT void JAVA_BIND(Log, debug)(JNIEnv* env, jclass, jstring message)
  {
    if (!message)
    {
      return;
    }

    const char* messageStr = env->GetStringUTFChars(message, nullptr);
    f3d::log::debug(messageStr);
    env->ReleaseStringUTFChars(message, messageStr);
  }

  JNIEXPORT void JAVA_BIND(Log, info)(JNIEnv* env, jclass, jstring message)
  {
    if (!message)
    {
      return;
    }

    const char* messageStr = env->GetStringUTFChars(message, nullptr);
    f3d::log::info(messageStr);
    env->ReleaseStringUTFChars(message, messageStr);
  }

  JNIEXPORT void JAVA_BIND(Log, warn)(JNIEnv* env, jclass, jstring message)
  {
    if (!message)
    {
      return;
    }

    const char* messageStr = env->GetStringUTFChars(message, nullptr);
    f3d::log::warn(messageStr);
    env->ReleaseStringUTFChars(message, messageStr);
  }

  JNIEXPORT void JAVA_BIND(Log, error)(JNIEnv* env, jclass, jstring message)
  {
    if (!message)
    {
      return;
    }

    const char* messageStr = env->GetStringUTFChars(message, nullptr);
    f3d::log::error(messageStr);
    env->ReleaseStringUTFChars(message, messageStr);
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

    jclass enumClass = env->GetObjectClass(level);
    jmethodID getValueMethod = env->GetMethodID(enumClass, "getValue", "()I");
    jint levelValue = env->CallIntMethod(level, getValueMethod);

    f3d::log::VerboseLevel cppLevel = static_cast<f3d::log::VerboseLevel>(levelValue);
    f3d::log::setVerboseLevel(cppLevel, forceStdErr != 0);
  }

  JNIEXPORT jobject JAVA_BIND(Log, getVerboseLevel)(JNIEnv* env, jclass)
  {
    f3d::log::VerboseLevel cppLevel = f3d::log::getVerboseLevel();

    jclass enumClass = env->FindClass("app/f3d/F3D/Log$VerboseLevel");
    jmethodID fromValueMethod =
      env->GetStaticMethodID(enumClass, "fromValue", "(I)Lapp/f3d/F3D/Log$VerboseLevel;");

    return env->CallStaticObjectMethod(enumClass, fromValueMethod, static_cast<int>(cppLevel));
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
