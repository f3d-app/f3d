#include "F3DJavaBindings.h"

#include <app_f3d_F3D_Options.h>

#include <options.h>

namespace
{
f3d::options& GetOptionsFromEngine(JNIEnv* env, jobject self)
{
  return GetEngine(env, self)->getOptions();
}
}

extern "C"
{
  JNIEXPORT void JAVA_BIND(Options, setAsBool)(
    JNIEnv* env, jobject self, jstring name, jboolean value)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    GetOptionsFromEngine(env, self).set(str, static_cast<bool>(value));
    env->ReleaseStringUTFChars(name, str);
  }

  JNIEXPORT void JAVA_BIND(Options, setAsInt)(JNIEnv* env, jobject self, jstring name, jint value)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    GetOptionsFromEngine(env, self).set(str, static_cast<int>(value));
    env->ReleaseStringUTFChars(name, str);
  }

  JNIEXPORT void JAVA_BIND(Options, setAsDouble)(
    JNIEnv* env, jobject self, jstring name, jdouble value)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    GetOptionsFromEngine(env, self).set(str, static_cast<double>(value));
    env->ReleaseStringUTFChars(name, str);
  }

  JNIEXPORT void JAVA_BIND(Options, setAsString)(
    JNIEnv* env, jobject self, jstring name, jstring value)
  {
    const char* nameStr = env->GetStringUTFChars(name, nullptr);
    const char* valueStr = env->GetStringUTFChars(value, nullptr);
    GetOptionsFromEngine(env, self).set(nameStr, std::string(valueStr));
    env->ReleaseStringUTFChars(name, nameStr);
    env->ReleaseStringUTFChars(value, valueStr);
  }

  JNIEXPORT void JAVA_BIND(Options, setAsDoubleVector)(
    JNIEnv* env, jobject self, jstring name, jdoubleArray values)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    jsize len = env->GetArrayLength(values);
    double* arr = env->GetDoubleArrayElements(values, nullptr);

    std::vector<double> vec(arr, arr + len);
    GetOptionsFromEngine(env, self).set(str, vec);

    env->ReleaseDoubleArrayElements(values, arr, 0);
    env->ReleaseStringUTFChars(name, str);
  }

  JNIEXPORT void JAVA_BIND(Options, setAsIntVector)(
    JNIEnv* env, jobject self, jstring name, jintArray values)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    jsize len = env->GetArrayLength(values);
    int* arr = env->GetIntArrayElements(values, nullptr);

    std::vector<int> vec(arr, arr + len);
    GetOptionsFromEngine(env, self).set(str, vec);

    env->ReleaseIntArrayElements(values, arr, 0);
    env->ReleaseStringUTFChars(name, str);
  }

  JNIEXPORT jboolean JAVA_BIND(Options, getAsBool)(JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    bool value = std::get<bool>(GetOptionsFromEngine(env, self).get(str));
    env->ReleaseStringUTFChars(name, str);
    return value;
  }

  JNIEXPORT jint JAVA_BIND(Options, getAsInt)(JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    int value = std::get<int>(GetOptionsFromEngine(env, self).get(str));
    env->ReleaseStringUTFChars(name, str);
    return value;
  }

  JNIEXPORT jdouble JAVA_BIND(Options, getAsDouble)(JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    double value = std::get<double>(GetOptionsFromEngine(env, self).get(str));
    env->ReleaseStringUTFChars(name, str);
    return value;
  }

  JNIEXPORT jstring JAVA_BIND(Options, getAsString)(JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    std::string value = std::get<std::string>(GetOptionsFromEngine(env, self).get(str));
    env->ReleaseStringUTFChars(name, str);
    return env->NewStringUTF(value.c_str());
  }

  JNIEXPORT jstring JAVA_BIND(Options, getAsStringRepresentation)(
    JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    std::string value = GetOptionsFromEngine(env, self).getAsString(str);
    env->ReleaseStringUTFChars(name, str);
    return env->NewStringUTF(value.c_str());
  }

  JNIEXPORT void JAVA_BIND(Options, setAsStringRepresentation)(
    JNIEnv* env, jobject self, jstring name, jstring str)
  {
    const char* nameStr = env->GetStringUTFChars(name, nullptr);
    const char* valueStr = env->GetStringUTFChars(str, nullptr);
    GetOptionsFromEngine(env, self).setAsString(nameStr, valueStr);
    env->ReleaseStringUTFChars(name, nameStr);
    env->ReleaseStringUTFChars(str, valueStr);
  }

  JNIEXPORT jdoubleArray JAVA_BIND(Options, getAsDoubleVector)(
    JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    std::vector<double> vec =
      std::get<std::vector<double>>(GetOptionsFromEngine(env, self).get(str));
    env->ReleaseStringUTFChars(name, str);

    jdoubleArray result = env->NewDoubleArray(vec.size());
    env->SetDoubleArrayRegion(result, 0, vec.size(), vec.data());
    return result;
  }

  JNIEXPORT jintArray JAVA_BIND(Options, getAsIntVector)(JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    std::vector<int> vec = std::get<std::vector<int>>(GetOptionsFromEngine(env, self).get(str));
    env->ReleaseStringUTFChars(name, str);

    jintArray result = env->NewIntArray(vec.size());
    env->SetIntArrayRegion(result, 0, vec.size(), vec.data());
    return result;
  }

  JNIEXPORT void JAVA_BIND(Options, toggle)(JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    GetOptionsFromEngine(env, self).toggle(str);
    env->ReleaseStringUTFChars(name, str);
  }

  JNIEXPORT jboolean JAVA_BIND(Options, isSame)(
    JNIEnv* env, jobject self, jobject other, jstring name)
  {
    jclass otherClass = env->GetObjectClass(other);
    jfieldID fid = env->GetFieldID(otherClass, "mNativeAddress", "J");
    jlong otherPtr = env->GetLongField(other, fid);

    const char* str = env->GetStringUTFChars(name, nullptr);
    bool result = GetOptionsFromEngine(env, self).isSame(
      reinterpret_cast<f3d::engine*>(otherPtr)->getOptions(), str);
    env->ReleaseStringUTFChars(name, str);
    return result;
  }

  JNIEXPORT jboolean JAVA_BIND(Options, hasValue)(JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    bool result = GetOptionsFromEngine(env, self).hasValue(str);
    env->ReleaseStringUTFChars(name, str);
    return result;
  }

  JNIEXPORT void JAVA_BIND(Options, copy)(JNIEnv* env, jobject self, jobject other, jstring name)
  {
    jclass otherClass = env->GetObjectClass(other);
    jfieldID fid = env->GetFieldID(otherClass, "mNativeAddress", "J");
    jlong otherPtr = env->GetLongField(other, fid);

    const char* str = env->GetStringUTFChars(name, nullptr);
    GetOptionsFromEngine(env, self).copy(
      reinterpret_cast<f3d::engine*>(otherPtr)->getOptions(), str);
    env->ReleaseStringUTFChars(name, str);
  }

  JNIEXPORT jobject JAVA_BIND(Options, getAllNames)(JNIEnv* env, jclass)
  {
    std::vector<std::string> names = f3d::options::getAllNames();
    return CreateStringList(env, names);
  }

  JNIEXPORT jobject JAVA_BIND(Options, getNames)(JNIEnv* env, jobject self)
  {
    f3d::options& opts = GetOptionsFromEngine(env, self);
    std::vector<std::string> names = opts.getNames();
    return CreateStringList(env, names);
  }

  JNIEXPORT jobject JAVA_BIND(Options, getClosestOption)(JNIEnv* env, jobject self, jstring option)
  {
    const char* str = env->GetStringUTFChars(option, nullptr);
    auto [name, distance] = GetOptionsFromEngine(env, self).getClosestOption(str);
    env->ReleaseStringUTFChars(option, str);

    jclass resultClass = env->FindClass("app/f3d/F3D/Options$ClosestOptionResult");
    jmethodID constructor = env->GetMethodID(resultClass, "<init>", "(Ljava/lang/String;I)V");

    jstring jname = env->NewStringUTF(name.c_str());
    jobject result = env->NewObject(resultClass, constructor, jname, static_cast<jint>(distance));
    env->DeleteLocalRef(jname);

    return result;
  }

  JNIEXPORT jboolean JAVA_BIND(Options, isOptional)(JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    bool result = GetOptionsFromEngine(env, self).isOptional(str);
    env->ReleaseStringUTFChars(name, str);
    return result;
  }

  JNIEXPORT void JAVA_BIND(Options, reset)(JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    GetOptionsFromEngine(env, self).reset(str);
    env->ReleaseStringUTFChars(name, str);
  }

  JNIEXPORT void JAVA_BIND(Options, removeValue)(JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    GetOptionsFromEngine(env, self).removeValue(str);
    env->ReleaseStringUTFChars(name, str);
  }
}
