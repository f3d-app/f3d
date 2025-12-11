#include "F3DJavaBindings.h"

#include <app_f3d_F3D_Context.h>

#include <context.h>

struct f3d_java_context
{
  f3d::context::function func;

  explicit f3d_java_context(f3d::context::function&& f)
    : func(std::move(f))
  {
  }
};

extern "C"
{
  JNIEXPORT jlong JAVA_BIND(Context, glx)(JNIEnv*, jclass)
  {
    return reinterpret_cast<jlong>(new f3d_java_context(f3d::context::glx()));
  }

  JNIEXPORT jlong JAVA_BIND(Context, wgl)(JNIEnv*, jclass)
  {
    return reinterpret_cast<jlong>(new f3d_java_context(f3d::context::wgl()));
  }

  JNIEXPORT jlong JAVA_BIND(Context, cocoa)(JNIEnv*, jclass)
  {
    return reinterpret_cast<jlong>(new f3d_java_context(f3d::context::cocoa()));
  }

  JNIEXPORT jlong JAVA_BIND(Context, egl)(JNIEnv*, jclass)
  {
    return reinterpret_cast<jlong>(new f3d_java_context(f3d::context::egl()));
  }

  JNIEXPORT jlong JAVA_BIND(Context, osmesa)(JNIEnv*, jclass)
  {
    return reinterpret_cast<jlong>(new f3d_java_context(f3d::context::osmesa()));
  }

  JNIEXPORT jlong JAVA_BIND(Context, getSymbol)(JNIEnv* env, jclass, jstring lib, jstring func)
  {
    const char* libStr = env->GetStringUTFChars(lib, nullptr);
    const char* funcStr = env->GetStringUTFChars(func, nullptr);

    f3d_java_context* ctx = new f3d_java_context(f3d::context::getSymbol(libStr, funcStr));

    env->ReleaseStringUTFChars(lib, libStr);
    env->ReleaseStringUTFChars(func, funcStr);

    return reinterpret_cast<jlong>(ctx);
  }

  JNIEXPORT void JAVA_BIND(Context, delete)(JNIEnv*, jclass, jlong contextHandle)
  {
    delete reinterpret_cast<f3d_java_context*>(contextHandle);
  }
}
