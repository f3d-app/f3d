#include "F3DJavaBindings.h"

#include <app_f3d_F3D_Image.h>

#include <image.h>

extern "C"
{
  JNIEXPORT jobject JAVA_BIND(Image, getSupportedFormats)(JNIEnv* env, jclass)
  {
    return CreateStringList(env, f3d::image::getSupportedFormats());
  }

  JNIEXPORT jlong JAVA_BIND(Image, nativeCreateFromFile)(JNIEnv* env, jclass, jstring filePath)
  {
    JniUTFString path(env, filePath);
    jlong result = 0;
    try
    {
      result = reinterpret_cast<jlong>(new f3d::image(path.c_str()));
    }
    catch (const f3d::image::read_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Image$ReadException", e.what());
    }
    return result;
  }

  JNIEXPORT jlong JAVA_BIND(Image, nativeCreate)(
    JNIEnv* env, jclass, jint width, jint height, jint channelCount, jint type)
  {
    f3d::image::ChannelType channelType = static_cast<f3d::image::ChannelType>(type);
    f3d::image* img = new f3d::image(width, height, channelCount, channelType);
    return reinterpret_cast<jlong>(img);
  }

  JNIEXPORT void JAVA_BIND(Image, nativeDestroy)(JNIEnv* env, jclass, jlong nativeAddress)
  {
    f3d::image* img = reinterpret_cast<f3d::image*>(nativeAddress);
    delete img;
  }

  JNIEXPORT jint JAVA_BIND(Image, getWidth)(JNIEnv* env, jobject self)
  {
    return GetImage(env, self)->getWidth();
  }

  JNIEXPORT jint JAVA_BIND(Image, getHeight)(JNIEnv* env, jobject self)
  {
    return GetImage(env, self)->getHeight();
  }

  JNIEXPORT jint JAVA_BIND(Image, getChannelCount)(JNIEnv* env, jobject self)
  {
    return GetImage(env, self)->getChannelCount();
  }

  JNIEXPORT jobject JAVA_BIND(Image, getChannelType)(JNIEnv* env, jobject self)
  {
    f3d::image::ChannelType type = GetImage(env, self)->getChannelType();

    JniLocalRef<jclass> enumClass(env, env->FindClass("app/f3d/F3D/Image$ChannelType"));
    jfieldID fieldID;

    switch (type)
    {
      case f3d::image::ChannelType::BYTE:
        fieldID = env->GetStaticFieldID(enumClass, "BYTE", "Lapp/f3d/F3D/Image$ChannelType;");
        break;
      case f3d::image::ChannelType::SHORT:
        fieldID = env->GetStaticFieldID(enumClass, "SHORT", "Lapp/f3d/F3D/Image$ChannelType;");
        break;
      case f3d::image::ChannelType::FLOAT:
        fieldID = env->GetStaticFieldID(enumClass, "FLOAT", "Lapp/f3d/F3D/Image$ChannelType;");
        break;
      default:
        fieldID = env->GetStaticFieldID(enumClass, "BYTE", "Lapp/f3d/F3D/Image$ChannelType;");
        break;
    }

    return env->GetStaticObjectField(enumClass, fieldID);
  }

  JNIEXPORT jint JAVA_BIND(Image, getChannelTypeSize)(JNIEnv* env, jobject self)
  {
    return GetImage(env, self)->getChannelTypeSize();
  }

  JNIEXPORT jobject JAVA_BIND(Image, setContent)(JNIEnv* env, jobject self, jbyteArray buffer)
  {
    f3d::image* img = GetImage(env, self);

    jbyte* bufferData = env->GetByteArrayElements(buffer, nullptr);
    img->setContent(bufferData);

    env->ReleaseByteArrayElements(buffer, bufferData, 0);
    return self;
  }

  JNIEXPORT jbyteArray JAVA_BIND(Image, getContent)(JNIEnv* env, jobject self)
  {
    f3d::image* img = GetImage(env, self);

    void* content = img->getContent();
    unsigned int size =
      img->getWidth() * img->getHeight() * img->getChannelCount() * img->getChannelTypeSize();

    jbyteArray result = env->NewByteArray(size);
    env->SetByteArrayRegion(result, 0, size, static_cast<jbyte*>(content));

    return result;
  }

  JNIEXPORT jdoubleArray JAVA_BIND(Image, getNormalizedPixel)(
    JNIEnv* env, jobject self, jint x, jint y)
  {
    std::vector<double> pixel = GetImage(env, self)->getNormalizedPixel({ x, y });

    jdoubleArray result = env->NewDoubleArray(pixel.size());
    env->SetDoubleArrayRegion(result, 0, pixel.size(), pixel.data());

    return result;
  }

  JNIEXPORT jdouble JAVA_BIND(Image, compare)(JNIEnv* env, jobject self, jobject reference)
  {
    return GetImage(env, self)->compare(*GetImage(env, reference));
  }

  JNIEXPORT jobject JAVA_BIND(Image, save)(
    JNIEnv* env, jobject self, jstring filePath, jobject format)
  {
    f3d::image* img = GetImage(env, self);

    JniUTFString path(env, filePath);

    JniLocalRef<jclass> formatEnum(env, env->GetObjectClass(format));
    jmethodID ordinalMethod = env->GetMethodID(formatEnum, "ordinal", "()I");
    jint formatOrdinal = env->CallIntMethod(format, ordinalMethod);

    f3d::image::SaveFormat saveFormat = static_cast<f3d::image::SaveFormat>(formatOrdinal);
    try
    {
      img->save(path.c_str(), saveFormat);
    }
    catch (const f3d::image::write_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Image$WriteException", e.what());
    }

    return self;
  }

  JNIEXPORT jbyteArray JAVA_BIND(Image, saveBuffer)(JNIEnv* env, jobject self, jobject format)
  {
    f3d::image* img = GetImage(env, self);

    JniLocalRef<jclass> formatEnum(env, env->GetObjectClass(format));
    jmethodID ordinalMethod = env->GetMethodID(formatEnum, "ordinal", "()I");
    jint formatOrdinal = env->CallIntMethod(format, ordinalMethod);

    f3d::image::SaveFormat saveFormat = static_cast<f3d::image::SaveFormat>(formatOrdinal);
    try
    {
      std::vector<unsigned char> buffer = img->saveBuffer(saveFormat);

      jbyteArray result = env->NewByteArray(buffer.size());
      env->SetByteArrayRegion(result, 0, buffer.size(), reinterpret_cast<jbyte*>(buffer.data()));

      return result;
    }
    catch (const f3d::image::write_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Image$WriteException", e.what());
    }
    return nullptr;
  }

  JNIEXPORT jstring JAVA_BIND(Image, toTerminalText)(JNIEnv* env, jobject self)
  {
    std::string text = GetImage(env, self)->toTerminalText();
    return env->NewStringUTF(text.c_str());
  }

  JNIEXPORT jobject JAVA_BIND(Image, setMetadata)(
    JNIEnv* env, jobject self, jstring key, jstring value)
  {
    f3d::image* img = GetImage(env, self);

    JniUTFString keyStr(env, key);
    JniUTFString valueStr(env, value);

    img->setMetadata(keyStr.c_str(), valueStr.c_str());

    return self;
  }

  JNIEXPORT jstring JAVA_BIND(Image, getMetadata)(JNIEnv* env, jobject self, jstring key)
  {
    f3d::image* img = GetImage(env, self);

    JniUTFString keyStr(env, key);
    std::string value;
    try
    {
      value = img->getMetadata(keyStr.c_str());
    }
    catch (const f3d::image::metadata_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Image$MetadataException", e.what());
    }
    return env->ExceptionCheck() ? nullptr : env->NewStringUTF(value.c_str());
  }

  JNIEXPORT jobject JAVA_BIND(Image, allMetadata)(JNIEnv* env, jobject self)
  {
    return CreateStringList(env, GetImage(env, self)->allMetadata());
  }
}
