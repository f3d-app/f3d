#include "F3DJavaBindings.h"

#include <app_f3d_F3D_VideoEncoder.h>
#include <app_f3d_F3D_VideoFrame.h>
#include <app_f3d_F3D_VideoPacket.h>
#include <app_f3d_F3D_Window.h>

#include <video_encoder.h>

#include <map>

namespace
{
std::map<jlong, jobject> g_packetListeners;
}

template<typename T>
struct SmartPointerWrapper
{
  std::shared_ptr<T> impl;

  SmartPointerWrapper(std::shared_ptr<T> ptr)
    : impl(std::move(ptr))
  {
  }
};

struct f3d_video_encoder_t : public SmartPointerWrapper<f3d::video_encoder>
{
  using SmartPointerWrapper<f3d::video_encoder>::SmartPointerWrapper;
};
struct f3d_video_frame_t : public SmartPointerWrapper<f3d::video_frame>
{
  using SmartPointerWrapper<f3d::video_frame>::SmartPointerWrapper;
};

// Helper function to get the f3d::video_frame pointer from a Java object
inline f3d::video_frame* GetVideoFrame(JNIEnv* env, jobject self)
{
  JniLocalRef<jclass> cls(env, env->GetObjectClass(self));
  jfieldID fid = env->GetFieldID(cls, "mNativeAddress", "J");
  jlong ptr = env->GetLongField(self, fid);

  return reinterpret_cast<f3d_video_frame_t*>(ptr)->impl.get();
}

// Helper function to get the f3d::video_packet pointer from a Java object
inline f3d::video_packet* GetVideoPacket(JNIEnv* env, jobject self)
{
  JniLocalRef<jclass> cls(env, env->GetObjectClass(self));
  jfieldID fid = env->GetFieldID(cls, "mNativeAddress", "J");
  jlong ptr = env->GetLongField(self, fid);

  return reinterpret_cast<f3d::video_packet*>(ptr);
}

// Helper function to get the f3d::video_encoder pointer from a Java object
inline f3d::video_encoder* GetVideoEncoder(JNIEnv* env, jobject self)
{
  JniLocalRef<jclass> cls(env, env->GetObjectClass(self));
  jfieldID fid = env->GetFieldID(cls, "mNativeAddress", "J");
  jlong ptr = env->GetLongField(self, fid);

  return reinterpret_cast<f3d_video_encoder_t*>(ptr)->impl.get();
}

