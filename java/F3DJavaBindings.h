#ifndef F3D_JAVA_BINDINGS_H
#define F3D_JAVA_BINDINGS_H

#include <jni.h>

#include <engine.h>

#include <map>
#include <string>
#include <vector>

#define JAVA_BIND(Cls, Func) JNICALL Java_app_f3d_F3D_##Cls##_##Func

namespace fs = std::filesystem;

// Helper function to get the f3d::engine pointer from a Java object
inline f3d::engine* GetEngine(JNIEnv* env, jobject self)
{
  jclass cls = env->GetObjectClass(self);
  jfieldID fid = env->GetFieldID(cls, "mNativeAddress", "J");
  jlong ptr = env->GetLongField(self, fid);

  return reinterpret_cast<f3d::engine*>(ptr);
}

// Helper function to convert std::vector<std::string> to Java List
inline jobject CreateStringList(JNIEnv* env, const std::vector<std::string>& vec)
{
  jclass arrayListClass = env->FindClass("java/util/ArrayList");
  jmethodID arrayListConstructor = env->GetMethodID(arrayListClass, "<init>", "()V");
  jmethodID addMethod = env->GetMethodID(arrayListClass, "add", "(Ljava/lang/Object;)Z");

  jobject list = env->NewObject(arrayListClass, arrayListConstructor);

  for (const auto& str : vec)
  {
    jstring jstr = env->NewStringUTF(str.c_str());
    env->CallBooleanMethod(list, addMethod, jstr);
    env->DeleteLocalRef(jstr);
  }

  return list;
}

// Helper function to convert std::vector<double> to Java List
inline jobject CreateDoubleList(JNIEnv* env, const std::vector<double>& vec)
{
  jclass arrayListClass = env->FindClass("java/util/ArrayList");
  jmethodID arrayListConstructor = env->GetMethodID(arrayListClass, "<init>", "()V");
  jmethodID addMethod = env->GetMethodID(arrayListClass, "add", "(Ljava/lang/Object;)Z");

  jclass doubleClass = env->FindClass("java/lang/Double");
  jmethodID doubleConstructor = env->GetMethodID(doubleClass, "<init>", "(D)V");

  jobject list = env->NewObject(arrayListClass, arrayListConstructor);

  for (const auto& d : vec)
  {
    jobject jvalue = env->NewObject(doubleClass, doubleConstructor, (jdouble)d);
    env->CallBooleanMethod(list, addMethod, jvalue);
    env->DeleteLocalRef(jvalue);
  }

  // Optional but good practice
  env->DeleteLocalRef(doubleClass);
  env->DeleteLocalRef(arrayListClass);

  return list;
}

// Helper function to convert std::map<std::string, bool> to Java Map
inline jobject CreateStringBooleanMap(JNIEnv* env, const std::map<std::string, bool>& map)
{
  jclass hashMapClass = env->FindClass("java/util/HashMap");
  jmethodID hashMapConstructor = env->GetMethodID(hashMapClass, "<init>", "()V");
  jmethodID putMethod = env->GetMethodID(
    hashMapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

  jclass booleanClass = env->FindClass("java/lang/Boolean");
  jmethodID booleanConstructor = env->GetMethodID(booleanClass, "<init>", "(Z)V");

  jobject jmap = env->NewObject(hashMapClass, hashMapConstructor);

  for (const auto& [key, value] : map)
  {
    jstring jkey = env->NewStringUTF(key.c_str());
    jobject jvalue = env->NewObject(booleanClass, booleanConstructor, value);
    env->CallObjectMethod(jmap, putMethod, jkey, jvalue);
    env->DeleteLocalRef(jkey);
    env->DeleteLocalRef(jvalue);
  }

  return jmap;
}

#endif // F3D_JAVA_BINDINGS_H
