#include "F3DJavaBindings.h"

#include <app_f3d_F3D_Scene.h>

#include <scene.h>
#include <types.h>

static std::vector<std::string> JavaListToStringVector(JNIEnv* env, jobject list)
{
  std::vector<std::string> vec;

  jclass listClass = env->GetObjectClass(list);
  jmethodID sizeMethod = env->GetMethodID(listClass, "size", "()I");
  jmethodID getMethod = env->GetMethodID(listClass, "get", "(I)Ljava/lang/Object;");

  jint size = env->CallIntMethod(list, sizeMethod);

  for (jint i = 0; i < size; i++)
  {
    if (jstring jstr = static_cast<jstring>(env->CallObjectMethod(list, getMethod, i)))
    {
      const char* str = env->GetStringUTFChars(jstr, nullptr);
      vec.push_back(str);
      env->ReleaseStringUTFChars(jstr, str);
      env->DeleteLocalRef(jstr);
    }
  }

  return vec;
}

static f3d::mesh_t JavaMeshToCppMesh(JNIEnv* env, jobject jmesh)
{
  f3d::mesh_t cppMesh;

  jclass meshClass = env->GetObjectClass(jmesh);

  jfieldID pointsField = env->GetFieldID(meshClass, "points", "[F");
  jfieldID normalsField = env->GetFieldID(meshClass, "normals", "[F");
  jfieldID textureCoordinatesField = env->GetFieldID(meshClass, "textureCoordinates", "[F");
  jfieldID faceSidesField = env->GetFieldID(meshClass, "faceSides", "[I");
  jfieldID faceIndicesField = env->GetFieldID(meshClass, "faceIndices", "[I");

  jfloatArray pointsArray = static_cast<jfloatArray>(env->GetObjectField(jmesh, pointsField));
  jfloatArray normalsArray = static_cast<jfloatArray>(env->GetObjectField(jmesh, normalsField));
  jfloatArray textureCoordinatesArray =
    static_cast<jfloatArray>(env->GetObjectField(jmesh, textureCoordinatesField));
  jintArray faceSidesArray = static_cast<jintArray>(env->GetObjectField(jmesh, faceSidesField));
  jintArray faceIndicesArray = static_cast<jintArray>(env->GetObjectField(jmesh, faceIndicesField));

  if (pointsArray)
  {
    jsize pointsLen = env->GetArrayLength(pointsArray);
    float* pointsData = env->GetFloatArrayElements(pointsArray, nullptr);
    cppMesh.points.assign(pointsData, pointsData + pointsLen);
    env->ReleaseFloatArrayElements(pointsArray, pointsData, 0);
  }

  if (normalsArray)
  {
    jsize normalsLen = env->GetArrayLength(normalsArray);
    float* normalsData = env->GetFloatArrayElements(normalsArray, nullptr);
    cppMesh.normals.assign(normalsData, normalsData + normalsLen);
    env->ReleaseFloatArrayElements(normalsArray, normalsData, 0);
  }

  if (textureCoordinatesArray)
  {
    jsize texCoordsLen = env->GetArrayLength(textureCoordinatesArray);
    float* texCoordsData = env->GetFloatArrayElements(textureCoordinatesArray, nullptr);
    cppMesh.texture_coordinates.assign(texCoordsData, texCoordsData + texCoordsLen);
    env->ReleaseFloatArrayElements(textureCoordinatesArray, texCoordsData, 0);
  }

  if (faceSidesArray)
  {
    jsize faceSidesLen = env->GetArrayLength(faceSidesArray);
    int* faceSidesData = env->GetIntArrayElements(faceSidesArray, nullptr);
    cppMesh.face_sides.assign(faceSidesData, faceSidesData + faceSidesLen);
    env->ReleaseIntArrayElements(faceSidesArray, faceSidesData, 0);
  }

  if (faceIndicesArray)
  {
    jsize faceIndicesLen = env->GetArrayLength(faceIndicesArray);
    int* faceIndicesData = env->GetIntArrayElements(faceIndicesArray, nullptr);
    cppMesh.face_indices.assign(faceIndicesData, faceIndicesData + faceIndicesLen);
    env->ReleaseIntArrayElements(faceIndicesArray, faceIndicesData, 0);
  }

  return cppMesh;
}

