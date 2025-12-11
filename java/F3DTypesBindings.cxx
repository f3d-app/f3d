#include "F3DJavaBindings.h"

#include <app_f3d_F3D_Transform2D.h>
#include <app_f3d_F3D_Types_Mesh.h>

#include <types.h>

extern "C"
{
  JNIEXPORT jobject JAVA_BIND(Transform2D, create)(
    JNIEnv* env, jclass, jdoubleArray scale, jdoubleArray translate, jdouble angleDeg)
  {
    if (!scale || !translate)
    {
      return nullptr;
    }

    jsize scaleLen = env->GetArrayLength(scale);
    jsize translateLen = env->GetArrayLength(translate);

    if (scaleLen != 2 || translateLen != 2)
    {
      jclass exceptionClass = env->FindClass("java/lang/IllegalArgumentException");
      env->ThrowNew(exceptionClass, "Scale and translate arrays must have exactly 2 elements");
      return nullptr;
    }

    double* scaleData = env->GetDoubleArrayElements(scale, nullptr);
    double* translateData = env->GetDoubleArrayElements(translate, nullptr);

    f3d::double_array_t<2> cppScale({ scaleData[0], scaleData[1] });
    f3d::double_array_t<2> cppTranslate({ translateData[0], translateData[1] });
    f3d::transform2d_t cppTransform(cppScale, cppTranslate, angleDeg);

    env->ReleaseDoubleArrayElements(scale, scaleData, 0);
    env->ReleaseDoubleArrayElements(translate, translateData, 0);

    jclass transform2DClass = env->FindClass("app/f3d/F3D/Transform2D");
    jmethodID constructor = env->GetMethodID(transform2DClass, "<init>", "()V");
    jobject result = env->NewObject(transform2DClass, constructor);

    jfieldID dataField = env->GetFieldID(transform2DClass, "data", "[D");
    jdoubleArray dataArray = env->NewDoubleArray(9);
    std::vector<double> vec = cppTransform;
    env->SetDoubleArrayRegion(dataArray, 0, 9, vec.data());
    env->SetObjectField(result, dataField, dataArray);

    return result;
  }
  JNIEXPORT jobject JAVA_BIND(Types_00024Mesh, isValid)(JNIEnv* env, jobject self)
  {
    jclass meshClass = env->GetObjectClass(self);

    jfieldID pointsField = env->GetFieldID(meshClass, "points", "[F");
    jfieldID normalsField = env->GetFieldID(meshClass, "normals", "[F");
    jfieldID textureCoordinatesField = env->GetFieldID(meshClass, "textureCoordinates", "[F");
    jfieldID faceSidesField = env->GetFieldID(meshClass, "faceSides", "[I");
    jfieldID faceIndicesField = env->GetFieldID(meshClass, "faceIndices", "[I");

    jfloatArray pointsArray = (jfloatArray)env->GetObjectField(self, pointsField);
    jfloatArray normalsArray = (jfloatArray)env->GetObjectField(self, normalsField);
    jfloatArray textureCoordinatesArray =
      (jfloatArray)env->GetObjectField(self, textureCoordinatesField);
    jintArray faceSidesArray = (jintArray)env->GetObjectField(self, faceSidesField);
    jintArray faceIndicesArray = (jintArray)env->GetObjectField(self, faceIndicesField);

    f3d::mesh_t cppMesh;

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
      if (normalsLen > 0)
      {
        float* normalsData = env->GetFloatArrayElements(normalsArray, nullptr);
        cppMesh.normals.assign(normalsData, normalsData + normalsLen);
        env->ReleaseFloatArrayElements(normalsArray, normalsData, 0);
      }
    }

    if (textureCoordinatesArray)
    {
      jsize texCoordsLen = env->GetArrayLength(textureCoordinatesArray);
      if (texCoordsLen > 0)
      {
        float* texCoordsData = env->GetFloatArrayElements(textureCoordinatesArray, nullptr);
        cppMesh.texture_coordinates.assign(texCoordsData, texCoordsData + texCoordsLen);
        env->ReleaseFloatArrayElements(textureCoordinatesArray, texCoordsData, 0);
      }
    }

    if (faceSidesArray)
    {
      jsize faceSidesLen = env->GetArrayLength(faceSidesArray);
      if (faceSidesLen > 0)
      {
        int* faceSidesData = env->GetIntArrayElements(faceSidesArray, nullptr);
        cppMesh.face_sides.assign(faceSidesData, faceSidesData + faceSidesLen);
        env->ReleaseIntArrayElements(faceSidesArray, faceSidesData, 0);
      }
    }

    if (faceIndicesArray)
    {
      jsize faceIndicesLen = env->GetArrayLength(faceIndicesArray);
      if (faceIndicesLen > 0)
      {
        int* faceIndicesData = env->GetIntArrayElements(faceIndicesArray, nullptr);
        cppMesh.face_indices.assign(faceIndicesData, faceIndicesData + faceIndicesLen);
        env->ReleaseIntArrayElements(faceIndicesArray, faceIndicesData, 0);
      }
    }

    auto [valid, errorMessage] = cppMesh.isValid();

    jclass validationResultClass = env->FindClass("app/f3d/F3D/Types$Mesh$ValidationResult");
    jmethodID constructor =
      env->GetMethodID(validationResultClass, "<init>", "(ZLjava/lang/String;)V");

    jstring jErrorMessage = env->NewStringUTF(errorMessage.c_str());
    jobject result = env->NewObject(validationResultClass, constructor, valid, jErrorMessage);

    env->DeleteLocalRef(jErrorMessage);

    return result;
  }
}
