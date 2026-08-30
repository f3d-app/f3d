#include "F3DJavaBindings.h"

#include <app_f3d_F3D_Engine.h>
#include <app_f3d_F3D_Engine_State.h>

#include <context.h>
#include <engine.h>
#include <scene.h>

extern "C"
{
  JNIEXPORT jlong JAVA_BIND(Engine, nativeCreate)(JNIEnv* env, jclass, jboolean offscreen)
  {
    try
    {
      return reinterpret_cast<jlong>(new f3d::engine(f3d::engine::create(offscreen)));
    }
    catch (const f3d::context::loading_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$LoadingException", e.what());
    }
    catch (const f3d::context::symbol_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$SymbolException", e.what());
    }
    catch (const f3d::engine::no_window_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Engine$NoWindowException", e.what());
    }
    catch (const f3d::engine::cache_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Engine$CacheException", e.what());
    }
    return 0;
  }

  JNIEXPORT jlong JAVA_BIND(Engine, nativeCreateNone)(JNIEnv* env, jclass)
  {
    try
    {
      return reinterpret_cast<jlong>(new f3d::engine(f3d::engine::createNone()));
    }
    catch (const f3d::engine::no_window_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Engine$NoWindowException", e.what());
    }
    catch (const f3d::engine::cache_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Engine$CacheException", e.what());
    }
    return 0;
  }

  JNIEXPORT jlong JAVA_BIND(Engine, nativeCreateGLX)(JNIEnv* env, jclass, jboolean offscreen)
  {
    try
    {
      return reinterpret_cast<jlong>(new f3d::engine(f3d::engine::createGLX(offscreen)));
    }
    catch (const f3d::context::loading_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$LoadingException", e.what());
    }
    catch (const f3d::context::symbol_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$SymbolException", e.what());
    }
    catch (const f3d::engine::no_window_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Engine$NoWindowException", e.what());
    }
    catch (const f3d::engine::cache_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Engine$CacheException", e.what());
    }
    return 0;
  }

  JNIEXPORT jlong JAVA_BIND(Engine, nativeCreateWGL)(JNIEnv* env, jclass, jboolean offscreen)
  {
    try
    {
      return reinterpret_cast<jlong>(new f3d::engine(f3d::engine::createWGL(offscreen)));
    }
    catch (const f3d::context::loading_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$LoadingException", e.what());
    }
    catch (const f3d::context::symbol_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$SymbolException", e.what());
    }
    catch (const f3d::engine::no_window_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Engine$NoWindowException", e.what());
    }
    catch (const f3d::engine::cache_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Engine$CacheException", e.what());
    }
    return 0;
  }

  JNIEXPORT jlong JAVA_BIND(Engine, nativeCreateEGL)(JNIEnv* env, jclass)
  {
    try
    {
      return reinterpret_cast<jlong>(new f3d::engine(f3d::engine::createEGL()));
    }
    catch (const f3d::context::loading_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$LoadingException", e.what());
    }
    catch (const f3d::context::symbol_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$SymbolException", e.what());
    }
    catch (const f3d::engine::no_window_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Engine$NoWindowException", e.what());
    }
    catch (const f3d::engine::cache_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Engine$CacheException", e.what());
    }
    return 0;
  }

  JNIEXPORT jlong JAVA_BIND(Engine, nativeCreateOSMesa)(JNIEnv* env, jclass)
  {
    try
    {
      return reinterpret_cast<jlong>(new f3d::engine(f3d::engine::createOSMesa()));
    }
    catch (const f3d::context::loading_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$LoadingException", e.what());
    }
    catch (const f3d::context::symbol_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$SymbolException", e.what());
    }
    catch (const f3d::engine::no_window_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Engine$NoWindowException", e.what());
    }
    catch (const f3d::engine::cache_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Engine$CacheException", e.what());
    }
    return 0;
  }

  JNIEXPORT jlong JAVA_BIND(Engine, nativeCreateExternal)(
    JNIEnv* env, jclass, jobject getProcAddress)
  {
    if (!getProcAddress)
    {
      return 0;
    }

    jobject globalRef = env->NewGlobalRef(getProcAddress);

    JniLocalRef<jclass> contextFunctionClass(env, env->GetObjectClass(getProcAddress));
    jmethodID methodID =
      env->GetMethodID(contextFunctionClass, "getProcAddress", "(Ljava/lang/String;)J");

    f3d::context::function func = [env, globalRef, methodID](const char* name) -> f3d::context::fptr
    {
      JniLocalRef<jstring> jname(env, env->NewStringUTF(name));
      jlong addr = env->CallLongMethod(globalRef, methodID, jname.get());
      return reinterpret_cast<f3d::context::fptr>(addr);
    };

    try
    {
      return reinterpret_cast<jlong>(new f3d::engine(f3d::engine::createExternal(func)));
    }
    catch (const f3d::engine::no_window_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Engine$NoWindowException", e.what());
    }
    catch (const f3d::engine::cache_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Engine$CacheException", e.what());
    }
    return 0;
  }

  JNIEXPORT jlong JAVA_BIND(Engine, nativeCreateExternalGLX)(JNIEnv* env, jclass)
  {
    try
    {
      return reinterpret_cast<jlong>(new f3d::engine(f3d::engine::createExternalGLX()));
    }
    catch (const f3d::context::loading_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$LoadingException", e.what());
    }
    catch (const f3d::context::symbol_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$SymbolException", e.what());
    }
    catch (const f3d::engine::no_window_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Engine$NoWindowException", e.what());
    }
    catch (const f3d::engine::cache_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Engine$CacheException", e.what());
    }
    return 0;
  }

  JNIEXPORT jlong JAVA_BIND(Engine, nativeCreateExternalWGL)(JNIEnv* env, jclass)
  {
    try
    {
      return reinterpret_cast<jlong>(new f3d::engine(f3d::engine::createExternalWGL()));
    }
    catch (const f3d::context::loading_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$LoadingException", e.what());
    }
    catch (const f3d::context::symbol_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$SymbolException", e.what());
    }
    catch (const f3d::engine::no_window_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Engine$NoWindowException", e.what());
    }
    catch (const f3d::engine::cache_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Engine$CacheException", e.what());
    }
    return 0;
  }

  JNIEXPORT jlong JAVA_BIND(Engine, nativeCreateExternalCOCOA)(JNIEnv* env, jclass)
  {
    try
    {
      return reinterpret_cast<jlong>(new f3d::engine(f3d::engine::createExternalCOCOA()));
    }
    catch (const f3d::context::loading_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$LoadingException", e.what());
    }
    catch (const f3d::context::symbol_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$SymbolException", e.what());
    }
    catch (const f3d::engine::no_window_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Engine$NoWindowException", e.what());
    }
    catch (const f3d::engine::cache_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Engine$CacheException", e.what());
    }
    return 0;
  }

  JNIEXPORT jlong JAVA_BIND(Engine, nativeCreateExternalEGL)(JNIEnv* env, jclass)
  {
    try
    {
      return reinterpret_cast<jlong>(new f3d::engine(f3d::engine::createExternalEGL()));
    }
    catch (const f3d::context::loading_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$LoadingException", e.what());
    }
    catch (const f3d::context::symbol_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$SymbolException", e.what());
    }
    catch (const f3d::engine::no_window_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Engine$NoWindowException", e.what());
    }
    catch (const f3d::engine::cache_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Engine$CacheException", e.what());
    }
    return 0;
  }

  JNIEXPORT jlong JAVA_BIND(Engine, nativeCreateExternalOSMesa)(JNIEnv* env, jclass)
  {
    try
    {
      return reinterpret_cast<jlong>(new f3d::engine(f3d::engine::createExternalOSMesa()));
    }
    catch (const f3d::context::loading_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$LoadingException", e.what());
    }
    catch (const f3d::context::symbol_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Context$SymbolException", e.what());
    }
    catch (const f3d::engine::no_window_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Engine$NoWindowException", e.what());
    }
    catch (const f3d::engine::cache_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Engine$CacheException", e.what());
    }
    return 0;
  }

  JNIEXPORT void JAVA_BIND(Engine, nativeDestroy)(JNIEnv*, jclass, jlong ptr)
  {
    delete reinterpret_cast<f3d::engine*>(ptr);
  }

  JNIEXPORT void JAVA_BIND(Engine, loadPlugin)(JNIEnv* env, jclass, jstring str)
  {
    JniUTFString plugin(env, str);
    try
    {
      f3d::engine::loadPlugin(plugin.c_str());
    }
    catch (const f3d::engine::plugin_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Engine$PluginException", e.what());
    }
  }

  JNIEXPORT void JAVA_BIND(Engine, autoloadPlugins)(JNIEnv*, jclass)
  {
    f3d::engine::autoloadPlugins();
  }

  JNIEXPORT void JAVA_BIND(Engine, setCachePath)(JNIEnv* env, jobject self, jstring path)
  {
    JniUTFString str(env, path);
    try
    {
      GetEngine(env, self)->setCachePath(fs::path(str.c_str()));
    }
    catch (const f3d::engine::cache_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Engine$CacheException", e.what());
    }
  }

  JNIEXPORT jstring JAVA_BIND(Engine, getCachePath)(JNIEnv* env, jobject self)
  {
    return env->NewStringUTF(GetEngine(env, self)->getCachePath().string().c_str());
  }

  JNIEXPORT jlong JAVA_BIND(Engine, nativeDump)(JNIEnv* env, jobject self)
  {
    try
    {
      return reinterpret_cast<jlong>(new f3d::engine::state(GetEngine(env, self)->dump()));
    }
    catch (const f3d::engine::statefile_exception& e)
    {
      F3DThrowJavaException(env, "java/lang/RuntimeException", e.what());
      return 0;
    }
  }

  JNIEXPORT void JAVA_BIND(Engine, nativeLoad)(JNIEnv* env, jobject self, jlong stateAddress)
  {
    try
    {
      GetEngine(env, self)->load(*reinterpret_cast<f3d::engine::state*>(stateAddress));
    }
    catch (const f3d::engine::statefile_exception& e)
    {
      F3DThrowJavaException(env, "java/lang/RuntimeException", e.what());
    }
    catch (const f3d::scene::load_failure_exception& e)
    {
      F3DThrowJavaException(env, "java/lang/RuntimeException", e.what());
    }
  }

  JNIEXPORT jlong JAVA_SCOPED_BIND(Engine, State, nativeFromString)(
    JNIEnv* env, jclass, jstring content)
  {
    JniUTFString str(env, content);
    jlong ptr = 0;
    try
    {
      ptr = reinterpret_cast<jlong>(
        new f3d::engine::state(f3d::engine::state::fromString(str.c_str())));
    }
    catch (const f3d::engine::statefile_exception& e)
    {
      F3DThrowJavaException(env, "java/lang/RuntimeException", e.what());
    }
    return ptr;
  }

  JNIEXPORT jlong JAVA_SCOPED_BIND(Engine, State, nativeFromFile)(JNIEnv* env, jclass, jstring path)
  {
    JniUTFString str(env, path);
    jlong ptr = 0;
    try
    {
      ptr = reinterpret_cast<jlong>(
        new f3d::engine::state(f3d::engine::state::fromFile(fs::path(str.c_str()))));
    }
    catch (const f3d::engine::statefile_exception& e)
    {
      F3DThrowJavaException(env, "java/lang/RuntimeException", e.what());
    }
    return ptr;
  }

  JNIEXPORT jlong JAVA_SCOPED_BIND(Engine, State, nativeFromClipboard)(JNIEnv* env, jclass)
  {
    try
    {
      return reinterpret_cast<jlong>(new f3d::engine::state(f3d::engine::state::fromClipboard()));
    }
    catch (const f3d::engine::statefile_exception& e)
    {
      F3DThrowJavaException(env, "java/lang/RuntimeException", e.what());
      return 0;
    }
  }

  JNIEXPORT jstring JAVA_SCOPED_BIND(Engine, State, toString)(JNIEnv* env, jobject self)
  {
    return env->NewStringUTF(GetState(env, self)->toString().c_str());
  }

  JNIEXPORT void JAVA_SCOPED_BIND(Engine, State, toFile)(JNIEnv* env, jobject self, jstring path)
  {
    JniUTFString str(env, path);
    try
    {
      GetState(env, self)->toFile(fs::path(str.c_str()));
    }
    catch (const f3d::engine::statefile_exception& e)
    {
      F3DThrowJavaException(env, "java/lang/RuntimeException", e.what());
    }
  }

  JNIEXPORT void JAVA_SCOPED_BIND(Engine, State, toClipboard)(JNIEnv* env, jobject self)
  {
    try
    {
      GetState(env, self)->toClipboard();
    }
    catch (const f3d::engine::statefile_exception& e)
    {
      F3DThrowJavaException(env, "java/lang/RuntimeException", e.what());
    }
  }

  JNIEXPORT void JAVA_SCOPED_BIND(Engine, State, nativeDestroy)(JNIEnv*, jclass, jlong ptr)
  {
    delete reinterpret_cast<f3d::engine::state*>(ptr);
  }

  JNIEXPORT void JAVA_BIND(Engine, setOptions)(JNIEnv* env, jobject self, jobject options)
  {
    JniLocalRef<jclass> optionsClass(env, env->GetObjectClass(options));
    jfieldID fid = env->GetFieldID(optionsClass, "mNativeAddress", "J");
    jlong optionsPtr = env->GetLongField(options, fid);

    GetEngine(env, self)->setOptions(*reinterpret_cast<f3d::options*>(optionsPtr));
  }

  JNIEXPORT jobject JAVA_BIND(Engine, getInteractor)(JNIEnv* env, jobject self)
  {
    try
    {
      f3d::interactor& interactor = GetEngine(env, self)->getInteractor();

      JniLocalRef<jclass> interactorClass(env, env->FindClass("app/f3d/F3D/Interactor"));
      jmethodID constructor = env->GetMethodID(interactorClass, "<init>", "(J)V");

      return env->NewObject(interactorClass, constructor, reinterpret_cast<jlong>(&interactor));
    }
    catch (const f3d::engine::no_interactor_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Engine$NoInteractorException", e.what());
    }
    return nullptr;
  }

  JNIEXPORT jobject JAVA_BIND(Engine, getPluginsList)(JNIEnv* env, jclass, jstring path)
  {
    JniUTFString str(env, path);
    std::vector<std::string> plugins = f3d::engine::getPluginsList(fs::path(str.c_str()));

    return CreateStringList(env, plugins);
  }

  JNIEXPORT jobject JAVA_BIND(Engine, getLibInfo)(JNIEnv* env, jclass)
  {
    const f3d::engine::libInformation& info = f3d::engine::getLibInfo();

    JniLocalRef<jclass> libInfoClass(env, env->FindClass("app/f3d/F3D/Engine$LibInfo"));
    jmethodID constructor = env->GetMethodID(libInfoClass, "<init>",
      "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
      "Ljava/lang/String;Ljava/util/Map;Ljava/lang/String;Ljava/util/List;Ljava/lang/String;)V");

    JniLocalRef<jstring> version(env, env->NewStringUTF(info.Version.c_str()));
    JniLocalRef<jstring> versionFull(env, env->NewStringUTF(info.VersionFull.c_str()));
    JniLocalRef<jstring> buildDate(env, env->NewStringUTF(info.BuildDate.c_str()));
    JniLocalRef<jstring> buildSystem(env, env->NewStringUTF(info.BuildSystem.c_str()));
    JniLocalRef<jstring> compiler(env, env->NewStringUTF(info.Compiler.c_str()));
    JniLocalRef<jobject> modules(env, CreateStringBooleanMap(env, info.Modules));
    JniLocalRef<jstring> vtkVersion(env, env->NewStringUTF(info.VTKVersion.c_str()));
    JniLocalRef<jobject> copyrights(env, CreateStringList(env, info.Copyrights));
    JniLocalRef<jstring> license(env, env->NewStringUTF(info.License.c_str()));

    jobject libInfo = env->NewObject(libInfoClass, constructor, version.get(), versionFull.get(),
      buildDate.get(), buildSystem.get(), compiler.get(), modules.get(), vtkVersion.get(),
      copyrights.get(), license.get());

    return libInfo;
  }

  JNIEXPORT jobject JAVA_BIND(Engine, getReadersInfo)(JNIEnv* env, jclass)
  {
    const std::vector<f3d::engine::readerInformation>& readers = f3d::engine::getReadersInfo();

    JniLocalRef<jclass> arrayListClass(env, env->FindClass("java/util/ArrayList"));
    jmethodID arrayListConstructor = env->GetMethodID(arrayListClass, "<init>", "()V");
    jmethodID addMethod = env->GetMethodID(arrayListClass, "add", "(Ljava/lang/Object;)Z");

    jobject list = env->NewObject(arrayListClass, arrayListConstructor);

    JniLocalRef<jclass> readerInfoClass(env, env->FindClass("app/f3d/F3D/Engine$ReaderInfo"));
    jmethodID readerInfoConstructor = env->GetMethodID(readerInfoClass, "<init>",
      "(Ljava/lang/String;Ljava/lang/String;Ljava/util/List;Ljava/util/List;Ljava/lang/"
      "String;ZZ)V");

    for (const auto& reader : readers)
    {
      JniLocalRef<jstring> name(env, env->NewStringUTF(reader.Name.c_str()));
      JniLocalRef<jstring> description(env, env->NewStringUTF(reader.Description.c_str()));
      JniLocalRef<jobject> extensions(env, CreateStringList(env, reader.Extensions));
      JniLocalRef<jobject> mimeTypes(env, CreateStringList(env, reader.MimeTypes));
      JniLocalRef<jstring> pluginName(env, env->NewStringUTF(reader.PluginName.c_str()));
      jboolean hasSceneReader = reader.HasSceneReader;
      jboolean hasGeometryReader = reader.HasGeometryReader;

      JniLocalRef<jobject> readerInfo(env,
        env->NewObject(readerInfoClass, readerInfoConstructor, name.get(), description.get(),
          extensions.get(), mimeTypes.get(), pluginName.get(), hasSceneReader, hasGeometryReader));

      env->CallBooleanMethod(list, addMethod, readerInfo.get());
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
    JniUTFString nameStr(env, name);
    JniUTFString valueStr(env, value);

    try
    {
      f3d::engine::setReaderOption(nameStr.c_str(), valueStr.c_str());
    }
    catch (const f3d::options::inexistent_exception& e)
    {
      F3DThrowJavaException(env, "app/f3d/F3D/Options$InexistentException", e.what());
    }
  }

  JNIEXPORT jobject JAVA_BIND(Engine, getAllReaderOptionNames)(JNIEnv* env, jclass)
  {
    std::vector<std::string> names = f3d::engine::getAllReaderOptionNames();
    return CreateStringList(env, names);
  }
}