static f3d::light_state_t JavaLightStateToCppLightState(JNIEnv* env, jobject jlightState)
{
  f3d::light_state_t cppLightState;

  jclass lightStateClass = env->GetObjectClass(jlightState);

  jfieldID typeField = env->GetFieldID(lightStateClass, "type", "Lapp/f3d/F3D/Types$LightType;");
  jfieldID positionField = env->GetFieldID(lightStateClass, "position", "[D");
  jfieldID colorField = env->GetFieldID(lightStateClass, "color", "[D");
  jfieldID directionField = env->GetFieldID(lightStateClass, "direction", "[D");
  jfieldID positionalLightField = env->GetFieldID(lightStateClass, "positionalLight", "Z");
  jfieldID intensityField = env->GetFieldID(lightStateClass, "intensity", "D");
  jfieldID switchStateField = env->GetFieldID(lightStateClass, "switchState", "Z");

  jobject jtype = env->GetObjectField(jlightState, typeField);
  jclass typeEnumClass = env->GetObjectClass(jtype);
  jmethodID getValueMethod = env->GetMethodID(typeEnumClass, "getValue", "()I");
  jint typeValue = env->CallIntMethod(jtype, getValueMethod);
  cppLightState.type = static_cast<f3d::light_type>(typeValue);

  if (jdoubleArray jposition =
        static_cast<jdoubleArray>(env->GetObjectField(jlightState, positionField)))
  {
    double* posData = env->GetDoubleArrayElements(jposition, nullptr);
    cppLightState.position = { posData[0], posData[1], posData[2] };
    env->ReleaseDoubleArrayElements(jposition, posData, 0);
  }

  if (jdoubleArray jcolor = static_cast<jdoubleArray>(env->GetObjectField(jlightState, colorField)))
  {
    double* colorData = env->GetDoubleArrayElements(jcolor, nullptr);
    cppLightState.color = { colorData[0], colorData[1], colorData[2] };
    env->ReleaseDoubleArrayElements(jcolor, colorData, 0);
  }

  if (jdoubleArray jdirection =
        static_cast<jdoubleArray>(env->GetObjectField(jlightState, directionField)))
  {
    double* dirData = env->GetDoubleArrayElements(jdirection, nullptr);
    cppLightState.direction = { dirData[0], dirData[1], dirData[2] };
    env->ReleaseDoubleArrayElements(jdirection, dirData, 0);
  }

  cppLightState.positionalLight = env->GetBooleanField(jlightState, positionalLightField);
  cppLightState.intensity = env->GetDoubleField(jlightState, intensityField);
  cppLightState.switchState = env->GetBooleanField(jlightState, switchStateField);

  return cppLightState;
}

