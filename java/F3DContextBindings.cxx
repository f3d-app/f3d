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
  JNIEXPORT jlong JAVA_BIND(Context, glx)(JNIEnv* env, jclass)
  {
    try
    {
      return reinterpret_cast<jlong>(new f3d_java_context(f3d::context::glx()));
    }
    catch (const f3d::context::loading_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$LoadingException", e.what());
    }
    catch (const f3d::context::symbol_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$SymbolException", e.what());
    }
    return 0;
  }

  JNIEXPORT jlong JAVA_BIND(Context, wgl)(JNIEnv* env, jclass)
  {
    try
    {
      return reinterpret_cast<jlong>(new f3d_java_context(f3d::context::wgl()));
    }
    catch (const f3d::context::loading_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$LoadingException", e.what());
    }
    catch (const f3d::context::symbol_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$SymbolException", e.what());
    }
    return 0;
  }

  JNIEXPORT jlong JAVA_BIND(Context, cocoa)(JNIEnv* env, jclass)
  {
    try
    {
      return reinterpret_cast<jlong>(new f3d_java_context(f3d::context::cocoa()));
    }
    catch (const f3d::context::loading_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$LoadingException", e.what());
    }
    catch (const f3d::context::symbol_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$SymbolException", e.what());
    }
    return 0;
  }

  JNIEXPORT jlong JAVA_BIND(Context, egl)(JNIEnv* env, jclass)
  {
    try
    {
      return reinterpret_cast<jlong>(new f3d_java_context(f3d::context::egl()));
    }
    catch (const f3d::context::loading_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$LoadingException", e.what());
    }
    catch (const f3d::context::symbol_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$SymbolException", e.what());
    }
    return 0;
  }

  JNIEXPORT jlong JAVA_BIND(Context, osmesa)(JNIEnv* env, jclass)
  {
    try
    {
      return reinterpret_cast<jlong>(new f3d_java_context(f3d::context::osmesa()));
    }
    catch (const f3d::context::loading_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$LoadingException", e.what());
    }
    catch (const f3d::context::symbol_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$SymbolException", e.what());
    }
    return 0;
  }

  JNIEXPORT jlong JAVA_BIND(Context, getSymbol)(JNIEnv* env, jclass, jstring lib, jstring func)
  {
    const char* libStr = env->GetStringUTFChars(lib, nullptr);
    const char* funcStr = env->GetStringUTFChars(func, nullptr);

    try
    {
      f3d_java_context* ctx = new f3d_java_context(f3d::context::getSymbol(libStr, funcStr));
      env->ReleaseStringUTFChars(lib, libStr);
      env->ReleaseStringUTFChars(func, funcStr);
      return reinterpret_cast<jlong>(ctx);
    }
    catch (const f3d::context::loading_exception& e)
    {
      env->ReleaseStringUTFChars(lib, libStr);
      env->ReleaseStringUTFChars(func, funcStr);
      F3DThrowJavaException(env, "app/f3d/F3D/Context$LoadingException", e.what());
    }
    catch (const f3d::context::symbol_exception& e)
    {
      env->ReleaseStringUTFChars(lib, libStr);
      env->ReleaseStringUTFChars(func, funcStr);
      F3DThrowJavaException(env, "app/f3d/F3D/Context$SymbolException", e.what());
    }
    return 0;
  }

  JNIEXPORT void JAVA_BIND(Context, delete)(JNIEnv*, jclass, jlong contextHandle)
  {
    delete reinterpret_cast<f3d_java_context*>(contextHandle);
  }
}
