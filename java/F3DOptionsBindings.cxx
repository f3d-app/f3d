#include "F3DJavaBindings.h"

#include <app_f3d_F3D_Options.h>

#include <options.h>

#include <cassert>
#include <stdexcept>
#include <variant>

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
    try
    {
      GetOptionsFromEngine(env, self).set(str, static_cast<bool>(value));
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::incompatible_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
    env->ReleaseStringUTFChars(name, str);
  }

  JNIEXPORT void JAVA_BIND(Options, setAsInt)(JNIEnv* env, jobject self, jstring name, jint value)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    try
    {
      GetOptionsFromEngine(env, self).set(str, static_cast<int>(value));
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::incompatible_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
    env->ReleaseStringUTFChars(name, str);
  }

  JNIEXPORT void JAVA_BIND(Options, setAsDouble)(
    JNIEnv* env, jobject self, jstring name, jdouble value)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    try
    {
      GetOptionsFromEngine(env, self).set(str, static_cast<double>(value));
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::incompatible_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
    env->ReleaseStringUTFChars(name, str);
  }

  JNIEXPORT void JAVA_BIND(Options, setAsString)(
    JNIEnv* env, jobject self, jstring name, jstring value)
  {
    const char* nameStr = env->GetStringUTFChars(name, nullptr);
    const char* valueStr = env->GetStringUTFChars(value, nullptr);
    try
    {
      GetOptionsFromEngine(env, self).set(nameStr, std::string(valueStr));
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::incompatible_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
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
    env->ReleaseDoubleArrayElements(values, arr, 0);

    try
    {
      GetOptionsFromEngine(env, self).set(str, vec);
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::incompatible_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
    env->ReleaseStringUTFChars(name, str);
  }

  JNIEXPORT void JAVA_BIND(Options, setAsIntVector)(
    JNIEnv* env, jobject self, jstring name, jintArray values)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    jsize len = env->GetArrayLength(values);
    int* arr = env->GetIntArrayElements(values, nullptr);
    std::vector<int> vec(arr, arr + len);
    env->ReleaseIntArrayElements(values, arr, 0);

    try
    {
      GetOptionsFromEngine(env, self).set(str, vec);
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::incompatible_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
    env->ReleaseStringUTFChars(name, str);
  }

  JNIEXPORT jboolean JAVA_BIND(Options, getAsBool)(JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    try
    {
      bool value = std::get<bool>(GetOptionsFromEngine(env, self).get(str));
      env->ReleaseStringUTFChars(name, str);
      return value;
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      env->ReleaseStringUTFChars(name, str);
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::no_value_exception& e)
    {
      env->ReleaseStringUTFChars(name, str);
      F3DThrowJavaException(env, "app/f3d/F3D/Options$NoValueException", e.what());
    }
    catch (const std::bad_variant_access& e)
    {
      env->ReleaseStringUTFChars(name, str);
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
    return false;
  }

  JNIEXPORT jint JAVA_BIND(Options, getAsInt)(JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    try
    {
      int value = std::get<int>(GetOptionsFromEngine(env, self).get(str));
      env->ReleaseStringUTFChars(name, str);
      return value;
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      env->ReleaseStringUTFChars(name, str);
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::no_value_exception& e)
    {
      env->ReleaseStringUTFChars(name, str);
      F3DThrowJavaException(env, "app/f3d/F3D/Options$NoValueException", e.what());
    }
    catch (const std::bad_variant_access& e)
    {
      env->ReleaseStringUTFChars(name, str);
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
    return 0;
  }

  JNIEXPORT jdouble JAVA_BIND(Options, getAsDouble)(JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    try
    {
      double value = std::get<double>(GetOptionsFromEngine(env, self).get(str));
      env->ReleaseStringUTFChars(name, str);
      return value;
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      env->ReleaseStringUTFChars(name, str);
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::no_value_exception& e)
    {
      env->ReleaseStringUTFChars(name, str);
      F3DThrowJavaException(env, "app/f3d/F3D/Options$NoValueException", e.what());
    }
    catch (const std::bad_variant_access& e)
    {
      env->ReleaseStringUTFChars(name, str);
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
    return 0.0;
  }

  JNIEXPORT jstring JAVA_BIND(Options, getAsString)(JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    try
    {
      std::string value = std::get<std::string>(GetOptionsFromEngine(env, self).get(str));
      env->ReleaseStringUTFChars(name, str);
      return env->NewStringUTF(value.c_str());
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      env->ReleaseStringUTFChars(name, str);
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::no_value_exception& e)
    {
      env->ReleaseStringUTFChars(name, str);
      F3DThrowJavaException(env, "app/f3d/F3D/Options$NoValueException", e.what());
    }
    catch (const std::bad_variant_access& e)
    {
      env->ReleaseStringUTFChars(name, str);
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
    return nullptr;
  }

  JNIEXPORT jstring JAVA_BIND(Options, getAsStringRepresentation)(
    JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    try
    {
      std::string value = GetOptionsFromEngine(env, self).getAsString(str);
      env->ReleaseStringUTFChars(name, str);
      return env->NewStringUTF(value.c_str());
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      env->ReleaseStringUTFChars(name, str);
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::no_value_exception& e)
    {
      env->ReleaseStringUTFChars(name, str);
      F3DThrowJavaException(env, "app/f3d/F3D/Options$NoValueException", e.what());
    }
    return nullptr;
  }

  JNIEXPORT void JAVA_BIND(Options, setAsStringRepresentation)(
    JNIEnv* env, jobject self, jstring name, jstring str)
  {
    const char* nameStr = env->GetStringUTFChars(name, nullptr);
    const char* valueStr = env->GetStringUTFChars(str, nullptr);
    try
    {
      GetOptionsFromEngine(env, self).setAsString(nameStr, valueStr);
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::parsing_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$ParsingException", e.what());
    }
    env->ReleaseStringUTFChars(name, nameStr);
    env->ReleaseStringUTFChars(str, valueStr);
  }

  JNIEXPORT jdoubleArray JAVA_BIND(Options, getAsDoubleVector)(
    JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    try
    {
      std::vector<double> vec =
        std::get<std::vector<double>>(GetOptionsFromEngine(env, self).get(str));
      env->ReleaseStringUTFChars(name, str);

      jdoubleArray result = env->NewDoubleArray(vec.size());
      env->SetDoubleArrayRegion(result, 0, vec.size(), vec.data());
      return result;
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      env->ReleaseStringUTFChars(name, str);
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::no_value_exception& e)
    {
      env->ReleaseStringUTFChars(name, str);
      F3DThrowJavaException(env, "app/f3d/F3D/Options$NoValueException", e.what());
    }
    catch (const std::bad_variant_access& e)
    {
      env->ReleaseStringUTFChars(name, str);
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
    return nullptr;
  }

  JNIEXPORT jintArray JAVA_BIND(Options, getAsIntVector)(JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    try
    {
      std::vector<int> vec = std::get<std::vector<int>>(GetOptionsFromEngine(env, self).get(str));
      env->ReleaseStringUTFChars(name, str);

      jintArray result = env->NewIntArray(vec.size());
      env->SetIntArrayRegion(result, 0, vec.size(), vec.data());
      return result;
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      env->ReleaseStringUTFChars(name, str);
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::no_value_exception& e)
    {
      env->ReleaseStringUTFChars(name, str);
      F3DThrowJavaException(env, "app/f3d/F3D/Options$NoValueException", e.what());
    }
    catch (const std::bad_variant_access& e)
    {
      env->ReleaseStringUTFChars(name, str);
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
    return nullptr;
  }

  JNIEXPORT void JAVA_BIND(Options, toggle)(JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    try
    {
      GetOptionsFromEngine(env, self).toggle(str);
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::incompatible_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
    env->ReleaseStringUTFChars(name, str);
  }

  JNIEXPORT jboolean JAVA_BIND(Options, isSame)(
    JNIEnv* env, jobject self, jobject other, jstring name)
  {
    jclass otherClass = env->GetObjectClass(other);
    jfieldID fid = env->GetFieldID(otherClass, "mNativeAddress", "J");
    jlong otherPtr = env->GetLongField(other, fid);

    const char* str = env->GetStringUTFChars(name, nullptr);
    try
    {
      bool result = GetOptionsFromEngine(env, self).isSame(
        reinterpret_cast<f3d::engine*>(otherPtr)->getOptions(), str);
      env->ReleaseStringUTFChars(name, str);
      return result;
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      env->ReleaseStringUTFChars(name, str);
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    return false;
  }

  JNIEXPORT jboolean JAVA_BIND(Options, hasValue)(JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    try
    {
      bool result = GetOptionsFromEngine(env, self).hasValue(str);
      env->ReleaseStringUTFChars(name, str);
      return result;
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      env->ReleaseStringUTFChars(name, str);
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    return false;
  }

  JNIEXPORT void JAVA_BIND(Options, copy)(JNIEnv* env, jobject self, jobject other, jstring name)
  {
    jclass otherClass = env->GetObjectClass(other);
    jfieldID fid = env->GetFieldID(otherClass, "mNativeAddress", "J");
    jlong otherPtr = env->GetLongField(other, fid);

    const char* str = env->GetStringUTFChars(name, nullptr);
    try
    {
      GetOptionsFromEngine(env, self).copy(
        reinterpret_cast<f3d::engine*>(otherPtr)->getOptions(), str);
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
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
    try
    {
      bool result = GetOptionsFromEngine(env, self).isOptional(str);
      env->ReleaseStringUTFChars(name, str);
      return result;
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      env->ReleaseStringUTFChars(name, str);
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    return false;
  }

  JNIEXPORT jobject JAVA_BIND(Options, getType)(JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    try
    {
      f3d::options::option_type type = GetOptionsFromEngine(env, self).getType(str);
      env->ReleaseStringUTFChars(name, str);

      const char* enumName = nullptr;
      switch (type)
      {
        case f3d::options::option_type::BOOL:
          enumName = "BOOL";
          break;
        case f3d::options::option_type::INT:
          enumName = "INT";
          break;
        case f3d::options::option_type::DOUBLE:
          enumName = "DOUBLE";
          break;
        case f3d::options::option_type::RATIO:
          enumName = "RATIO";
          break;
        case f3d::options::option_type::STRING:
          enumName = "STRING";
          break;
        case f3d::options::option_type::PATH:
          enumName = "PATH";
          break;
        case f3d::options::option_type::COLOR:
          enumName = "COLOR";
          break;
        case f3d::options::option_type::DIRECTION:
          enumName = "DIRECTION";
          break;
        case f3d::options::option_type::COLORMAP:
          enumName = "COLORMAP";
          break;
        case f3d::options::option_type::TRANSFORM2D:
          enumName = "TRANSFORM2D";
          break;
        case f3d::options::option_type::DOUBLE_VECTOR:
          enumName = "DOUBLE_VECTOR";
          break;
        case f3d::options::option_type::INT_VECTOR:
          enumName = "INT_VECTOR";
          break;
        default:
          // Unreachable
          assert(false);
      }

      jclass typeClass = env->FindClass("app/f3d/F3D/Options$OptionType");
      jfieldID fid = env->GetStaticFieldID(typeClass, enumName, "Lapp/f3d/F3D/Options$OptionType;");
      return env->GetStaticObjectField(typeClass, fid);
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      env->ReleaseStringUTFChars(name, str);
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    return nullptr;
  }

  JNIEXPORT void JAVA_BIND(Options, reset)(JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    try
    {
      GetOptionsFromEngine(env, self).reset(str);
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    env->ReleaseStringUTFChars(name, str);
  }

  JNIEXPORT void JAVA_BIND(Options, removeValue)(JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    try
    {
      GetOptionsFromEngine(env, self).removeValue(str);
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::incompatible_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
    env->ReleaseStringUTFChars(name, str);
  }

  JNIEXPORT jboolean JAVA_BIND(Options, hasDomain)(JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    try
    {
      bool result = GetOptionsFromEngine(env, self).hasDomain(str);
      env->ReleaseStringUTFChars(name, str);
      return result;
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      env->ReleaseStringUTFChars(name, str);
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    return false;
  }

  JNIEXPORT jobject JAVA_BIND(Options, getDomainStyle)(JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    try
    {
      f3d::options::domain_style ds = GetOptionsFromEngine(env, self).getDomainStyle(str);
      env->ReleaseStringUTFChars(name, str);

      jclass enumClass = env->FindClass("app/f3d/F3D/Options$DomainStyle");
      jfieldID fieldID;

      switch (ds)
      {
        case f3d::options::domain_style::RANGE:
          fieldID = env->GetStaticFieldID(enumClass, "RANGE", "Lapp/f3d/F3D/Options$DomainStyle;");
          break;
        case f3d::options::domain_style::ENUM:
          fieldID = env->GetStaticFieldID(enumClass, "ENUM", "Lapp/f3d/F3D/Options$DomainStyle;");
          break;
        default:
        case f3d::options::domain_style::INDEX:
          fieldID = env->GetStaticFieldID(enumClass, "INDEX", "Lapp/f3d/F3D/Options$DomainStyle;");
          break;
      }
      return env->GetStaticObjectField(enumClass, fieldID);
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      env->ReleaseStringUTFChars(name, str);
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    return nullptr;
  }

  JNIEXPORT jobject JAVA_BIND(Options, getEnumDomain)(JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    try
    {
      jobject enumeration =
        CreateStringList(env, GetOptionsFromEngine(env, self).getEnumDomain(str));
      env->ReleaseStringUTFChars(name, str);
      return enumeration;
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      env->ReleaseStringUTFChars(name, str);
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::incompatible_exception& e)
    {
      env->ReleaseStringUTFChars(name, str);
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
    return nullptr;
  }

  JNIEXPORT jobject JAVA_BIND(Options, getRangeDomainAsDouble)(
    JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    const std::string nameStr = str;
    env->ReleaseStringUTFChars(name, str);

    jclass rangeClass = env->FindClass("app/f3d/F3D/Options$DomainRange");
    jmethodID rangeCtor = env->GetMethodID(
      rangeClass, "<init>", "(Ljava/lang/Number;Ljava/lang/Number;Ljava/lang/Number;)V");

    try
    {
      f3d::options::DomainRange<f3d::option_variant_t> range =
        GetOptionsFromEngine(env, self).getRangeDomain(nameStr);

      // Double matches both double and ratio domains, as ratio is exposed as double
      if (!std::holds_alternative<double>(range.min))
      {
        throw std::invalid_argument(
          "Trying to get range domain of " + nameStr + " as a Double but it is an Integer domain");
      }

      jclass doubleClass = env->FindClass("java/lang/Double");
      jmethodID valueOf = env->GetStaticMethodID(doubleClass, "valueOf", "(D)Ljava/lang/Double;");
      return env->NewObject(rangeClass, rangeCtor,
        env->CallStaticObjectMethod(doubleClass, valueOf, std::get<double>(range.min)),
        env->CallStaticObjectMethod(doubleClass, valueOf, std::get<double>(range.max)),
        env->CallStaticObjectMethod(doubleClass, valueOf, std::get<double>(range.increment)));
    }
    catch (const std::exception& e)
    {
      env->ThrowNew(env->FindClass("java/lang/IllegalArgumentException"), e.what());
      return nullptr;
    }
  }

  JNIEXPORT jobject JAVA_BIND(Options, getRangeDomainAsInt)(JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    const std::string nameStr = str;
    env->ReleaseStringUTFChars(name, str);

    jclass rangeClass = env->FindClass("app/f3d/F3D/Options$DomainRange");
    jmethodID rangeCtor = env->GetMethodID(
      rangeClass, "<init>", "(Ljava/lang/Number;Ljava/lang/Number;Ljava/lang/Number;)V");

    try
    {
      f3d::options::DomainRange<f3d::option_variant_t> range =
        GetOptionsFromEngine(env, self).getRangeDomain(nameStr);

      if (!std::holds_alternative<int>(range.min))
      {
        throw std::invalid_argument("Trying to get range domain of " + nameStr +
          " as an Integer but it is not an Integer domain");
      }

      jclass integerClass = env->FindClass("java/lang/Integer");
      jmethodID valueOf = env->GetStaticMethodID(integerClass, "valueOf", "(I)Ljava/lang/Integer;");
      return env->NewObject(rangeClass, rangeCtor,
        env->CallStaticObjectMethod(integerClass, valueOf, std::get<int>(range.min)),
        env->CallStaticObjectMethod(integerClass, valueOf, std::get<int>(range.max)),
        env->CallStaticObjectMethod(integerClass, valueOf, std::get<int>(range.increment)));
    }
    catch (const std::exception& e)
    {
      env->ThrowNew(env->FindClass("java/lang/IllegalArgumentException"), e.what());
      return nullptr;
    }
  }

  JNIEXPORT void JAVA_BIND(Options, increase)(JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    try
    {
      GetOptionsFromEngine(env, self).increase(str);
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    env->ReleaseStringUTFChars(name, str);
  }

  JNIEXPORT void JAVA_BIND(Options, decrease)(JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    try
    {
      GetOptionsFromEngine(env, self).decrease(str);
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    env->ReleaseStringUTFChars(name, str);
  }

  JNIEXPORT void JAVA_BIND(Options, cycle)(JNIEnv* env, jobject self, jstring name)
  {
    const char* str = env->GetStringUTFChars(name, nullptr);
    try
    {
      GetOptionsFromEngine(env, self).cycle(str);
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    env->ReleaseStringUTFChars(name, str);
  }
}
