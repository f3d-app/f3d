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
    const char* path = env->GetStringUTFChars(filePath, nullptr);
    f3d::image* img = new f3d::image(path);
    env->ReleaseStringUTFChars(filePath, path);
    return reinterpret_cast<jlong>(img);
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
    jclass cls = env->GetObjectClass(self);
    jfieldID fid = env->GetFieldID(cls, "mNativeAddress", "J");
    jlong ptr = env->GetLongField(self, fid);
    f3d::image* img = reinterpret_cast<f3d::image*>(ptr);
    return img->getWidth();
  }

  JNIEXPORT jint JAVA_BIND(Image, getHeight)(JNIEnv* env, jobject self)
  {
    jclass cls = env->GetObjectClass(self);
    jfieldID fid = env->GetFieldID(cls, "mNativeAddress", "J");
    jlong ptr = env->GetLongField(self, fid);
    f3d::image* img = reinterpret_cast<f3d::image*>(ptr);
    return img->getHeight();
  }

  JNIEXPORT jint JAVA_BIND(Image, getChannelCount)(JNIEnv* env, jobject self)
  {
    jclass cls = env->GetObjectClass(self);
    jfieldID fid = env->GetFieldID(cls, "mNativeAddress", "J");
    jlong ptr = env->GetLongField(self, fid);
    f3d::image* img = reinterpret_cast<f3d::image*>(ptr);
    return img->getChannelCount();
  }

  JNIEXPORT jobject JAVA_BIND(Image, getChannelType)(JNIEnv* env, jobject self)
  {
    jclass cls = env->GetObjectClass(self);
    jfieldID fid = env->GetFieldID(cls, "mNativeAddress", "J");
    jlong ptr = env->GetLongField(self, fid);
    f3d::image* img = reinterpret_cast<f3d::image*>(ptr);

    f3d::image::ChannelType type = img->getChannelType();

    jclass enumClass = env->FindClass("app/f3d/F3D/Image$ChannelType");
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
    jclass cls = env->GetObjectClass(self);
    jfieldID fid = env->GetFieldID(cls, "mNativeAddress", "J");
    jlong ptr = env->GetLongField(self, fid);
    f3d::image* img = reinterpret_cast<f3d::image*>(ptr);
    return img->getChannelTypeSize();
  }

  JNIEXPORT jobject JAVA_BIND(Image, setContent)(JNIEnv* env, jobject self, jbyteArray buffer)
  {
    jclass cls = env->GetObjectClass(self);
    jfieldID fid = env->GetFieldID(cls, "mNativeAddress", "J");
    jlong ptr = env->GetLongField(self, fid);
    f3d::image* img = reinterpret_cast<f3d::image*>(ptr);

    jbyte* bufferData = env->GetByteArrayElements(buffer, nullptr);
    img->setContent(bufferData);

    env->ReleaseByteArrayElements(buffer, bufferData, 0);
    return self;
  }

  JNIEXPORT jbyteArray JAVA_BIND(Image, getContent)(JNIEnv* env, jobject self)
  {
    jclass cls = env->GetObjectClass(self);
    jfieldID fid = env->GetFieldID(cls, "mNativeAddress", "J");
    jlong ptr = env->GetLongField(self, fid);
    f3d::image* img = reinterpret_cast<f3d::image*>(ptr);

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
    jclass cls = env->GetObjectClass(self);
    jfieldID fid = env->GetFieldID(cls, "mNativeAddress", "J");
    jlong ptr = env->GetLongField(self, fid);
    f3d::image* img = reinterpret_cast<f3d::image*>(ptr);

    std::vector<double> pixel = img->getNormalizedPixel({ x, y });

    jdoubleArray result = env->NewDoubleArray(pixel.size());
    env->SetDoubleArrayRegion(result, 0, pixel.size(), pixel.data());

    return result;
  }

  JNIEXPORT jdouble JAVA_BIND(Image, compare)(JNIEnv* env, jobject self, jobject reference)
  {
    jclass cls = env->GetObjectClass(self);
    jfieldID fid = env->GetFieldID(cls, "mNativeAddress", "J");
    jlong ptr = env->GetLongField(self, fid);
    f3d::image* img = reinterpret_cast<f3d::image*>(ptr);

    jclass refCls = env->GetObjectClass(reference);
    jfieldID refFid = env->GetFieldID(refCls, "mNativeAddress", "J");
    jlong refPtr = env->GetLongField(reference, refFid);
    f3d::image* refImg = reinterpret_cast<f3d::image*>(refPtr);

    return img->compare(*refImg);
  }

  JNIEXPORT jobject JAVA_BIND(Image, save)(
    JNIEnv* env, jobject self, jstring filePath, jobject format)
  {
    jclass cls = env->GetObjectClass(self);
    jfieldID fid = env->GetFieldID(cls, "mNativeAddress", "J");
    jlong ptr = env->GetLongField(self, fid);
    f3d::image* img = reinterpret_cast<f3d::image*>(ptr);

    const char* path = env->GetStringUTFChars(filePath, nullptr);

    jclass formatEnum = env->GetObjectClass(format);
    jmethodID ordinalMethod = env->GetMethodID(formatEnum, "ordinal", "()I");
    jint formatOrdinal = env->CallIntMethod(format, ordinalMethod);

    f3d::image::SaveFormat saveFormat = static_cast<f3d::image::SaveFormat>(formatOrdinal);
    img->save(path, saveFormat);

    env->ReleaseStringUTFChars(filePath, path);
    return self;
  }

  JNIEXPORT jbyteArray JAVA_BIND(Image, saveBuffer)(JNIEnv* env, jobject self, jobject format)
  {
    jclass cls = env->GetObjectClass(self);
    jfieldID fid = env->GetFieldID(cls, "mNativeAddress", "J");
    jlong ptr = env->GetLongField(self, fid);
    f3d::image* img = reinterpret_cast<f3d::image*>(ptr);

    jclass formatEnum = env->GetObjectClass(format);
    jmethodID ordinalMethod = env->GetMethodID(formatEnum, "ordinal", "()I");
    jint formatOrdinal = env->CallIntMethod(format, ordinalMethod);

    f3d::image::SaveFormat saveFormat = static_cast<f3d::image::SaveFormat>(formatOrdinal);
    std::vector<unsigned char> buffer = img->saveBuffer(saveFormat);

    jbyteArray result = env->NewByteArray(buffer.size());
    env->SetByteArrayRegion(result, 0, buffer.size(), reinterpret_cast<jbyte*>(buffer.data()));

    return result;
  }

  JNIEXPORT jstring JAVA_BIND(Image, toTerminalText)(JNIEnv* env, jobject self)
  {
    jclass cls = env->GetObjectClass(self);
    jfieldID fid = env->GetFieldID(cls, "mNativeAddress", "J");
    jlong ptr = env->GetLongField(self, fid);
    f3d::image* img = reinterpret_cast<f3d::image*>(ptr);

    std::string text = img->toTerminalText();
    return env->NewStringUTF(text.c_str());
  }

  JNIEXPORT jobject JAVA_BIND(Image, setMetadata)(
    JNIEnv* env, jobject self, jstring key, jstring value)
  {
    jclass cls = env->GetObjectClass(self);
    jfieldID fid = env->GetFieldID(cls, "mNativeAddress", "J");
    jlong ptr = env->GetLongField(self, fid);
    f3d::image* img = reinterpret_cast<f3d::image*>(ptr);

    const char* keyStr = env->GetStringUTFChars(key, nullptr);
    const char* valueStr = env->GetStringUTFChars(value, nullptr);

    img->setMetadata(keyStr, valueStr);

    env->ReleaseStringUTFChars(key, keyStr);
    env->ReleaseStringUTFChars(value, valueStr);

    return self;
  }

  JNIEXPORT jstring JAVA_BIND(Image, getMetadata)(JNIEnv* env, jobject self, jstring key)
  {
    jclass cls = env->GetObjectClass(self);
    jfieldID fid = env->GetFieldID(cls, "mNativeAddress", "J");
    jlong ptr = env->GetLongField(self, fid);
    f3d::image* img = reinterpret_cast<f3d::image*>(ptr);

    const char* keyStr = env->GetStringUTFChars(key, nullptr);
    std::string value = img->getMetadata(keyStr);
    env->ReleaseStringUTFChars(key, keyStr);

    return env->NewStringUTF(value.c_str());
  }

  JNIEXPORT jobject JAVA_BIND(Image, allMetadata)(JNIEnv* env, jobject self)
  {
    jclass cls = env->GetObjectClass(self);
    jfieldID fid = env->GetFieldID(cls, "mNativeAddress", "J");
    jlong ptr = env->GetLongField(self, fid);
    f3d::image* img = reinterpret_cast<f3d::image*>(ptr);

    return CreateStringList(env, img->allMetadata());
  }
}
