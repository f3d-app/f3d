#ifndef F3D_JAVA_BINDINGS_H
#define F3D_JAVA_BINDINGS_H

#include <jni.h>

#include <engine.h>

#include <map>
#include <string>
#include <vector>

#define JAVA_BIND(Cls, Func) JNICALL Java_app_f3d_F3D_##Cls##_##Func

// Same as JAVA_BIND but for a method of a nested class (00024 is the JNI encoding of the '$' scope
// separator, e.g. Engine$State)
#define JAVA_SCOPED_BIND(Cls, Nested, Func) JNICALL Java_app_f3d_F3D_##Cls##_00024##Nested##_##Func

namespace fs = std::filesystem;

/**
 * Throw a Java exception of the given class name with the given message.
 * Call this from a catch block, then immediately return from the JNI function
 * so that the pending Java exception is delivered to the caller.
 *
 * className uses JNI slash-notation, e.g.
 *   "app/f3d/F3D/Engine$NoInteractorException"
 *   "java/lang/RuntimeException"
 */
inline void F3DThrowJavaException(JNIEnv* env, const char* className, const char* msg)
{
  jclass cls = env->FindClass(className);
  if (cls)
  {
    env->ThrowNew(cls, msg);
    env->DeleteLocalRef(cls);
  }
}

// Helper function to get the f3d::engine pointer from a Java object
inline f3d::engine* GetEngine(JNIEnv* env, jobject self)
{
  jclass cls = env->GetObjectClass(self);
  jfieldID fid = env->GetFieldID(cls, "mNativeAddress", "J");
  jlong ptr = env->GetLongField(self, fid);

  return reinterpret_cast<f3d::engine*>(ptr);
}

// Helper function to get the f3d::engine::state pointer from a Java object
inline f3d::engine::state* GetState(JNIEnv* env, jobject self)
{
  jclass cls = env->GetObjectClass(self);
  jfieldID fid = env->GetFieldID(cls, "mNativeAddress", "J");
  jlong ptr = env->GetLongField(self, fid);

  return reinterpret_cast<f3d::engine::state*>(ptr);
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
