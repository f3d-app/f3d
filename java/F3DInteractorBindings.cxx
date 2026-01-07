#include "F3DJavaBindings.h"

#include <app_f3d_F3D_Interactor.h>

#include <interactor.h>

#include <map>

namespace
{
std::map<std::string, jobject> g_commandCallbacks;
JavaVM* g_jvm = nullptr;

f3d::interactor& GetInteractor(JNIEnv* env, jobject self)
{
  jclass cls = env->GetObjectClass(self);
  jfieldID fid = env->GetFieldID(cls, "mNativeAddress", "J");
  jlong ptr = env->GetLongField(self, fid);

  if (g_jvm == nullptr)
  {
    env->GetJavaVM(&g_jvm);
  }

  return *reinterpret_cast<f3d::interactor*>(ptr);
}

f3d::interaction_bind_t::ModifierKeys JavaModToNative(jint mod)
{
  switch (mod)
  {
    case 0:
      return f3d::interaction_bind_t::ModifierKeys::ANY;
    case 1:
      return f3d::interaction_bind_t::ModifierKeys::NONE;
    case 2:
      return f3d::interaction_bind_t::ModifierKeys::CTRL;
    case 3:
      return f3d::interaction_bind_t::ModifierKeys::SHIFT;
    case 4:
      return f3d::interaction_bind_t::ModifierKeys::CTRL_SHIFT;
    default:
      return f3d::interaction_bind_t::ModifierKeys::NONE;
  }
}

jint NativeModToJava(f3d::interaction_bind_t::ModifierKeys mod)
{
  switch (mod)
  {
    case f3d::interaction_bind_t::ModifierKeys::ANY:
      return 0;
    case f3d::interaction_bind_t::ModifierKeys::NONE:
      return 1;
    case f3d::interaction_bind_t::ModifierKeys::CTRL:
      return 2;
    case f3d::interaction_bind_t::ModifierKeys::SHIFT:
      return 3;
    case f3d::interaction_bind_t::ModifierKeys::CTRL_SHIFT:
      return 4;
    default:
      return 1;
  }
}

f3d::interaction_bind_t JavaBindToNative(JNIEnv* env, jobject bind)
{
  f3d::interaction_bind_t nativeBind;
  jclass bindClass = env->GetObjectClass(bind);

  jfieldID modField = env->GetFieldID(bindClass, "mod", "Lapp/f3d/F3D/Interactor$ModifierKeys;");
  jobject modObj = env->GetObjectField(bind, modField);
  jclass modEnum = env->GetObjectClass(modObj);
  jmethodID ordinalMethod = env->GetMethodID(modEnum, "ordinal", "()I");
  jint modOrdinal = env->CallIntMethod(modObj, ordinalMethod);
  nativeBind.mod = JavaModToNative(modOrdinal);

  jfieldID interField = env->GetFieldID(bindClass, "inter", "Ljava/lang/String;");
  jstring interStr = static_cast<jstring>(env->GetObjectField(bind, interField));
  const char* interCStr = env->GetStringUTFChars(interStr, nullptr);
  nativeBind.inter = interCStr;
  env->ReleaseStringUTFChars(interStr, interCStr);

  return nativeBind;
}

jobject NativeBindToJava(JNIEnv* env, const f3d::interaction_bind_t& bind)
{
  jclass bindClass = env->FindClass("app/f3d/F3D/Interactor$InteractionBind");
  jmethodID constructor = env->GetMethodID(bindClass, "<init>", "()V");
  jobject bindObj = env->NewObject(bindClass, constructor);

  jclass modEnum = env->FindClass("app/f3d/F3D/Interactor$ModifierKeys");
  jmethodID valuesMethod =
    env->GetStaticMethodID(modEnum, "values", "()[Lapp/f3d/F3D/Interactor$ModifierKeys;");
  jobjectArray modsArray =
    static_cast<jobjectArray>(env->CallStaticObjectMethod(modEnum, valuesMethod));
  jobject modObj = env->GetObjectArrayElement(modsArray, NativeModToJava(bind.mod));

  jfieldID modField = env->GetFieldID(bindClass, "mod", "Lapp/f3d/F3D/Interactor$ModifierKeys;");
  env->SetObjectField(bindObj, modField, modObj);

  jfieldID interField = env->GetFieldID(bindClass, "inter", "Ljava/lang/String;");
  jstring interStr = env->NewStringUTF(bind.inter.c_str());
  env->SetObjectField(bindObj, interField, interStr);

  return bindObj;
}
}

