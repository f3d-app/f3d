// Automatically generated headers
#include <app_f3d_F3D_Camera.h>
#include <app_f3d_F3D_Engine.h>
#include <app_f3d_F3D_Loader.h>
#include <app_f3d_F3D_Options.h>
#include <app_f3d_F3D_Window.h>

#include <engine.h>
#include <log.h>

#include <cassert>

#define JAVA_BIND(Cls, Func) JNICALL Java_app_f3d_F3D_##Cls##_##Func

inline f3d::engine* GetEngine(JNIEnv* env, jobject self)
{
  jclass cls = env->GetObjectClass(self);
  jfieldID fid = env->GetFieldID(cls, "mNativeAddress", "J");
  jlong ptr = env->GetLongField(self, fid);

  return reinterpret_cast<f3d::engine*>(ptr);
}

extern "C"
{
  // Engine
  JNIEXPORT void JAVA_BIND(Engine, loadPlugin)(JNIEnv* env, jclass, jstring str)
  {
    const char* plugin = env->GetStringUTFChars(str, nullptr);
    f3d::engine::loadPlugin(plugin);
    env->ReleaseStringUTFChars(str, plugin);
  }

  JNIEXPORT void JAVA_BIND(Engine, autoloadPlugins)(JNIEnv*, jclass)
  {
    f3d::engine::autoloadPlugins();
  }

  JNIEXPORT void JAVA_BIND(Engine, setCachePath)(JNIEnv* env, jobject self, jstring path)
  {
    const char* str = env->GetStringUTFChars(path, nullptr);
    GetEngine(env, self)->setCachePath(str);
    env->ReleaseStringUTFChars(path, str);
  }

  JNIEXPORT jlong JAVA_BIND(Engine, construct)(JNIEnv*, jobject)
  {
    f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
    return reinterpret_cast<jlong>(new f3d::engine());
  }

  JNIEXPORT void JAVA_BIND(Engine, destroy)(JNIEnv*, jobject, jlong ptr)
  {
    delete reinterpret_cast<f3d::engine*>(ptr);
  }

  // Loader
  JNIEXPORT void JAVA_BIND(Loader, loadScene)(JNIEnv* env, jobject self, jstring path)
  {
    const char* str = env->GetStringUTFChars(path, nullptr);
    GetEngine(env, self)->getLoader().loadScene(str);
    env->ReleaseStringUTFChars(path, str);
  }
  JNIEXPORT void JAVA_BIND(Loader, loadGeometry)(JNIEnv* env, jobject self, jstring path)
  {
    const char* str = env->GetStringUTFChars(path, nullptr);
    GetEngine(env, self)->getLoader().loadGeometry(str);
    env->ReleaseStringUTFChars(path, str);
  }

  // Window
  JNIEXPORT void JAVA_BIND(Window, render)(JNIEnv* env, jobject self)
  {
    GetEngine(env, self)->getWindow().render();
  }

  JNIEXPORT void JAVA_BIND(Window, setSize)(JNIEnv* env, jobject self, jint w, jint h)
  {
    GetEngine(env, self)->getWindow().setSize(w, h);
  }

  JNIEXPORT jint JAVA_BIND(Window, getWidth)(JNIEnv* env, jobject self)
  {
    return GetEngine(env, self)->getWindow().getWidth();
  }

  JNIEXPORT jint JAVA_BIND(Window, getHeight)(JNIEnv* env, jobject self)
  {
    return GetEngine(env, self)->getWindow().getHeight();
  }

  JNIEXPORT jdoubleArray JAVA_BIND(Window, getDisplayFromWorld)(
    JNIEnv* env, jobject self, jdoubleArray pt)
  {
    double* arr = env->GetDoubleArrayElements(pt, nullptr);
    f3d::point3_t newPt =
      GetEngine(env, self)->getWindow().getDisplayFromWorld({ arr[0], arr[1], arr[2] });
    env->ReleaseDoubleArrayElements(pt, arr, 0);

    jdoubleArray ret = env->NewDoubleArray(3);
    env->SetDoubleArrayRegion(ret, 0, 3, newPt.data());

    return ret;
  }

  JNIEXPORT jdoubleArray JAVA_BIND(Window, getWorldFromDisplay)(
    JNIEnv* env, jobject self, jdoubleArray pt)
  {
    double* arr = env->GetDoubleArrayElements(pt, nullptr);
    f3d::point3_t newPt =
      GetEngine(env, self)->getWindow().getWorldFromDisplay({ arr[0], arr[1], arr[2] });
    env->ReleaseDoubleArrayElements(pt, arr, 0);

    jdoubleArray ret = env->NewDoubleArray(3);
    env->SetDoubleArrayRegion(ret, 0, 3, newPt.data());

    return ret;
  }

  // Options
  JNIEXPORT void JAVA_BIND(Options, toggle)(JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    GetEngine(env, self)->getOptions().toggle(str);
    env->ReleaseStringUTFChars(name, str);
  }

  // Camera
  JNIEXPORT void JAVA_BIND(Camera, dolly)(JNIEnv* env, jobject self, jdouble factor)
  {
    GetEngine(env, self)->getWindow().getCamera().dolly(factor);
  }

  JNIEXPORT void JAVA_BIND(Camera, roll)(JNIEnv* env, jobject self, jdouble angle)
  {
    GetEngine(env, self)->getWindow().getCamera().roll(angle);
  }

  JNIEXPORT void JAVA_BIND(Camera, azimuth)(JNIEnv* env, jobject self, jdouble angle)
  {
    GetEngine(env, self)->getWindow().getCamera().azimuth(angle);
  }

  JNIEXPORT void JAVA_BIND(Camera, yaw)(JNIEnv* env, jobject self, jdouble angle)
  {
    GetEngine(env, self)->getWindow().getCamera().yaw(angle);
  }

  JNIEXPORT void JAVA_BIND(Camera, elevation)(JNIEnv* env, jobject self, jdouble angle)
  {
    GetEngine(env, self)->getWindow().getCamera().elevation(angle);
  }

  JNIEXPORT void JAVA_BIND(Camera, pitch)(JNIEnv* env, jobject self, jdouble angle)
  {
    GetEngine(env, self)->getWindow().getCamera().pitch(angle);
  }

  JNIEXPORT jdoubleArray JAVA_BIND(Camera, getFocalPoint)(JNIEnv* env, jobject self)
  {
    f3d::point3_t pt = GetEngine(env, self)->getWindow().getCamera().getFocalPoint();

    jdoubleArray ret = env->NewDoubleArray(3);
    env->SetDoubleArrayRegion(ret, 0, 3, pt.data());

    return ret;
  }

  JNIEXPORT void JAVA_BIND(Camera, setFocalPoint)(JNIEnv* env, jobject self, jdoubleArray pt)
  {
    assert(env->GetArrayLength(pt) == 3);

    double* arr = env->GetDoubleArrayElements(pt, nullptr);
    GetEngine(env, self)->getWindow().getCamera().setFocalPoint({ arr[0], arr[1], arr[2] });
    env->ReleaseDoubleArrayElements(pt, arr, 0);
  }

  JNIEXPORT jdoubleArray JAVA_BIND(Camera, getPosition)(JNIEnv* env, jobject self)
  {
    f3d::point3_t pt = GetEngine(env, self)->getWindow().getCamera().getPosition();

    jdoubleArray ret = env->NewDoubleArray(3);
    env->SetDoubleArrayRegion(ret, 0, 3, pt.data());

    return ret;
  }

  JNIEXPORT void JAVA_BIND(Camera, setPosition)(JNIEnv* env, jobject self, jdoubleArray pt)
  {
    assert(env->GetArrayLength(pt) == 3);

    double* arr = env->GetDoubleArrayElements(pt, nullptr);
    GetEngine(env, self)->getWindow().getCamera().setPosition({ arr[0], arr[1], arr[2] });
    env->ReleaseDoubleArrayElements(pt, arr, 0);
  }

  JNIEXPORT void JAVA_BIND(Camera, resetToBounds)(JNIEnv* env, jobject self)
  {
    GetEngine(env, self)->getWindow().getCamera().resetToBounds();
  }
}
