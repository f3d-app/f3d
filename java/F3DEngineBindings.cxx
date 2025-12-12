#include "F3DJavaBindings.h"

#include <app_f3d_F3D_Engine.h>

#include <context.h>

extern "C"
{
  JNIEXPORT jlong JAVA_BIND(Engine, nativeCreate)(JNIEnv*, jclass, jboolean offscreen)
  {
    return reinterpret_cast<jlong>(new f3d::engine(f3d::engine::create(offscreen)));
  }

  JNIEXPORT jlong JAVA_BIND(Engine, nativeCreateNone)(JNIEnv*, jclass)
  {
    return reinterpret_cast<jlong>(new f3d::engine(f3d::engine::createNone()));
  }

  JNIEXPORT jlong JAVA_BIND(Engine, nativeCreateGLX)(JNIEnv*, jclass, jboolean offscreen)
  {
    return reinterpret_cast<jlong>(new f3d::engine(f3d::engine::createGLX(offscreen)));
  }

  JNIEXPORT jlong JAVA_BIND(Engine, nativeCreateWGL)(JNIEnv*, jclass, jboolean offscreen)
  {
    return reinterpret_cast<jlong>(new f3d::engine(f3d::engine::createWGL(offscreen)));
  }

  JNIEXPORT jlong JAVA_BIND(Engine, nativeCreateEGL)(JNIEnv*, jclass)
  {
    return reinterpret_cast<jlong>(new f3d::engine(f3d::engine::createEGL()));
  }

  JNIEXPORT jlong JAVA_BIND(Engine, nativeCreateOSMesa)(JNIEnv*, jclass)
  {
    return reinterpret_cast<jlong>(new f3d::engine(f3d::engine::createOSMesa()));
  }

  JNIEXPORT jlong JAVA_BIND(Engine, nativeCreateExternal)(
    JNIEnv* env, jclass, jobject getProcAddress)
  {
    if (!getProcAddress)
    {
      return 0;
    }

    jobject globalRef = env->NewGlobalRef(getProcAddress);

    jclass contextFunctionClass = env->GetObjectClass(getProcAddress);
    jmethodID methodID =
      env->GetMethodID(contextFunctionClass, "getProcAddress", "(Ljava/lang/String;)J");

    f3d::context::function func = [env, globalRef, methodID](const char* name) -> f3d::context::fptr
    {
      jstring jname = env->NewStringUTF(name);
      jlong addr = env->CallLongMethod(globalRef, methodID, jname);
      env->DeleteLocalRef(jname);
      return reinterpret_cast<f3d::context::fptr>(addr);
    };

    return reinterpret_cast<jlong>(new f3d::engine(f3d::engine::createExternal(func)));
  }

  JNIEXPORT jlong JAVA_BIND(Engine, nativeCreateExternalGLX)(JNIEnv*, jclass)
  {
    return reinterpret_cast<jlong>(new f3d::engine(f3d::engine::createExternalGLX()));
  }

  JNIEXPORT jlong JAVA_BIND(Engine, nativeCreateExternalWGL)(JNIEnv*, jclass)
  {
    return reinterpret_cast<jlong>(new f3d::engine(f3d::engine::createExternalWGL()));
  }

  JNIEXPORT jlong JAVA_BIND(Engine, nativeCreateExternalCOCOA)(JNIEnv*, jclass)
  {
    return reinterpret_cast<jlong>(new f3d::engine(f3d::engine::createExternalCOCOA()));
  }

  JNIEXPORT jlong JAVA_BIND(Engine, nativeCreateExternalEGL)(JNIEnv*, jclass)
  {
    return reinterpret_cast<jlong>(new f3d::engine(f3d::engine::createExternalEGL()));
  }

  JNIEXPORT jlong JAVA_BIND(Engine, nativeCreateExternalOSMesa)(JNIEnv*, jclass)
  {
    return reinterpret_cast<jlong>(new f3d::engine(f3d::engine::createExternalOSMesa()));
  }

  JNIEXPORT void JAVA_BIND(Engine, nativeDestroy)(JNIEnv*, jclass, jlong ptr)
  {
    delete reinterpret_cast<f3d::engine*>(ptr);
  }

  JNIEXPORT void JAVA_BIND(Engine, loadPlugin)(JNIEnv* env, jclass, jstring str)
  {
    const char* plugin = env->GetStringUTFChars(str, nullptr);
    f3d::engine::loadPlugin(plugin);
    env->ReleaseStringUTFChars(str, plugin);
  }

  JNIEXPORT void JAVA_BIND(Engine, autoloadPlugins)(JNIEnv*, jclass)
  {
    f3d::engine::autoloadPlugins();
  }

  JNIEXPORT void JAVA_BIND(Engine, setCachePath)(JNIEnv* env, jobject self, jstring path)
  {
    const char* str = env->GetStringUTFChars(path, nullptr);
    GetEngine(env, self)->setCachePath(fs::path(str));
    env->ReleaseStringUTFChars(path, str);
  }

  JNIEXPORT void JAVA_BIND(Engine, setOptions)(JNIEnv* env, jobject self, jobject options)
  {
    jclass optionsClass = env->GetObjectClass(options);
    jfieldID fid = env->GetFieldID(optionsClass, "mNativeAddress", "J");
    jlong optionsPtr = env->GetLongField(options, fid);

    GetEngine(env, self)->setOptions(*reinterpret_cast<f3d::options*>(optionsPtr));
  }

  JNIEXPORT jobject JAVA_BIND(Engine, getInteractor)(JNIEnv* env, jobject self)
  {
    f3d::interactor& interactor = GetEngine(env, self)->getInteractor();

    jclass interactorClass = env->FindClass("app/f3d/F3D/Interactor");
    jmethodID constructor = env->GetMethodID(interactorClass, "<init>", "(J)V");

    return env->NewObject(interactorClass, constructor, reinterpret_cast<jlong>(&interactor));
  }

  JNIEXPORT jobject JAVA_BIND(Engine, getPluginsList)(JNIEnv* env, jclass, jstring path)
  {
    const char* str = env->GetStringUTFChars(path, nullptr);
    std::vector<std::string> plugins = f3d::engine::getPluginsList(fs::path(str));
    env->ReleaseStringUTFChars(path, str);

    return CreateStringList(env, plugins);
  }

  JNIEXPORT jobject JAVA_BIND(Engine, getLibInfo)(JNIEnv* env, jclass)
  {
    const f3d::engine::libInformation& info = f3d::engine::getLibInfo();

    jclass libInfoClass = env->FindClass("app/f3d/F3D/Engine$LibInfo");
    jmethodID constructor = env->GetMethodID(libInfoClass, "<init>",
      "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
      "Ljava/lang/String;Ljava/util/Map;Ljava/lang/String;Ljava/util/List;Ljava/lang/String;)V");

    jstring version = env->NewStringUTF(info.Version.c_str());
    jstring versionFull = env->NewStringUTF(info.VersionFull.c_str());
    jstring buildDate = env->NewStringUTF(info.BuildDate.c_str());
    jstring buildSystem = env->NewStringUTF(info.BuildSystem.c_str());
    jstring compiler = env->NewStringUTF(info.Compiler.c_str());
    jobject modules = CreateStringBooleanMap(env, info.Modules);
    jstring vtkVersion = env->NewStringUTF(info.VTKVersion.c_str());
    jobject copyrights = CreateStringList(env, info.Copyrights);
    jstring license = env->NewStringUTF(info.License.c_str());

    jobject libInfo = env->NewObject(libInfoClass, constructor, version, versionFull, buildDate,
      buildSystem, compiler, modules, vtkVersion, copyrights, license);

    env->DeleteLocalRef(version);
    env->DeleteLocalRef(versionFull);
    env->DeleteLocalRef(buildDate);
    env->DeleteLocalRef(buildSystem);
    env->DeleteLocalRef(compiler);
    env->DeleteLocalRef(modules);
    env->DeleteLocalRef(vtkVersion);
    env->DeleteLocalRef(copyrights);
    env->DeleteLocalRef(license);

    return libInfo;
  }

  JNIEXPORT jobject JAVA_BIND(Engine, getReadersInfo)(JNIEnv* env, jclass)
  {
    const std::vector<f3d::engine::readerInformation>& readers = f3d::engine::getReadersInfo();

    jclass arrayListClass = env->FindClass("java/util/ArrayList");
    jmethodID arrayListConstructor = env->GetMethodID(arrayListClass, "<init>", "()V");
    jmethodID addMethod = env->GetMethodID(arrayListClass, "add", "(Ljava/lang/Object;)Z");

    jobject list = env->NewObject(arrayListClass, arrayListConstructor);

    jclass readerInfoClass = env->FindClass("app/f3d/F3D/Engine$ReaderInfo");
    jmethodID readerInfoConstructor = env->GetMethodID(readerInfoClass, "<init>",
      "(Ljava/lang/String;Ljava/lang/String;Ljava/util/List;Ljava/util/List;Ljava/lang/"
      "String;ZZ)V");

    for (const auto& reader : readers)
    {
      jstring name = env->NewStringUTF(reader.Name.c_str());
      jstring description = env->NewStringUTF(reader.Description.c_str());
      jobject extensions = CreateStringList(env, reader.Extensions);
      jobject mimeTypes = CreateStringList(env, reader.MimeTypes);
      jstring pluginName = env->NewStringUTF(reader.PluginName.c_str());
      jboolean hasSceneReader = reader.HasSceneReader;
      jboolean hasGeometryReader = reader.HasGeometryReader;

      jobject readerInfo = env->NewObject(readerInfoClass, readerInfoConstructor, name, description,
        extensions, mimeTypes, pluginName, hasSceneReader, hasGeometryReader);

      env->CallBooleanMethod(list, addMethod, readerInfo);

      env->DeleteLocalRef(name);
      env->DeleteLocalRef(description);
      env->DeleteLocalRef(extensions);
      env->DeleteLocalRef(mimeTypes);
      env->DeleteLocalRef(pluginName);
      env->DeleteLocalRef(readerInfo);
    }

    return list;
  }

  JNIEXPORT jobject JAVA_BIND(Engine, getRenderingBackendList)(JNIEnv* env, jclass)
  {
    std::map<std::string, bool> backends = f3d::engine::getRenderingBackendList();
    return CreateStringBooleanMap(env, backends);
  }

  JNIEXPORT void JAVA_BIND(Engine, setReaderOption)(
    JNIEnv* env, jclass, jstring name, jstring value)
  {
    const char* nameStr = env->GetStringUTFChars(name, nullptr);
    const char* valueStr = env->GetStringUTFChars(value, nullptr);

    try
    {
      f3d::engine::setReaderOption(nameStr, valueStr);
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      env->ReleaseStringUTFChars(name, nameStr);
      env->ReleaseStringUTFChars(value, valueStr);

      jclass exceptionClass = env->FindClass("java/lang/IllegalArgumentException");
      env->ThrowNew(exceptionClass, e.what());
      return;
    }

    env->ReleaseStringUTFChars(name, nameStr);
    env->ReleaseStringUTFChars(value, valueStr);
  }

  JNIEXPORT jobject JAVA_BIND(Engine, getAllReaderOptionNames)(JNIEnv* env, jclass)
  {
    std::vector<std::string> names = f3d::engine::getAllReaderOptionNames();
    return CreateStringList(env, names);
  }
}
