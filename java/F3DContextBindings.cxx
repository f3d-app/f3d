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
    JniUTFString libStr(env, lib);
    JniUTFString funcStr(env, func);

    jlong result = 0;
    try
    {
      result = reinterpret_cast<jlong>(
        new f3d_java_context(f3d::context::getSymbol(libStr.c_str(), funcStr.c_str())));
    }
    catch (const f3d::context::loading_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$LoadingException", e.what());
    }
    catch (const f3d::context::symbol_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$SymbolException", e.what());
    }
    return result;
  }

  JNIEXPORT void JAVA_BIND(Context, delete)(JNIEnv*, jclass, jlong contextHandle)
  {
    delete reinterpret_cast<f3d_java_context*>(contextHandle);
  }
}
