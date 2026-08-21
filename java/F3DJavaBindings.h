#ifndef F3D_JAVA_BINDINGS_H
#define F3D_JAVA_BINDINGS_H

#include <jni.h>

#include <engine.h>
#include <image.h>

#include <map>
#include <string>
#include <vector>

#define JAVA_BIND(Cls, Func) JNICALL Java_app_f3d_F3D_##Cls##_##Func

// Same as JAVA_BIND but for a method of a nested class (00024 is the JNI encoding of the '$' scope
// separator, e.g. Engine$State)
#define JAVA_SCOPED_BIND(Cls, Nested, Func) JNICALL Java_app_f3d_F3D_##Cls##_00024##Nested##_##Func

namespace fs = std::filesystem;

/**
 * RAII helper wrapping GetStringUTFChars/ReleaseStringUTFChars.
 * The underlying C string is released automatically when the wrapper goes
 * out of scope, including on early returns or exceptions, so call sites can
 * no longer leak a UTF-8 string by forgetting (or being unable, due to an
 * exception) to call ReleaseStringUTFChars.
 *
 * A null jstring is handled gracefully: c_str() returns nullptr and no JNI
 * call is made.
 */
class JniUTFString
{
public:
  JniUTFString(JNIEnv* env, jstring jstr)
    : Env(env)
    , JStr(jstr)
    , CStr(jstr ? env->GetStringUTFChars(jstr, nullptr) : nullptr)
  {
  }

  ~JniUTFString()
  {
    if (this->CStr)
    {
      this->Env->ReleaseStringUTFChars(this->JStr, this->CStr);
    }
  }

  JniUTFString(const JniUTFString&) = delete;
  JniUTFString& operator=(const JniUTFString&) = delete;
  JniUTFString(JniUTFString&&) = delete;
  JniUTFString& operator=(JniUTFString&&) = delete;

  operator const char*() const
  {
    return this->CStr;
  }
  const char* c_str() const
  {
    return this->CStr;
  }

private:
  JNIEnv* Env;
  jstring JStr;
  const char* CStr;
};

/**
 * RAII helper for a local JNI reference (jclass, jobject, jstring, ...)
 * that must eventually be released with DeleteLocalRef. Deletes the local
 * reference automatically when the wrapper goes out of scope.
 *
 * A null reference is handled gracefully: no JNI call is made on destruction.
 */
template<typename T>
class JniLocalRef
{
public:
  JniLocalRef(JNIEnv* env, T ref)
    : Env(env)
    , Ref(ref)
  {
  }

  ~JniLocalRef()
  {
    if (this->Ref)
    {
      this->Env->DeleteLocalRef(this->Ref);
    }
  }

  JniLocalRef(const JniLocalRef&) = delete;
  JniLocalRef& operator=(const JniLocalRef&) = delete;
  JniLocalRef(JniLocalRef&&) = delete;
  JniLocalRef& operator=(JniLocalRef&&) = delete;

  operator T() const
  {
    return this->Ref;
  }
  T get() const
  {
    return this->Ref;
  }

private:
  JNIEnv* Env;
  T Ref;
};

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
  JniLocalRef<jclass> cls(env, env->FindClass(className));
  if (cls.get())
  {
    env->ThrowNew(cls, msg);
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

// Helper function to get the f3d::image pointer from a Java object
inline f3d::image* GetImage(JNIEnv* env, jobject self)
{
  JniLocalRef<jclass> cls(env, env->GetObjectClass(self));
  jfieldID fid = env->GetFieldID(cls, "mNativeAddress", "J");
  jlong ptr = env->GetLongField(self, fid);

  return reinterpret_cast<f3d::image*>(ptr);
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
    JniLocalRef<jstring> jstr(env, env->NewStringUTF(str.c_str()));
    env->CallBooleanMethod(list, addMethod, jstr.get());
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
    JniLocalRef<jstring> jkey(env, env->NewStringUTF(key.c_str()));
    JniLocalRef<jobject> jvalue(env, env->NewObject(booleanClass, booleanConstructor, value));
    env->CallObjectMethod(jmap, putMethod, jkey.get(), jvalue.get());
  }

  return jmap;
}

#endif // F3D_JAVA_BINDINGS_H
