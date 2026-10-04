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

  JNIEXPORT jdoubleArray JAVA_BIND(Animation, timeRange)(JNIEnv* env, jobject self)
  {
    auto [minTime, maxTime] = GetEngine(env, self)->getScene().getAnimation().timeRange();

    jdoubleArray result = env->NewDoubleArray(2);
    double timeRange[] = { minTime, maxTime };
    env->SetDoubleArrayRegion(result, 0, 2, timeRange);

    return result;
  }

  JNIEXPORT jdoubleArray JAVA_BIND(Animation, keyFrames)(JNIEnv* env, jobject self)
  {
    auto keyframeVec = GetEngine(env, self)->getScene().getAnimation().keyFrames();
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
}
