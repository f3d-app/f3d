#include "F3DJavaBindings.h"

#include <app_f3d_F3D_Window.h>

#include <types.h>
#include <window.h>

extern "C"
{
  JNIEXPORT jobject JAVA_BIND(Window, getType)(JNIEnv* env, jobject self)
  {
    f3d::window::Type type = GetEngine(env, self)->getWindow().getType();

    jclass enumClass = env->FindClass("app/f3d/F3D/Window$Type");
    jfieldID fieldID;

    switch (type)
    {
      case f3d::window::Type::NONE:
        fieldID = env->GetStaticFieldID(enumClass, "NONE", "Lapp/f3d/F3D/Window$Type;");
        break;
      case f3d::window::Type::EXTERNAL:
        fieldID = env->GetStaticFieldID(enumClass, "EXTERNAL", "Lapp/f3d/F3D/Window$Type;");
        break;
      case f3d::window::Type::GLX:
        fieldID = env->GetStaticFieldID(enumClass, "GLX", "Lapp/f3d/F3D/Window$Type;");
        break;
      case f3d::window::Type::WGL:
        fieldID = env->GetStaticFieldID(enumClass, "WGL", "Lapp/f3d/F3D/Window$Type;");
        break;
      case f3d::window::Type::COCOA:
        fieldID = env->GetStaticFieldID(enumClass, "COCOA", "Lapp/f3d/F3D/Window$Type;");
        break;
      case f3d::window::Type::EGL:
        fieldID = env->GetStaticFieldID(enumClass, "EGL", "Lapp/f3d/F3D/Window$Type;");
        break;
      case f3d::window::Type::OSMESA:
        fieldID = env->GetStaticFieldID(enumClass, "OSMESA", "Lapp/f3d/F3D/Window$Type;");
        break;
      case f3d::window::Type::WASM:
        fieldID = env->GetStaticFieldID(enumClass, "WASM", "Lapp/f3d/F3D/Window$Type;");
        break;
      default:
        fieldID = env->GetStaticFieldID(enumClass, "UNKNOWN", "Lapp/f3d/F3D/Window$Type;");
        break;
    }

    return env->GetStaticObjectField(enumClass, fieldID);
  }

  JNIEXPORT jboolean JAVA_BIND(Window, isOffscreen)(JNIEnv* env, jobject self)
  {
    return GetEngine(env, self)->getWindow().isOffscreen();
  }

  JNIEXPORT jboolean JAVA_BIND(Window, render)(JNIEnv* env, jobject self)
  {
    return GetEngine(env, self)->getWindow().render();
  }

  JNIEXPORT jobject JAVA_BIND(Window, renderToImage)(
    JNIEnv* env, jobject self, jboolean noBackground)
  {
    f3d::image* img = new f3d::image(GetEngine(env, self)->getWindow().renderToImage(noBackground));

    jclass imageClass = env->FindClass("app/f3d/F3D/Image");
    jmethodID constructor = env->GetMethodID(imageClass, "<init>", "(J)V");

    jobject result = env->NewObject(imageClass, constructor, reinterpret_cast<jlong>(img));

    return result;
  }

  JNIEXPORT jobject JAVA_BIND(Window, setSize)(JNIEnv* env, jobject self, jint width, jint height)
  {
    GetEngine(env, self)->getWindow().setSize(width, height);
    return self;
  }

  JNIEXPORT jint JAVA_BIND(Window, getWidth)(JNIEnv* env, jobject self)
  {
    return GetEngine(env, self)->getWindow().getWidth();
  }

  JNIEXPORT jint JAVA_BIND(Window, getHeight)(JNIEnv* env, jobject self)
  {
    return GetEngine(env, self)->getWindow().getHeight();
  }

  JNIEXPORT jobject JAVA_BIND(Window, setPosition)(JNIEnv* env, jobject self, jint x, jint y)
  {
    GetEngine(env, self)->getWindow().setPosition(x, y);
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Window, setIcon)(JNIEnv* env, jobject self, jbyteArray icon)
  {
    jsize iconSize = env->GetArrayLength(icon);
    jbyte* iconData = env->GetByteArrayElements(icon, nullptr);

    GetEngine(env, self)->getWindow().setIcon(reinterpret_cast<unsigned char*>(iconData), iconSize);

    env->ReleaseByteArrayElements(icon, iconData, 0);
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Window, setWindowName)(JNIEnv* env, jobject self, jstring windowName)
  {
    const char* name = env->GetStringUTFChars(windowName, nullptr);
    GetEngine(env, self)->getWindow().setWindowName(name);
    env->ReleaseStringUTFChars(windowName, name);
    return self;
  }

  JNIEXPORT jdoubleArray JAVA_BIND(Window, getWorldFromDisplay)(
    JNIEnv* env, jobject self, jdoubleArray displayPoint)
  {
    double* arr = env->GetDoubleArrayElements(displayPoint, nullptr);
    f3d::point3_t worldPoint =
      GetEngine(env, self)->getWindow().getWorldFromDisplay({ arr[0], arr[1], arr[2] });
    env->ReleaseDoubleArrayElements(displayPoint, arr, 0);

    jdoubleArray ret = env->NewDoubleArray(3);
    env->SetDoubleArrayRegion(ret, 0, 3, worldPoint.data());
    return ret;
  }

  JNIEXPORT jdoubleArray JAVA_BIND(Window, getDisplayFromWorld)(
    JNIEnv* env, jobject self, jdoubleArray worldPoint)
  {
    double* arr = env->GetDoubleArrayElements(worldPoint, nullptr);
    f3d::point3_t displayPoint =
      GetEngine(env, self)->getWindow().getDisplayFromWorld({ arr[0], arr[1], arr[2] });
    env->ReleaseDoubleArrayElements(worldPoint, arr, 0);

    jdoubleArray ret = env->NewDoubleArray(3);
    env->SetDoubleArrayRegion(ret, 0, 3, displayPoint.data());
    return ret;
  }
}
