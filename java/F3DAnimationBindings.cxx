#include "F3DJavaBindings.h"

#include <app_f3d_F3D_Animation.h>

#include <animation.h>
#include <types.h>

static std::vector<std::string> JavaListToStringVector(JNIEnv* env, jobject list)
{
  std::vector<std::string> vec;

  const JniLocalRef<jclass> listClass(env, env->GetObjectClass(list));
  jmethodID sizeMethod = env->GetMethodID(listClass, "size", "()I");
  jmethodID getMethod = env->GetMethodID(listClass, "get", "(I)Ljava/lang/Object;");

  const jint size = env->CallIntMethod(list, sizeMethod);

  for (jint i = 0; i < size; i++)
  {
    const JniLocalRef<jstring> jstr(
      env, static_cast<jstring>(env->CallObjectMethod(list, getMethod, i)));
    if (jstr.get())
    {
      const JniUTFString str(env, jstr);
      vec.push_back(str.c_str());
    }
  }

  return vec;
}

extern "C"
{
  JNIEXPORT jobject JAVA_BIND(Animation, loadTime)(JNIEnv* env, jobject self, jdouble timeValue)
  {
    GetEngine(env, self)->getScene().getAnimation().loadTime(timeValue);
    return self;
  }

  JNIEXPORT jdoubleArray JAVA_BIND(Animation, getTimeRange)(JNIEnv* env, jobject self)
  {
    auto [minTime, maxTime] = GetEngine(env, self)->getScene().getAnimation().getTimeRange();

    jdoubleArray result = env->NewDoubleArray(2);
    jdouble timeRange[] = { minTime, maxTime };
    env->SetDoubleArrayRegion(result, 0, 2, timeRange);

    return result;
  }

  JNIEXPORT jdoubleArray JAVA_BIND(Animation, getKeyFrames)(JNIEnv* env, jobject self)
  {
    auto keyframeVec = GetEngine(env, self)->getScene().getAnimation().getKeyFrames();
    jdoubleArray result = env->NewDoubleArray(keyframeVec.size());
    const jdouble* keyframes = keyframeVec.data();
    env->SetDoubleArrayRegion(result, 0, keyframeVec.size(), keyframes);
    return result;
  }

  JNIEXPORT jint JAVA_BIND(Animation, count)(JNIEnv* env, jobject self)
  {
    return GetEngine(env, self)->getScene().getAnimation().count();
  }

  JNIEXPORT jstring JAVA_BIND(Animation, getCurrentName)(JNIEnv* env, jobject self)
  {
    return env->NewStringUTF(GetEngine(env, self)->getScene().getAnimation().getName().c_str());
  }

  JNIEXPORT jstring JAVA_BIND(Animation, getName)(JNIEnv* env, jobject self, jint index)
  {
    return env->NewStringUTF(
      GetEngine(env, self)->getScene().getAnimation().getName(index).c_str());
  }

  JNIEXPORT jobject JAVA_BIND(Animation, getNames)(JNIEnv* env, jobject self)
  {
    return CreateStringList(env, GetEngine(env, self)->getScene().getAnimation().getNames());
  }

  JNIEXPORT jobject JAVA_BIND(Animation, toggle)(JNIEnv* env, jobject self, jobject direction)
  {
    const JniLocalRef<jclass> directionEnum(env, env->GetObjectClass(direction));
    jmethodID getValueMethod = env->GetMethodID(directionEnum, "getValue", "()I");
    const jint directionValue = env->CallIntMethod(direction, getValueMethod);

    const f3d::animation::Direction nativeDirection =
      static_cast<f3d::animation::Direction>(directionValue);

    GetEngine(env, self)->getScene().getAnimation().toggle(nativeDirection);
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Animation, start)(JNIEnv* env, jobject self, jobject direction)
  {
    const JniLocalRef<jclass> directionEnum(env, env->GetObjectClass(direction));
    jmethodID getValueMethod = env->GetMethodID(directionEnum, "getValue", "()I");
    const jint directionValue = env->CallIntMethod(direction, getValueMethod);

    const f3d::animation::Direction nativeDirection =
      static_cast<f3d::animation::Direction>(directionValue);

    GetEngine(env, self)->getScene().getAnimation().start(nativeDirection);
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Animation, stop)(JNIEnv* env, jobject self)
  {
    GetEngine(env, self)->getScene().getAnimation().stop();
    return self;
  }

  JNIEXPORT jboolean JAVA_BIND(Animation, isPlaying)(JNIEnv* env, jobject self)
  {
    return GetEngine(env, self)->getScene().getAnimation().isPlaying();
  }

  JNIEXPORT jobject JAVA_BIND(Animation, getDirection)(JNIEnv* env, jobject self)
  {
    const f3d::animation::Direction nativeDirection =
      GetEngine(env, self)->getScene().getAnimation().getDirection();

    const JniLocalRef<jclass> enumClass(env, env->FindClass("app/f3d/F3D/Animation$Direction"));
    jmethodID fromValueMethod =
      env->GetStaticMethodID(enumClass, "fromValue", "(I)Lapp/f3d/F3D/Animation$Direction;");

    return env->CallStaticObjectMethod(
      enumClass, fromValueMethod, static_cast<int>(nativeDirection));
  }
}