extern "C"
{
  // VideoFrame
  JNIEXPORT void JAVA_BIND(VideoFrame, nativeDestroy)(JNIEnv*, jclass, jlong nativeAddress)
  {
    delete reinterpret_cast<f3d_video_frame_t*>(nativeAddress);
  }

  JNIEXPORT jobject JAVA_BIND(VideoFrame, setTimestamp)(JNIEnv* env, jobject self, jlong timestamp)
  {
    GetVideoFrame(env, self)->setTimestamp(timestamp);
    return self;
  }

  // VideoPacket
  JNIEXPORT jbyteArray JAVA_BIND(VideoPacket, getPacketData)(JNIEnv* env, jobject self)
  {
    f3d::video_packet* packet = GetVideoPacket(env, self);
    jsize size = static_cast<jsize>(packet->getPacketSize());

    jbyteArray result = env->NewByteArray(size);
    if (size > 0)
    {
      env->SetByteArrayRegion(result, 0, size, reinterpret_cast<jbyte*>(packet->getPacketData()));
    }
    return result;
  }

  JNIEXPORT jlong JAVA_BIND(VideoPacket, getTimestamp)(JNIEnv* env, jobject self)
  {
    return GetVideoPacket(env, self)->getTimestamp();
  }

  JNIEXPORT jboolean JAVA_BIND(VideoPacket, isKeyFrame)(JNIEnv* env, jobject self)
  {
    return GetVideoPacket(env, self)->isKeyFrame() ? JNI_TRUE : JNI_FALSE;
  }

  // VideoEncoder
  JNIEXPORT jlong JAVA_BIND(VideoEncoder, nativeCreate)(JNIEnv* env, jclass, jint codec,
    jstring explicitCodecName, jint width, jint height, jdouble frameRate, jdouble bitrate,
    jboolean lowLatency)
  {
    JniUTFString jExplicitCodecName(env, explicitCodecName);

    f3d::video_encoder::params params;
    params.Codec = static_cast<f3d::video_encoder::codec>(codec);
    params.ExplicitCodecName = jExplicitCodecName.c_str() ? jExplicitCodecName.c_str() : "";
    params.Width = width;
    params.Height = height;
    params.FrameRate = frameRate;
    params.Bitrate = bitrate;
    params.LowLatency = lowLatency == JNI_TRUE;

    try
    {
      return reinterpret_cast<jlong>(new f3d_video_encoder_t(f3d::video_encoder::create(params)));
    }
    catch (const f3d::video_encoder::codec_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/VideoEncoder$CodecException", e.what());
      return 0;
    }
  }

  JNIEXPORT void JAVA_BIND(VideoEncoder, nativeDestroy)(JNIEnv* env, jclass, jlong nativeAddress)
  {
    auto it = g_packetListeners.find(nativeAddress);
    if (it != g_packetListeners.end())
    {
      env->DeleteGlobalRef(it->second);
      g_packetListeners.erase(it);
    }
    delete reinterpret_cast<f3d_video_encoder_t*>(nativeAddress);
  }

  JNIEXPORT jobject JAVA_BIND(VideoEncoder, getAvailableEncoders)(JNIEnv* env, jclass)
  {
    auto encoders = f3d::video_encoder::getAvailableEncoders();
    jclass arrayListClass = env->FindClass("java/util/ArrayList");
    jmethodID arrayListCtor = env->GetMethodID(arrayListClass, "<init>", "()V");
    jmethodID arrayListAdd = env->GetMethodID(arrayListClass, "add", "(Ljava/lang/Object;)Z");
    jobject result = env->NewObject(arrayListClass, arrayListCtor);

    jclass pairClass = env->FindClass("app/f3d/F3D/VideoEncoder$StringPair");
    jmethodID pairCtor =
      env->GetMethodID(pairClass, "<init>", "(Ljava/lang/String;Ljava/lang/String;)V");

    for (const auto& encoder : encoders)
    {
      jstring name = env->NewStringUTF(encoder.first.c_str());
      jstring description = env->NewStringUTF(encoder.second.c_str());
      jobject pair = env->NewObject(pairClass, pairCtor, name, description);
      env->CallBooleanMethod(result, arrayListAdd, pair);
      env->DeleteLocalRef(name);
      env->DeleteLocalRef(description);
      env->DeleteLocalRef(pair);
    }

    return result;
  }

  JNIEXPORT jint JAVA_BIND(VideoEncoder, getWidth)(JNIEnv* env, jobject self)
  {
    return GetVideoEncoder(env, self)->getWidth();
  }

  JNIEXPORT jint JAVA_BIND(VideoEncoder, getHeight)(JNIEnv* env, jobject self)
  {
    return GetVideoEncoder(env, self)->getHeight();
  }

  JNIEXPORT jobject JAVA_BIND(VideoEncoder, listen)(JNIEnv* env, jobject self, jobject listener)
  {
    f3d::video_encoder* encoder = GetVideoEncoder(env, self);

    JniLocalRef<jclass> cls(env, env->GetObjectClass(self));
    jlong nativeAddress = env->GetLongField(self, env->GetFieldID(cls, "mNativeAddress", "J"));

    JavaVM* jvm = nullptr;
    env->GetJavaVM(&jvm);

    auto it = g_packetListeners.find(nativeAddress);
    if (it != g_packetListeners.end())
    {
      env->DeleteGlobalRef(it->second);
    }
    g_packetListeners[nativeAddress] = env->NewGlobalRef(listener);

    encoder->listen(
      [=](const std::shared_ptr<f3d::video_packet>& packet)
      {
        JNIEnv* threadEnv = nullptr;
#ifdef __ANDROID__
        if (jvm->AttachCurrentThread(&threadEnv, nullptr) != JNI_OK)
#else
        if (jvm->AttachCurrentThread(reinterpret_cast<void**>(&threadEnv), nullptr) != JNI_OK)
#endif
        {
          return;
        }

        auto listenerIt = g_packetListeners.find(nativeAddress);
        if (listenerIt == g_packetListeners.end())
        {
          jvm->DetachCurrentThread();
          return;
        }
        jobject listenerObj = listenerIt->second;

        // Scoped so all JniLocalRef destructors run before DetachCurrentThread
        {
          JniLocalRef<jclass> packetClass(
            threadEnv, threadEnv->FindClass("app/f3d/F3D/VideoPacket"));
          jmethodID packetCtor = threadEnv->GetMethodID(packetClass, "<init>", "(J)V");
          JniLocalRef<jobject> packetObj(threadEnv,
            threadEnv->NewObject(packetClass, packetCtor, reinterpret_cast<jlong>(packet.get())));

          JniLocalRef<jclass> listenerClass(threadEnv, threadEnv->GetObjectClass(listenerObj));
          jmethodID executeMethod =
            threadEnv->GetMethodID(listenerClass, "execute", "(Lapp/f3d/F3D/VideoPacket;)V");
          threadEnv->CallVoidMethod(listenerObj, executeMethod, packetObj.get());
        }

        jvm->DetachCurrentThread();
      });

    return self;
  }

  JNIEXPORT jboolean JAVA_BIND(VideoEncoder, submit)(JNIEnv* env, jobject self, jobject frame)
  {
    f3d::video_encoder* encoder = GetVideoEncoder(env, self);
    f3d::video_frame* cppFrame = GetVideoFrame(env, frame);

    try
    {
      std::shared_ptr<f3d::video_frame> framePtr(cppFrame, [](f3d::video_frame*) {}); // non-owning
      return encoder->submit(framePtr) ? JNI_TRUE : JNI_FALSE;
    }
    catch (const f3d::video_encoder::transport_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/VideoEncoder$TransportException", e.what());
      return JNI_FALSE;
    }
  }

  JNIEXPORT jobject JAVA_BIND(VideoEncoder, flush)(JNIEnv* env, jobject self)
  {
    GetVideoEncoder(env, self)->flush();
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Window, getVideoFrame)(JNIEnv* env, jobject self)
  {
    f3d::window& win = GetEngine(env, self)->getWindow();
    f3d_video_frame_t* frame = new f3d_video_frame_t(win.getVideoFrame());

    JniLocalRef<jclass> frameClass(env, env->FindClass("app/f3d/F3D/VideoFrame"));
    jmethodID constructor = env->GetMethodID(frameClass, "<init>", "(J)V");

    jobject result = env->NewObject(frameClass, constructor, reinterpret_cast<jlong>(frame));

    return result;
  }
}
