#include "F3DJavaBindings.h"

#include <app_f3d_F3D_Camera.h>

#include <camera.h>

extern "C"
{
  JNIEXPORT jobject JAVA_BIND(Camera, setPosition)(JNIEnv* env, jobject self, jdoubleArray pos)
  {
    double* arr = env->GetDoubleArrayElements(pos, nullptr);
    GetEngine(env, self)->getWindow().getCamera().setPosition({ arr[0], arr[1], arr[2] });
    env->ReleaseDoubleArrayElements(pos, arr, 0);
    return self;
  }

  JNIEXPORT jdoubleArray JAVA_BIND(Camera, getPosition)(JNIEnv* env, jobject self)
  {
    f3d::point3_t pt = GetEngine(env, self)->getWindow().getCamera().getPosition();
    jdoubleArray ret = env->NewDoubleArray(3);
    env->SetDoubleArrayRegion(ret, 0, 3, pt.data());
    return ret;
  }

  JNIEXPORT jobject JAVA_BIND(Camera, setFocalPoint)(JNIEnv* env, jobject self, jdoubleArray foc)
  {
    double* arr = env->GetDoubleArrayElements(foc, nullptr);
    GetEngine(env, self)->getWindow().getCamera().setFocalPoint({ arr[0], arr[1], arr[2] });
    env->ReleaseDoubleArrayElements(foc, arr, 0);
    return self;
  }

  JNIEXPORT jdoubleArray JAVA_BIND(Camera, getFocalPoint)(JNIEnv* env, jobject self)
  {
    f3d::point3_t pt = GetEngine(env, self)->getWindow().getCamera().getFocalPoint();
    jdoubleArray ret = env->NewDoubleArray(3);
    env->SetDoubleArrayRegion(ret, 0, 3, pt.data());
    return ret;
  }

  JNIEXPORT jdouble JAVA_BIND(Camera, getWorldAzimuth)(JNIEnv* env, jobject self)
  {
    return GetEngine(env, self)->getWindow().getCamera().getWorldAzimuth();
  }

  JNIEXPORT jdouble JAVA_BIND(Camera, getWorldElevation)(JNIEnv* env, jobject self)
  {
    return GetEngine(env, self)->getWindow().getCamera().getWorldElevation();
  }

  JNIEXPORT jdouble JAVA_BIND(Camera, getDistance)(JNIEnv* env, jobject self)
  {
    return GetEngine(env, self)->getWindow().getCamera().getDistance();
  }

  JNIEXPORT jobject JAVA_BIND(Camera, setViewUp)(JNIEnv* env, jobject self, jdoubleArray up)
  {
    double* arr = env->GetDoubleArrayElements(up, nullptr);
    GetEngine(env, self)->getWindow().getCamera().setViewUp({ arr[0], arr[1], arr[2] });
    env->ReleaseDoubleArrayElements(up, arr, 0);
    return self;
  }

  JNIEXPORT jdoubleArray JAVA_BIND(Camera, getViewUp)(JNIEnv* env, jobject self)
  {
    f3d::vector3_t vec = GetEngine(env, self)->getWindow().getCamera().getViewUp();
    jdoubleArray ret = env->NewDoubleArray(3);
    env->SetDoubleArrayRegion(ret, 0, 3, vec.data());
    return ret;
  }

  JNIEXPORT jobject JAVA_BIND(Camera, setViewAngle)(JNIEnv* env, jobject self, jdouble angle)
  {
    GetEngine(env, self)->getWindow().getCamera().setViewAngle(angle);
    return self;
  }

  JNIEXPORT jdouble JAVA_BIND(Camera, getViewAngle)(JNIEnv* env, jobject self)
  {
    return GetEngine(env, self)->getWindow().getCamera().getViewAngle();
  }

  JNIEXPORT jobject JAVA_BIND(Camera, setState)(JNIEnv* env, jobject self, jobject state)
  {
    jclass stateClass = env->GetObjectClass(state);

    jfieldID positionField = env->GetFieldID(stateClass, "position", "[D");
    jfieldID focalPointField = env->GetFieldID(stateClass, "focalPoint", "[D");
    jfieldID viewUpField = env->GetFieldID(stateClass, "viewUp", "[D");
    jfieldID viewAngleField = env->GetFieldID(stateClass, "viewAngle", "D");

    jdoubleArray posArray = static_cast<jdoubleArray>(env->GetObjectField(state, positionField));
    jdoubleArray focArray = static_cast<jdoubleArray>(env->GetObjectField(state, focalPointField));
    jdoubleArray upArray = static_cast<jdoubleArray>(env->GetObjectField(state, viewUpField));
    jdouble angle = env->GetDoubleField(state, viewAngleField);

    double* pos = env->GetDoubleArrayElements(posArray, nullptr);
    double* foc = env->GetDoubleArrayElements(focArray, nullptr);
    double* up = env->GetDoubleArrayElements(upArray, nullptr);

    f3d::camera_state_t cppState;
    cppState.position = { pos[0], pos[1], pos[2] };
    cppState.focalPoint = { foc[0], foc[1], foc[2] };
    cppState.viewUp = { up[0], up[1], up[2] };
    cppState.viewAngle = angle;

    GetEngine(env, self)->getWindow().getCamera().setState(cppState);

    env->ReleaseDoubleArrayElements(posArray, pos, 0);
    env->ReleaseDoubleArrayElements(focArray, foc, 0);
    env->ReleaseDoubleArrayElements(upArray, up, 0);

    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Camera, getState)(JNIEnv* env, jobject self)
  {
    f3d::camera_state_t cppState = GetEngine(env, self)->getWindow().getCamera().getState();

    jclass stateClass = env->FindClass("app/f3d/F3D/Camera$CameraState");
    jmethodID constructor = env->GetMethodID(stateClass, "<init>", "()V");
    jobject state = env->NewObject(stateClass, constructor);

    jfieldID positionField = env->GetFieldID(stateClass, "position", "[D");
    jfieldID focalPointField = env->GetFieldID(stateClass, "focalPoint", "[D");
    jfieldID viewUpField = env->GetFieldID(stateClass, "viewUp", "[D");
    jfieldID viewAngleField = env->GetFieldID(stateClass, "viewAngle", "D");

    jdoubleArray posArray = env->NewDoubleArray(3);
    jdoubleArray focArray = env->NewDoubleArray(3);
    jdoubleArray upArray = env->NewDoubleArray(3);

    env->SetDoubleArrayRegion(posArray, 0, 3, cppState.position.data());
    env->SetDoubleArrayRegion(focArray, 0, 3, cppState.focalPoint.data());
    env->SetDoubleArrayRegion(upArray, 0, 3, cppState.viewUp.data());

    env->SetObjectField(state, positionField, posArray);
    env->SetObjectField(state, focalPointField, focArray);
    env->SetObjectField(state, viewUpField, upArray);
    env->SetDoubleField(state, viewAngleField, cppState.viewAngle);

    return state;
  }

  JNIEXPORT jobject JAVA_BIND(Camera, dolly)(JNIEnv* env, jobject self, jdouble val)
  {
    GetEngine(env, self)->getWindow().getCamera().dolly(val);
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Camera, pan)(
    JNIEnv* env, jobject self, jdouble right, jdouble up, jdouble forward)
  {
    GetEngine(env, self)->getWindow().getCamera().pan(right, up, forward);
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Camera, zoom)(JNIEnv* env, jobject self, jdouble factor)
  {
    GetEngine(env, self)->getWindow().getCamera().zoom(factor);
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Camera, roll)(JNIEnv* env, jobject self, jdouble angle)
  {
    GetEngine(env, self)->getWindow().getCamera().roll(angle);
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Camera, azimuth)(JNIEnv* env, jobject self, jdouble angle)
  {
    GetEngine(env, self)->getWindow().getCamera().azimuth(angle);
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Camera, yaw)(JNIEnv* env, jobject self, jdouble angle)
  {
    GetEngine(env, self)->getWindow().getCamera().yaw(angle);
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Camera, elevation)(JNIEnv* env, jobject self, jdouble angle)
  {
    GetEngine(env, self)->getWindow().getCamera().elevation(angle);
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Camera, pitch)(JNIEnv* env, jobject self, jdouble angle)
  {
    GetEngine(env, self)->getWindow().getCamera().pitch(angle);
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Camera, setCurrentAsDefault)(JNIEnv* env, jobject self)
  {
    GetEngine(env, self)->getWindow().getCamera().setCurrentAsDefault();
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Camera, resetToDefault)(JNIEnv* env, jobject self)
  {
    GetEngine(env, self)->getWindow().getCamera().resetToDefault();
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Camera, resetToBounds)(JNIEnv* env, jobject self, jdouble zoomFactor)
  {
    GetEngine(env, self)->getWindow().getCamera().resetToBounds(zoomFactor);
    return self;
  }
}
