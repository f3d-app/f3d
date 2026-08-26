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
    JniUTFString str(env, name);
    try
    {
      GetOptionsFromEngine(env, self).set(str.c_str(), static_cast<bool>(value));
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::incompatible_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
  }

  JNIEXPORT void JAVA_BIND(Options, setAsInt)(JNIEnv* env, jobject self, jstring name, jint value)
  {
    JniUTFString str(env, name);
    try
    {
      GetOptionsFromEngine(env, self).set(str.c_str(), static_cast<int>(value));
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::incompatible_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
  }

  JNIEXPORT void JAVA_BIND(Options, setAsDouble)(
    JNIEnv* env, jobject self, jstring name, jdouble value)
  {
    JniUTFString str(env, name);
    try
    {
      GetOptionsFromEngine(env, self).set(str.c_str(), static_cast<double>(value));
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::incompatible_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
  }

  JNIEXPORT void JAVA_BIND(Options, setAsString)(
    JNIEnv* env, jobject self, jstring name, jstring value)
  {
    JniUTFString nameStr(env, name);
    JniUTFString valueStr(env, value);
    try
    {
      GetOptionsFromEngine(env, self).set(nameStr.c_str(), std::string(valueStr.c_str()));
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::incompatible_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
  }

  JNIEXPORT void JAVA_BIND(Options, setAsDoubleVector)(
    JNIEnv* env, jobject self, jstring name, jdoubleArray values)
  {
    JniUTFString str(env, name);
    jsize len = env->GetArrayLength(values);
    double* arr = env->GetDoubleArrayElements(values, nullptr);
    std::vector<double> vec(arr, arr + len);
    env->ReleaseDoubleArrayElements(values, arr, 0);

    try
    {
      GetOptionsFromEngine(env, self).set(str.c_str(), vec);
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::incompatible_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
  }

  JNIEXPORT void JAVA_BIND(Options, setAsIntVector)(
    JNIEnv* env, jobject self, jstring name, jintArray values)
  {
    JniUTFString str(env, name);
    jsize len = env->GetArrayLength(values);
    int* arr = env->GetIntArrayElements(values, nullptr);
    std::vector<int> vec(arr, arr + len);
    env->ReleaseIntArrayElements(values, arr, 0);

    try
    {
      GetOptionsFromEngine(env, self).set(str.c_str(), vec);
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::incompatible_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
  }

  JNIEXPORT jboolean JAVA_BIND(Options, getAsBool)(JNIEnv* env, jobject self, jstring name)
  {
    JniUTFString str(env, name);
    bool value = false;
    try
    {
      value = std::get<bool>(GetOptionsFromEngine(env, self).get(str.c_str()));
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::no_value_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$NoValueException", e.what());
    }
    catch (const std::bad_variant_access& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
    return value;
  }

  JNIEXPORT jint JAVA_BIND(Options, getAsInt)(JNIEnv* env, jobject self, jstring name)
  {
    JniUTFString str(env, name);
    int value = 0;
    try
    {
      value = std::get<int>(GetOptionsFromEngine(env, self).get(str.c_str()));
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::no_value_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$NoValueException", e.what());
    }
    catch (const std::bad_variant_access& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
    return value;
  }

  JNIEXPORT jdouble JAVA_BIND(Options, getAsDouble)(JNIEnv* env, jobject self, jstring name)
  {
    JniUTFString str(env, name);
    double value = 0.0;
    try
    {
      value = std::get<double>(GetOptionsFromEngine(env, self).get(str.c_str()));
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::no_value_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$NoValueException", e.what());
    }
    catch (const std::bad_variant_access& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
    return value;
  }

  JNIEXPORT jstring JAVA_BIND(Options, getAsString)(JNIEnv* env, jobject self, jstring name)
  {
    JniUTFString str(env, name);
    std::string value;
    try
    {
      value = std::get<std::string>(GetOptionsFromEngine(env, self).get(str.c_str()));
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::no_value_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$NoValueException", e.what());
    }
    catch (const std::bad_variant_access& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
    return env->ExceptionCheck() ? nullptr : env->NewStringUTF(value.c_str());
  }

  JNIEXPORT jstring JAVA_BIND(Options, getAsStringRepresentation)(
    JNIEnv* env, jobject self, jstring name)
  {
    JniUTFString str(env, name);
    std::string value;
    try
    {
      value = GetOptionsFromEngine(env, self).getAsString(str.c_str());
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::no_value_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$NoValueException", e.what());
    }
    return env->ExceptionCheck() ? nullptr : env->NewStringUTF(value.c_str());
  }

  JNIEXPORT void JAVA_BIND(Options, setAsStringRepresentation)(
    JNIEnv* env, jobject self, jstring name, jstring str)
  {
    JniUTFString nameStr(env, name);
    JniUTFString valueStr(env, str);
    try
    {
      GetOptionsFromEngine(env, self).setAsString(nameStr.c_str(), valueStr.c_str());
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::parsing_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$ParsingException", e.what());
    }
  }

  JNIEXPORT jdoubleArray JAVA_BIND(Options, getAsDoubleVector)(
    JNIEnv* env, jobject self, jstring name)
  {
    JniUTFString str(env, name);
    jdoubleArray result = nullptr;
    try
    {
      std::vector<double> vec =
        std::get<std::vector<double>>(GetOptionsFromEngine(env, self).get(str.c_str()));
      result = env->NewDoubleArray(vec.size());
      env->SetDoubleArrayRegion(result, 0, vec.size(), vec.data());
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::no_value_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$NoValueException", e.what());
    }
    catch (const std::bad_variant_access& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
    return result;
  }

  JNIEXPORT jintArray JAVA_BIND(Options, getAsIntVector)(JNIEnv* env, jobject self, jstring name)
  {
    JniUTFString str(env, name);
    jintArray result = nullptr;
    try
    {
      std::vector<int> vec =
        std::get<std::vector<int>>(GetOptionsFromEngine(env, self).get(str.c_str()));
      result = env->NewIntArray(vec.size());
      env->SetIntArrayRegion(result, 0, vec.size(), vec.data());
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::no_value_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$NoValueException", e.what());
    }
    catch (const std::bad_variant_access& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
    return result;
  }

  JNIEXPORT void JAVA_BIND(Options, toggle)(JNIEnv* env, jobject self, jstring name)
  {
    JniUTFString str(env, name);
    try
    {
      GetOptionsFromEngine(env, self).toggle(str.c_str());
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::incompatible_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
  }

  JNIEXPORT jboolean JAVA_BIND(Options, isSame)(
    JNIEnv* env, jobject self, jobject other, jstring name)
  {
    JniLocalRef<jclass> otherClass(env, env->GetObjectClass(other));
    jfieldID fid = env->GetFieldID(otherClass, "mNativeAddress", "J");
    jlong otherPtr = env->GetLongField(other, fid);

    JniUTFString str(env, name);
    bool result = false;
    try
    {
      result = GetOptionsFromEngine(env, self).isSame(
        reinterpret_cast<f3d::engine*>(otherPtr)->getOptions(), str.c_str());
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    return result;
  }

  JNIEXPORT jboolean JAVA_BIND(Options, hasValue)(JNIEnv* env, jobject self, jstring name)
  {
    JniUTFString str(env, name);
    bool result = false;
    try
    {
      result = GetOptionsFromEngine(env, self).hasValue(str.c_str());
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    return result;
  }

  JNIEXPORT void JAVA_BIND(Options, copy)(JNIEnv* env, jobject self, jobject other, jstring name)
  {
    JniLocalRef<jclass> otherClass(env, env->GetObjectClass(other));
    jfieldID fid = env->GetFieldID(otherClass, "mNativeAddress", "J");
    jlong otherPtr = env->GetLongField(other, fid);

    JniUTFString str(env, name);
    try
    {
      GetOptionsFromEngine(env, self).copy(
        reinterpret_cast<f3d::engine*>(otherPtr)->getOptions(), str.c_str());
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
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
    JniUTFString str(env, option);
    auto [name, distance] = GetOptionsFromEngine(env, self).getClosestOption(str.c_str());

    JniLocalRef<jclass> resultClass(env, env->FindClass("app/f3d/F3D/Options$ClosestOptionResult"));
    jmethodID constructor = env->GetMethodID(resultClass, "<init>", "(Ljava/lang/String;I)V");

    JniLocalRef<jstring> jname(env, env->NewStringUTF(name.c_str()));
    jobject result =
      env->NewObject(resultClass, constructor, jname.get(), static_cast<jint>(distance));

    return result;
  }

  JNIEXPORT jboolean JAVA_BIND(Options, isOptional)(JNIEnv* env, jobject self, jstring name)
  {
    JniUTFString str(env, name);
    bool result = false;
    try
    {
      result = GetOptionsFromEngine(env, self).isOptional(str.c_str());
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    return result;
  }

  JNIEXPORT jobject JAVA_BIND(Options, getType)(JNIEnv* env, jobject self, jstring name)
  {
    JniUTFString str(env, name);
    jobject result = nullptr;
    try
    {
      f3d::options::option_type type = GetOptionsFromEngine(env, self).getType(str.c_str());

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

      JniLocalRef<jclass> typeClass(env, env->FindClass("app/f3d/F3D/Options$OptionType"));
      jfieldID fid = env->GetStaticFieldID(typeClass, enumName, "Lapp/f3d/F3D/Options$OptionType;");
      result = env->GetStaticObjectField(typeClass, fid);
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    return result;
  }

  JNIEXPORT void JAVA_BIND(Options, reset)(JNIEnv* env, jobject self, jstring name)
  {
    JniUTFString str(env, name);
    try
    {
      GetOptionsFromEngine(env, self).reset(str.c_str());
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
  }

  JNIEXPORT void JAVA_BIND(Options, removeValue)(JNIEnv* env, jobject self, jstring name)
  {
    JniUTFString str(env, name);
    try
    {
      GetOptionsFromEngine(env, self).removeValue(str.c_str());
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::incompatible_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
  }

  JNIEXPORT jboolean JAVA_BIND(Options, hasDomain)(JNIEnv* env, jobject self, jstring name)
  {
    JniUTFString str(env, name);
    bool result = false;
    try
    {
      result = GetOptionsFromEngine(env, self).hasDomain(str.c_str());
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    return result;
  }

  JNIEXPORT jobject JAVA_BIND(Options, getDomainStyle)(JNIEnv* env, jobject self, jstring name)
  {
    JniUTFString str(env, name);
    jobject result = nullptr;
    try
    {
      f3d::options::domain_style ds = GetOptionsFromEngine(env, self).getDomainStyle(str.c_str());

      JniLocalRef<jclass> enumClass(env, env->FindClass("app/f3d/F3D/Options$DomainStyle"));
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
      result = env->GetStaticObjectField(enumClass, fieldID);
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    return result;
  }

  JNIEXPORT jobject JAVA_BIND(Options, getRangeDomainAsDouble)(
    JNIEnv* env, jobject self, jstring name)
  {
    JniUTFString str(env, name);
    const std::string nameStr = str.c_str();

    JniLocalRef<jclass> rangeClass(env, env->FindClass("app/f3d/F3D/Options$DomainRange"));
    jmethodID rangeCtor = env->GetMethodID(
      rangeClass, "<init>", "(Ljava/lang/Number;Ljava/lang/Number;Ljava/lang/Number;)V");
    jobject result = nullptr;

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

      JniLocalRef<jclass> doubleClass(env, env->FindClass("java/lang/Double"));
      jmethodID valueOf = env->GetStaticMethodID(doubleClass, "valueOf", "(D)Ljava/lang/Double;");
      result = env->NewObject(rangeClass, rangeCtor,
        env->CallStaticObjectMethod(doubleClass, valueOf, std::get<double>(range.min)),
        env->CallStaticObjectMethod(doubleClass, valueOf, std::get<double>(range.max)),
        env->CallStaticObjectMethod(doubleClass, valueOf, std::get<double>(range.increment)));
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::incompatible_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
    return result;
  }

  JNIEXPORT jobject JAVA_BIND(Options, getRangeDomainAsInt)(JNIEnv* env, jobject self, jstring name)
  {
    JniUTFString str(env, name);
    const std::string nameStr = str.c_str();

    JniLocalRef<jclass> rangeClass(env, env->FindClass("app/f3d/F3D/Options$DomainRange"));
    jmethodID rangeCtor = env->GetMethodID(
      rangeClass, "<init>", "(Ljava/lang/Number;Ljava/lang/Number;Ljava/lang/Number;)V");
    jobject result = nullptr;

    try
    {
      f3d::options::DomainRange<f3d::option_variant_t> range =
        GetOptionsFromEngine(env, self).getRangeDomain(nameStr);

      if (!std::holds_alternative<int>(range.min))
      {
        throw std::invalid_argument("Trying to get range domain of " + nameStr +
          " as an Integer but it is not an Integer domain");
      }

      JniLocalRef<jclass> integerClass(env, env->FindClass("java/lang/Integer"));
      jmethodID valueOf = env->GetStaticMethodID(integerClass, "valueOf", "(I)Ljava/lang/Integer;");
      result = env->NewObject(rangeClass, rangeCtor,
        env->CallStaticObjectMethod(integerClass, valueOf, std::get<int>(range.min)),
        env->CallStaticObjectMethod(integerClass, valueOf, std::get<int>(range.max)),
        env->CallStaticObjectMethod(integerClass, valueOf, std::get<int>(range.increment)));
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::incompatible_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
    return result;
  }

  JNIEXPORT jobject JAVA_BIND(Options, getEnumDomainAsString)(JNIEnv* env, jobject self, jstring name)
  {
    JniUTFString str(env, name);
    const std::string nameStr = str.c_str();
    jobject result = nullptr;
    try
    {
      f3d::options::DomainEnum<f3d::option_variant_t> domain =
        GetOptionsFromEngine(env, self).getEnumDomain(nameStr);
      std::vector<std::string> enumeration(domain.enumeration.size());
      std::transform(domain.enumeration.begin(), domain.enumeration.end(), enumeration.begin(),
        [](const auto& value) { return std::get<std::string>(value); });
      result = CreateStringList(env, enumeration);
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::incompatible_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
    return result;
  }

  JNIEXPORT jobject JAVA_BIND(Options, getIndexDomain)(JNIEnv* env, jobject self, jstring name)
  {
    JniUTFString str(env, name);
    const std::string nameStr = str.c_str();
    jobject result = nullptr;
    try
    {
      f3d::options::DomainIndex domain = GetOptionsFromEngine(env, self).getIndexDomain(nameStr);

      jclass integerClass = env->FindClass("java/lang/Integer");
      jmethodID valueOf = env->GetStaticMethodID(integerClass, "valueOf", "(I)Ljava/lang/Integer;");
      if (domain.max.has_value())
      {
        result = env->CallStaticObjectMethod(integerClass, valueOf, domain.max.value());
      }
      else
      {
        // TODO print as `0` in the test, why ?
        result = env->CallStaticObjectMethod(integerClass, valueOf, nullptr);
      }
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
    catch (const f3d::options::incompatible_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$IncompatibleException", e.what());
    }
    return result;
  }

  JNIEXPORT void JAVA_BIND(Options, increase)(JNIEnv* env, jobject self, jstring name)
  {
    JniUTFString str(env, name);
    try
    {
      GetOptionsFromEngine(env, self).increase(str.c_str());
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
  }

  JNIEXPORT void JAVA_BIND(Options, decrease)(JNIEnv* env, jobject self, jstring name)
  {
    JniUTFString str(env, name);
    try
    {
      GetOptionsFromEngine(env, self).decrease(str.c_str());
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
  }

  JNIEXPORT void JAVA_BIND(Options, cycle)(JNIEnv* env, jobject self, jstring name)
  {
    JniUTFString str(env, name);
    try
    {
      GetOptionsFromEngine(env, self).cycle(str.c_str());
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
  }
}