// Helper function to convert C++ f3d::light_state_t to Java Types.LightState
static jobject CppLightStateToJavaLightState(JNIEnv* env, const f3d::light_state_t& cppLightState)
{
  jclass lightStateClass = env->FindClass("app/f3d/F3D/Types$LightState");
  jmethodID constructor = env->GetMethodID(lightStateClass, "<init>", "()V");
  jobject jlightState = env->NewObject(lightStateClass, constructor);

  jclass typeEnumClass = env->FindClass("app/f3d/F3D/Types$LightType");
  jmethodID fromValueMethod =
    env->GetStaticMethodID(typeEnumClass, "fromValue", "(I)Lapp/f3d/F3D/Types$LightType;");
  jobject jtype = env->CallStaticObjectMethod(
    typeEnumClass, fromValueMethod, static_cast<int>(cppLightState.type));
  jfieldID typeField = env->GetFieldID(lightStateClass, "type", "Lapp/f3d/F3D/Types$LightType;");
  env->SetObjectField(jlightState, typeField, jtype);

  jdoubleArray jposition = env->NewDoubleArray(3);
  double posData[] = { cppLightState.position[0], cppLightState.position[1],
    cppLightState.position[2] };
  env->SetDoubleArrayRegion(jposition, 0, 3, posData);
  jfieldID positionField = env->GetFieldID(lightStateClass, "position", "[D");
  env->SetObjectField(jlightState, positionField, jposition);

  jdoubleArray jcolor = env->NewDoubleArray(3);
  double colorData[] = { cppLightState.color[0], cppLightState.color[1], cppLightState.color[2] };
  env->SetDoubleArrayRegion(jcolor, 0, 3, colorData);
  jfieldID colorField = env->GetFieldID(lightStateClass, "color", "[D");
  env->SetObjectField(jlightState, colorField, jcolor);

  jdoubleArray jdirection = env->NewDoubleArray(3);
  double dirData[] = { cppLightState.direction[0], cppLightState.direction[1],
    cppLightState.direction[2] };
  env->SetDoubleArrayRegion(jdirection, 0, 3, dirData);
  jfieldID directionField = env->GetFieldID(lightStateClass, "direction", "[D");
  env->SetObjectField(jlightState, directionField, jdirection);

  jfieldID positionalLightField = env->GetFieldID(lightStateClass, "positionalLight", "Z");
  env->SetBooleanField(jlightState, positionalLightField, cppLightState.positionalLight);

  jfieldID intensityField = env->GetFieldID(lightStateClass, "intensity", "D");
  env->SetDoubleField(jlightState, intensityField, cppLightState.intensity);

  jfieldID switchStateField = env->GetFieldID(lightStateClass, "switchState", "Z");
  env->SetBooleanField(jlightState, switchStateField, cppLightState.switchState);

  return jlightState;
}