extern "C"
{
  JNIEXPORT jobject JAVA_BIND(Interactor, initCommands)(JNIEnv* env, jobject self)
  {
    GetInteractor(env, self).initCommands();
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Interactor, addCommand)(
    JNIEnv* env, jobject self, jstring action, jobject callback)
  {
    const char* actionStr = env->GetStringUTFChars(action, nullptr);
    std::string actionCpp = actionStr;
    env->ReleaseStringUTFChars(action, actionStr);

    g_commandCallbacks[actionCpp] = env->NewGlobalRef(callback);

    auto cppCallback = [actionCpp](const std::vector<std::string>& args)
    {
      JNIEnv* env = nullptr;

#ifdef __ANDROID__
      if (g_jvm->AttachCurrentThread(&env, nullptr) != JNI_OK)
#else
      if (g_jvm->AttachCurrentThread(reinterpret_cast<void**>(&env), nullptr) != JNI_OK)
#endif
      {
        return;
      }

      jobject callback = g_commandCallbacks[actionCpp];
      if (callback == nullptr)
      {
        g_jvm->DetachCurrentThread();
        return;
      }

      jclass arrayListClass = env->FindClass("java/util/ArrayList");
      jmethodID arrayListConstructor = env->GetMethodID(arrayListClass, "<init>", "()V");
      jmethodID addMethod = env->GetMethodID(arrayListClass, "add", "(Ljava/lang/Object;)Z");
      jobject argsList = env->NewObject(arrayListClass, arrayListConstructor);

      for (const auto& arg : args)
      {
        jstring jstr = env->NewStringUTF(arg.c_str());
        env->CallBooleanMethod(argsList, addMethod, jstr);
        env->DeleteLocalRef(jstr);
      }

      jclass callbackClass = env->GetObjectClass(callback);
      jmethodID executeMethod = env->GetMethodID(callbackClass, "execute", "(Ljava/util/List;)V");
      env->CallVoidMethod(callback, executeMethod, argsList);

      env->DeleteLocalRef(argsList);

      g_jvm->DetachCurrentThread();
    };

    GetInteractor(env, self).addCommand(actionCpp, cppCallback);
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Interactor, removeCommand)(JNIEnv* env, jobject self, jstring action)
  {
    const char* actionStr = env->GetStringUTFChars(action, nullptr);
    std::string actionCpp = actionStr;
    env->ReleaseStringUTFChars(action, actionStr);

    auto it = g_commandCallbacks.find(actionCpp);
    if (it != g_commandCallbacks.end())
    {
      env->DeleteGlobalRef(it->second);
      g_commandCallbacks.erase(it);
    }

    GetInteractor(env, self).removeCommand(actionCpp);
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Interactor, getCommandActions)(JNIEnv* env, jobject self)
  {
    return CreateStringList(env, GetInteractor(env, self).getCommandActions());
  }

  JNIEXPORT jboolean JAVA_BIND(Interactor, triggerCommand)(
    JNIEnv* env, jobject self, jstring command, jboolean keepComments)
  {
    const char* commandStr = env->GetStringUTFChars(command, nullptr);
    bool result = GetInteractor(env, self).triggerCommand(commandStr, keepComments);
    env->ReleaseStringUTFChars(command, commandStr);
    return result;
  }

  JNIEXPORT jobject JAVA_BIND(Interactor, initBindings)(JNIEnv* env, jobject self)
  {
    GetInteractor(env, self).initBindings();
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Interactor, addBindingCommands)(
    JNIEnv* env, jobject self, jobject bind, jobject commands, jstring group, jobject type)
  {
    f3d::interaction_bind_t nativeBind = JavaBindToNative(env, bind);

    jclass listClass = env->GetObjectClass(commands);
    jmethodID sizeMethod = env->GetMethodID(listClass, "size", "()I");
    jmethodID getMethod = env->GetMethodID(listClass, "get", "(I)Ljava/lang/Object;");
    jint size = env->CallIntMethod(commands, sizeMethod);

    std::vector<std::string> commandsVec;
    for (jint i = 0; i < size; i++)
    {
      jstring cmdStr = static_cast<jstring>(env->CallObjectMethod(commands, getMethod, i));
      const char* cmdCStr = env->GetStringUTFChars(cmdStr, nullptr);
      commandsVec.push_back(cmdCStr);
      env->ReleaseStringUTFChars(cmdStr, cmdCStr);
    }

    const char* groupStr = env->GetStringUTFChars(group, nullptr);
    std::string groupCpp = groupStr;
    env->ReleaseStringUTFChars(group, groupStr);

    jclass typeEnum = env->GetObjectClass(type);
    jmethodID ordinalMethod = env->GetMethodID(typeEnum, "ordinal", "()I");
    jint typeOrdinal = env->CallIntMethod(type, ordinalMethod);

    f3d::interactor::BindingType nativeType;
    switch (typeOrdinal)
    {
      case 0:
        nativeType = f3d::interactor::BindingType::CYCLIC;
        break;
      case 1:
        nativeType = f3d::interactor::BindingType::NUMERICAL;
        break;
      case 2:
        nativeType = f3d::interactor::BindingType::TOGGLE;
        break;
      case 3:
      default:
        nativeType = f3d::interactor::BindingType::OTHER;
        break;
    }

    GetInteractor(env, self).addBinding(nativeBind, commandsVec, groupCpp, nullptr, nativeType);
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Interactor, addBindingCommand)(
    JNIEnv* env, jobject self, jobject bind, jstring command, jstring group, jobject type)
  {
    f3d::interaction_bind_t nativeBind = JavaBindToNative(env, bind);

    const char* commandStr = env->GetStringUTFChars(command, nullptr);
    std::string commandCpp = commandStr;
    env->ReleaseStringUTFChars(command, commandStr);

    const char* groupStr = env->GetStringUTFChars(group, nullptr);
    std::string groupCpp = groupStr;
    env->ReleaseStringUTFChars(group, groupStr);

    jclass typeEnum = env->GetObjectClass(type);
    jmethodID ordinalMethod = env->GetMethodID(typeEnum, "ordinal", "()I");
    jint typeOrdinal = env->CallIntMethod(type, ordinalMethod);

    f3d::interactor::BindingType nativeType;
    switch (typeOrdinal)
    {
      case 0:
        nativeType = f3d::interactor::BindingType::CYCLIC;
        break;
      case 1:
        nativeType = f3d::interactor::BindingType::NUMERICAL;
        break;
      case 2:
        nativeType = f3d::interactor::BindingType::TOGGLE;
        break;
      case 3:
      default:
        nativeType = f3d::interactor::BindingType::OTHER;
        break;
    }

    GetInteractor(env, self).addBinding(nativeBind, commandCpp, groupCpp, nullptr, nativeType);
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Interactor, removeBinding)(JNIEnv* env, jobject self, jobject bind)
  {
    f3d::interaction_bind_t nativeBind = JavaBindToNative(env, bind);
    GetInteractor(env, self).removeBinding(nativeBind);
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Interactor, getBindGroups)(JNIEnv* env, jobject self)
  {
    return CreateStringList(env, GetInteractor(env, self).getBindGroups());
  }

  JNIEXPORT jobject JAVA_BIND(Interactor, getBindsForGroup)(
    JNIEnv* env, jobject self, jstring group)
  {
    const char* groupStr = env->GetStringUTFChars(group, nullptr);
    std::vector<f3d::interaction_bind_t> binds =
      GetInteractor(env, self).getBindsForGroup(groupStr);
    env->ReleaseStringUTFChars(group, groupStr);

    jclass arrayListClass = env->FindClass("java/util/ArrayList");
    jmethodID arrayListConstructor = env->GetMethodID(arrayListClass, "<init>", "()V");
    jmethodID addMethod = env->GetMethodID(arrayListClass, "add", "(Ljava/lang/Object;)Z");
    jobject list = env->NewObject(arrayListClass, arrayListConstructor);

    for (const auto& bind : binds)
    {
      jobject bindObj = NativeBindToJava(env, bind);
      env->CallBooleanMethod(list, addMethod, bindObj);
      env->DeleteLocalRef(bindObj);
    }

    return list;
  }

  JNIEXPORT jobject JAVA_BIND(Interactor, getBinds)(JNIEnv* env, jobject self)
  {
    std::vector<f3d::interaction_bind_t> binds = GetInteractor(env, self).getBinds();

    jclass arrayListClass = env->FindClass("java/util/ArrayList");
    jmethodID arrayListConstructor = env->GetMethodID(arrayListClass, "<init>", "()V");
    jmethodID addMethod = env->GetMethodID(arrayListClass, "add", "(Ljava/lang/Object;)Z");
    jobject list = env->NewObject(arrayListClass, arrayListConstructor);

    for (const auto& bind : binds)
    {
      jobject bindObj = NativeBindToJava(env, bind);
      env->CallBooleanMethod(list, addMethod, bindObj);
      env->DeleteLocalRef(bindObj);
    }

    return list;
  }

  JNIEXPORT jobject JAVA_BIND(Interactor, getBindingDocumentation)(
    JNIEnv* env, jobject self, jobject bind)
  {
    f3d::interaction_bind_t nativeBind = JavaBindToNative(env, bind);
    auto doc = GetInteractor(env, self).getBindingDocumentation(nativeBind);

    jclass docClass = env->FindClass("app/f3d/F3D/Interactor$BindingDocumentation");
    jmethodID constructor =
      env->GetMethodID(docClass, "<init>", "(Ljava/lang/String;Ljava/lang/String;)V");

    jstring docStr = env->NewStringUTF(doc.first.c_str());
    jstring valueStr = env->NewStringUTF(doc.second.c_str());

    jobject docObj = env->NewObject(docClass, constructor, docStr, valueStr);

    env->DeleteLocalRef(docStr);
    env->DeleteLocalRef(valueStr);

    return docObj;
  }

  JNIEXPORT jobject JAVA_BIND(Interactor, getBindingType)(JNIEnv* env, jobject self, jobject bind)
  {
    f3d::interaction_bind_t nativeBind = JavaBindToNative(env, bind);
    f3d::interactor::BindingType type = GetInteractor(env, self).getBindingType(nativeBind);

    jclass enumClass = env->FindClass("app/f3d/F3D/Interactor$BindingType");
    jfieldID fieldID;

    switch (type)
    {
      case f3d::interactor::BindingType::CYCLIC:
        fieldID =
          env->GetStaticFieldID(enumClass, "CYCLIC", "Lapp/f3d/F3D/Interactor$BindingType;");
        break;
      case f3d::interactor::BindingType::NUMERICAL:
        fieldID =
          env->GetStaticFieldID(enumClass, "NUMERICAL", "Lapp/f3d/F3D/Interactor$BindingType;");
        break;
      case f3d::interactor::BindingType::TOGGLE:
        fieldID =
          env->GetStaticFieldID(enumClass, "TOGGLE", "Lapp/f3d/F3D/Interactor$BindingType;");
        break;
      case f3d::interactor::BindingType::OTHER:
      default:
        fieldID = env->GetStaticFieldID(enumClass, "OTHER", "Lapp/f3d/F3D/Interactor$BindingType;");
        break;
    }

    return env->GetStaticObjectField(enumClass, fieldID);
  }

  JNIEXPORT jobject JAVA_BIND(Interactor, toggleAnimation)(
    JNIEnv* env, jobject self, jobject direction)
  {
    jclass directionEnum = env->GetObjectClass(direction);
    jmethodID getValueMethod = env->GetMethodID(directionEnum, "getValue", "()I");
    jint directionValue = env->CallIntMethod(direction, getValueMethod);

    f3d::interactor::AnimationDirection nativeDirection =
      static_cast<f3d::interactor::AnimationDirection>(directionValue);

    GetInteractor(env, self).toggleAnimation(nativeDirection);
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Interactor, startAnimation)(
    JNIEnv* env, jobject self, jobject direction)
  {
    jclass directionEnum = env->GetObjectClass(direction);
    jmethodID getValueMethod = env->GetMethodID(directionEnum, "getValue", "()I");
    jint directionValue = env->CallIntMethod(direction, getValueMethod);

    f3d::interactor::AnimationDirection nativeDirection =
      static_cast<f3d::interactor::AnimationDirection>(directionValue);

    GetInteractor(env, self).startAnimation(nativeDirection);
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Interactor, stopAnimation)(JNIEnv* env, jobject self)
  {
    GetInteractor(env, self).stopAnimation();
    return self;
  }

  JNIEXPORT jboolean JAVA_BIND(Interactor, isPlayingAnimation)(JNIEnv* env, jobject self)
  {
    return GetInteractor(env, self).isPlayingAnimation();
  }

  JNIEXPORT jobject JAVA_BIND(Interactor, getAnimationDirection)(JNIEnv* env, jobject self)
  {
    f3d::interactor::AnimationDirection nativeDirection =
      GetInteractor(env, self).getAnimationDirection();

    jclass enumClass = env->FindClass("app/f3d/F3D/Interactor$AnimationDirection");
    jmethodID fromValueMethod = env->GetStaticMethodID(
      enumClass, "fromValue", "(I)Lapp/f3d/F3D/Interactor$AnimationDirection;");

    return env->CallStaticObjectMethod(
      enumClass, fromValueMethod, static_cast<int>(nativeDirection));
  }

  JNIEXPORT jstring JAVA_BIND(Interactor, getAnimationName)(JNIEnv* env, jobject self, jint indices)
  {
    return env->NewStringUTF(GetInteractor(env, self).getAnimationName(indices).c_str());
  }

  JNIEXPORT jobject JAVA_BIND(Interactor, getAnimationNames)(JNIEnv* env, jobject self)
  {
    return CreateStringList(env, GetInteractor(env, self).getAnimationNames());
  }

  JNIEXPORT jobject JAVA_BIND(Interactor, enableCameraMovement)(JNIEnv* env, jobject self)
  {
    GetInteractor(env, self).enableCameraMovement();
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Interactor, disableCameraMovement)(JNIEnv* env, jobject self)
  {
    GetInteractor(env, self).disableCameraMovement();
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Interactor, triggerModUpdate)(JNIEnv* env, jobject self, jobject mod)
  {
    jclass modEnum = env->GetObjectClass(mod);
    jmethodID ordinalMethod = env->GetMethodID(modEnum, "ordinal", "()I");
    jint modOrdinal = env->CallIntMethod(mod, ordinalMethod);

    f3d::interactor::InputModifier nativeMod;
    switch (modOrdinal)
    {
      case 0:
        nativeMod = f3d::interactor::InputModifier::NONE;
        break;
      case 1:
        nativeMod = f3d::interactor::InputModifier::CTRL;
        break;
      case 2:
        nativeMod = f3d::interactor::InputModifier::SHIFT;
        break;
      case 3:
        nativeMod = f3d::interactor::InputModifier::CTRL_SHIFT;
        break;
      default:
        nativeMod = f3d::interactor::InputModifier::NONE;
        break;
    }

    GetInteractor(env, self).triggerModUpdate(nativeMod);
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Interactor, triggerMouseButton)(
    JNIEnv* env, jobject self, jobject action, jobject button)
  {
    jclass actionEnum = env->GetObjectClass(action);
    jmethodID actionOrdinalMethod = env->GetMethodID(actionEnum, "ordinal", "()I");
    jint actionOrdinal = env->CallIntMethod(action, actionOrdinalMethod);

    jclass buttonEnum = env->GetObjectClass(button);
    jmethodID buttonOrdinalMethod = env->GetMethodID(buttonEnum, "ordinal", "()I");
    jint buttonOrdinal = env->CallIntMethod(button, buttonOrdinalMethod);

    f3d::interactor::InputAction nativeAction = actionOrdinal == 0
      ? f3d::interactor::InputAction::PRESS
      : f3d::interactor::InputAction::RELEASE;

    f3d::interactor::MouseButton nativeButton;
    switch (buttonOrdinal)
    {
      case 0:
        nativeButton = f3d::interactor::MouseButton::LEFT;
        break;
      case 1:
        nativeButton = f3d::interactor::MouseButton::RIGHT;
        break;
      case 2:
        nativeButton = f3d::interactor::MouseButton::MIDDLE;
        break;
      default:
        nativeButton = f3d::interactor::MouseButton::LEFT;
        break;
    }

    GetInteractor(env, self).triggerMouseButton(nativeAction, nativeButton);
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Interactor, triggerMousePosition)(
    JNIEnv* env, jobject self, jdouble xpos, jdouble ypos)
  {
    GetInteractor(env, self).triggerMousePosition(xpos, ypos);
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Interactor, triggerMouseWheel)(
    JNIEnv* env, jobject self, jobject direction)
  {
    jclass directionEnum = env->GetObjectClass(direction);
    jmethodID ordinalMethod = env->GetMethodID(directionEnum, "ordinal", "()I");
    jint directionOrdinal = env->CallIntMethod(direction, ordinalMethod);

    f3d::interactor::WheelDirection nativeDirection;
    switch (directionOrdinal)
    {
      case 0:
        nativeDirection = f3d::interactor::WheelDirection::FORWARD;
        break;
      case 1:
        nativeDirection = f3d::interactor::WheelDirection::BACKWARD;
        break;
      case 2:
        nativeDirection = f3d::interactor::WheelDirection::LEFT;
        break;
      case 3:
        nativeDirection = f3d::interactor::WheelDirection::RIGHT;
        break;
      default:
        nativeDirection = f3d::interactor::WheelDirection::FORWARD;
        break;
    }

    GetInteractor(env, self).triggerMouseWheel(nativeDirection);
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Interactor, triggerKeyboardKey)(
    JNIEnv* env, jobject self, jobject action, jstring keySym)
  {
    jclass actionEnum = env->GetObjectClass(action);
    jmethodID ordinalMethod = env->GetMethodID(actionEnum, "ordinal", "()I");
    jint actionOrdinal = env->CallIntMethod(action, ordinalMethod);

    f3d::interactor::InputAction nativeAction = actionOrdinal == 0
      ? f3d::interactor::InputAction::PRESS
      : f3d::interactor::InputAction::RELEASE;

    const char* keySymStr = env->GetStringUTFChars(keySym, nullptr);
    GetInteractor(env, self).triggerKeyboardKey(nativeAction, keySymStr);
    env->ReleaseStringUTFChars(keySym, keySymStr);

    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Interactor, triggerTextCharacter)(
    JNIEnv* env, jobject self, jint codepoint)
  {
    GetInteractor(env, self).triggerTextCharacter(codepoint);
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Interactor, triggerEventLoop)(
    JNIEnv* env, jobject self, jdouble deltaTime)
  {
    GetInteractor(env, self).triggerEventLoop(deltaTime);
    return self;
  }

  JNIEXPORT jboolean JAVA_BIND(Interactor, playInteraction)(
    JNIEnv* env, jobject self, jstring file, jdouble deltaTime)
  {
    const char* fileStr = env->GetStringUTFChars(file, nullptr);
    bool result = GetInteractor(env, self).playInteraction(fileStr, deltaTime);
    env->ReleaseStringUTFChars(file, fileStr);
    return result;
  }

  JNIEXPORT jboolean JAVA_BIND(Interactor, recordInteraction)(
    JNIEnv* env, jobject self, jstring file)
  {
    const char* fileStr = env->GetStringUTFChars(file, nullptr);
    bool result = GetInteractor(env, self).recordInteraction(fileStr);
    env->ReleaseStringUTFChars(file, fileStr);
    return result;
  }

  JNIEXPORT jobject JAVA_BIND(Interactor, start)(JNIEnv* env, jobject self, jdouble deltaTime)
  {
    GetInteractor(env, self).start(deltaTime);
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Interactor, stop)(JNIEnv* env, jobject self)
  {
    GetInteractor(env, self).stop();
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Interactor, requestRender)(JNIEnv* env, jobject self)
  {
    GetInteractor(env, self).requestRender();
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Interactor, requestStop)(JNIEnv* env, jobject self)
  {
    GetInteractor(env, self).requestStop();
    return self;
  }
}