extern "C"
{
  JNIEXPORT jobject JAVA_BIND(Scene, add)(JNIEnv* env, jobject self, jstring path)
  {
    if (!path)
    {
      return self;
    }

    const char* str = env->GetStringUTFChars(path, nullptr);
    try
    {
      GetEngine(env, self)->getScene().add(str);
    }
    catch (const std::exception& e)
    {
      env->ReleaseStringUTFChars(path, str);
      jclass exceptionClass = env->FindClass("java/lang/RuntimeException");
      env->ThrowNew(exceptionClass, e.what());
      return nullptr;
    }
    env->ReleaseStringUTFChars(path, str);
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Scene, addAll)(JNIEnv* env, jobject self, jobject paths)
  {
    if (!paths)
    {
      return self;
    }

    std::vector<std::string> vec = JavaListToStringVector(env, paths);
    try
    {
      GetEngine(env, self)->getScene().add(vec);
    }
    catch (const std::exception& e)
    {
      jclass exceptionClass = env->FindClass("java/lang/RuntimeException");
      env->ThrowNew(exceptionClass, e.what());
      return nullptr;
    }
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Scene, addMesh)(JNIEnv* env, jobject self, jobject mesh)
  {
    if (!mesh)
    {
      return self;
    }

    f3d::mesh_t cppMesh = JavaMeshToCppMesh(env, mesh);
    try
    {
      GetEngine(env, self)->getScene().add(cppMesh);
    }
    catch (const std::exception& e)
    {
      jclass exceptionClass = env->FindClass("java/lang/RuntimeException");
      env->ThrowNew(exceptionClass, e.what());
      return nullptr;
    }
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Scene, clear)(JNIEnv* env, jobject self)
  {
    GetEngine(env, self)->getScene().clear();
    return self;
  }

  JNIEXPORT jint JAVA_BIND(Scene, addLight)(JNIEnv* env, jobject self, jobject lightState)
  {
    if (!lightState)
    {
      return -1;
    }

    f3d::light_state_t cppLightState = JavaLightStateToCppLightState(env, lightState);
    try
    {
      return GetEngine(env, self)->getScene().addLight(cppLightState);
    }
    catch (const std::exception& e)
    {
      jclass exceptionClass = env->FindClass("java/lang/RuntimeException");
      env->ThrowNew(exceptionClass, e.what());
      return -1;
    }
  }

  JNIEXPORT jint JAVA_BIND(Scene, getLightCount)(JNIEnv* env, jobject self)
  {
    return GetEngine(env, self)->getScene().getLightCount();
  }

  JNIEXPORT jobject JAVA_BIND(Scene, getLight)(JNIEnv* env, jobject self, jint index)
  {
    try
    {
      f3d::light_state_t cppLightState = GetEngine(env, self)->getScene().getLight(index);
      return CppLightStateToJavaLightState(env, cppLightState);
    }
    catch (const std::exception& e)
    {
      jclass exceptionClass = env->FindClass("java/lang/RuntimeException");
      env->ThrowNew(exceptionClass, e.what());
      return nullptr;
    }
  }

  JNIEXPORT jobject JAVA_BIND(Scene, updateLight)(
    JNIEnv* env, jobject self, jint index, jobject lightState)
  {
    if (!lightState)
    {
      return self;
    }

    f3d::light_state_t cppLightState = JavaLightStateToCppLightState(env, lightState);
    try
    {
      GetEngine(env, self)->getScene().updateLight(index, cppLightState);
    }
    catch (const std::exception& e)
    {
      jclass exceptionClass = env->FindClass("java/lang/RuntimeException");
      env->ThrowNew(exceptionClass, e.what());
      return nullptr;
    }
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Scene, removeLight)(JNIEnv* env, jobject self, jint index)
  {
    try
    {
      GetEngine(env, self)->getScene().removeLight(index);
    }
    catch (const std::exception& e)
    {
      jclass exceptionClass = env->FindClass("java/lang/RuntimeException");
      env->ThrowNew(exceptionClass, e.what());
      return nullptr;
    }
    return self;
  }

  JNIEXPORT jobject JAVA_BIND(Scene, removeAllLights)(JNIEnv* env, jobject self)
  {
    GetEngine(env, self)->getScene().removeAllLights();
    return self;
  }

  JNIEXPORT jboolean JAVA_BIND(Scene, supports)(JNIEnv* env, jobject self, jstring filePath)
  {
    if (!filePath)
    {
      return false;
    }

    const char* str = env->GetStringUTFChars(filePath, nullptr);
    bool result = GetEngine(env, self)->getScene().supports(str);
    env->ReleaseStringUTFChars(filePath, str);
    return result;
  }

  JNIEXPORT jobject JAVA_BIND(Scene, loadAnimationTime)(
    JNIEnv* env, jobject self, jdouble timeValue)
  {
    GetEngine(env, self)->getScene().loadAnimationTime(timeValue);
    return self;
  }

  JNIEXPORT jdoubleArray JAVA_BIND(Scene, animationTimeRange)(JNIEnv* env, jobject self)
  {
    auto [minTime, maxTime] = GetEngine(env, self)->getScene().animationTimeRange();

    jdoubleArray result = env->NewDoubleArray(2);
    double timeRange[] = { minTime, maxTime };
    env->SetDoubleArrayRegion(result, 0, 2, timeRange);

    return result;
  }

  JNIEXPORT jint JAVA_BIND(Scene, availableAnimations)(JNIEnv* env, jobject self)
  {
    return GetEngine(env, self)->getScene().availableAnimations();
  }

  JNIEXPORT jstring JAVA_BIND(Scene, getAnimationName)(JNIEnv* env, jobject self, jint indices)
  {
    return env->NewStringUTF(GetEngine(env, self)->getScene().getAnimationName(indices).c_str());
  }

  JNIEXPORT jobject JAVA_BIND(Scene, getAnimationNames)(JNIEnv* env, jobject self)
  {
    return CreateStringList(env, GetEngine(env, self)->getScene().getAnimationNames());
  }
}
