#include "vtkF3DUSDImporter.h"

#include "vtkF3DFaceVaryingPointDispatcher.h"

#include <vtkActor.h>
#include <vtkCapsuleSource.h>
#include <vtkConeSource.h>
#include <vtkCubeSource.h>
#include <vtkCylinderSource.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkImageAppendComponents.h>
#include <vtkImageData.h>
#include <vtkImageExtractComponents.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Factory.h>
#include <vtkImageResize.h>
#include <vtkInformation.h>
#include <vtkInformationStringKey.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkPolyDataTangents.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkTexture.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkTriangleFilter.h>

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#if __GNUC__ >= 9
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#endif
#elif defined(_MSC_VER)
#pragma warning(push, 0)
#endif
#include <pxr/usd/ar/asset.h>
#include <pxr/usd/ar/resolver.h>
#include <pxr/usd/usd/modelAPI.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/capsule.h>
#include <pxr/usd/usdGeom/cone.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/cylinder.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usdGeom/pointInstancer.h>
#include <pxr/usd/usdGeom/primvarsAPI.h>
#include <pxr/usd/usdGeom/scope.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/tokens.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdShade/material.h>
#include <pxr/usd/usdShade/materialBindingAPI.h>
#include <pxr/usd/usdSkel/bakeSkinning.h>
#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop, 0)
#endif

class vtkF3DUSDImporter::vtkInternals
{
public:
  void ReadScene(const std::string& filePath)
  {
    // in case of failure, you may want to set PXR_PLUGINPATH_NAME to the lib/usd path
    if (!this->Stage)
    {
      this->Stage = pxr::UsdStage::Open(filePath);

      if (this->Stage)
      {
        // TODO: USD bake skinning is not performant
        // We need to read joints and do the skinning in the shader
        // See https://github.com/f3d-app/f3d/issues/1076
        pxr::UsdSkelBakeSkinning(this->Stage->Traverse());
      }
    }
  }

  template<typename T>
  std::pair<pxr::UsdShadeShader, pxr::TfToken> GetConnectedShaderPrim(const T& port)
  {
    if (port)
    {
      pxr::UsdShadeConnectableAPI api;
      pxr::UsdShadeAttributeType type;
      pxr::TfToken token;
      port.GetConnectedSource(&api, &token, &type);

      if (api && api.GetPrim().IsA<pxr::UsdShadeShader>())
      {
        return { pxr::UsdShadeShader(api.GetPrim()), token };
      }
    }
    return { pxr::UsdShadeShader(), pxr::TfToken() };
  }

  vtkSmartPointer<vtkMatrix4x4> ConvertMatrix(const pxr::GfMatrix4d& uMatrix)
  {
    vtkNew<vtkMatrix4x4> mat;

    std::copy(uMatrix.data(), uMatrix.data() + 16, mat->GetData());
    mat->Transpose();

    return mat;
  }

  vtkSmartPointer<vtkMatrix4x4> GetLocalTransform(
    const pxr::UsdGeomImageable& node, pxr::UsdTimeCode timeCode)
  {
    // get xform
    return this->ConvertMatrix(node.ComputeLocalToWorldTransform(timeCode));
  }

  void AddActor(vtkRenderer* renderer, const pxr::SdfPath& path, const pxr::UsdGeomGprim& geomPrim,
    const pxr::UsdPrim& prim, vtkMatrix4x4* mat, vtkPolyData* polydata)
  {
    pxr::SdfPath actorPath = path.AppendChild(pxr::TfToken(prim.GetName()));

    auto& actor = this->ActorMap[actorPath.GetAsString()];
    bool actorAlreadyExists = (actor != nullptr);

    if (!actorAlreadyExists)
    {
      actor = vtkSmartPointer<vtkActor>::New();

      // get associated material/shader
      pxr::UsdShadeMaterial material =
        pxr::UsdShadeMaterialBindingAPI(prim).ComputeBoundMaterial(pxr::UsdShadeTokens->preview);

      if (material)
      {
        auto [shaderPrim, token] = this->GetConnectedShaderPrim(material.GetSurfaceOutput());

        auto prop = this->GetVTKProperty(shaderPrim);

        if (prop)
        {
          actor->SetProperty(prop);

          // enable translucent flag if required
          vtkTexture* baseColor = prop->GetTexture("albedoTex");
          if (prop->GetOpacity() < 0.99 ||
            (baseColor && baseColor->GetInput()->GetNumberOfScalarComponents() == 4))
          {
            actor->ForceTranslucentOn();
          }

          // activate correct UV set if relevant
          vtkInformation* info = prop->GetInformation();
          const char* uvName = info->Get(vtkF3DUSDImporter::TCOORDS_NAME());

          if (uvName && uvName[0] != 0)
          {
            polydata->GetPointData()->SetActiveAttribute(uvName, vtkDataSetAttributes::TCOORDS);
          }
        }
      }
      else
      {
        // if there is no material, fallback on display color
        pxr::UsdAttribute displayColorAttr = geomPrim.GetDisplayColorAttr();

        vtkNew<vtkProperty> prop;
        prop->SetInterpolationToPBR();

        pxr::VtArray<pxr::GfVec3f> color;
        if (displayColorAttr.Get(&color) && color.size() == 1)
        {
          prop->SetColor(color[0][0], color[0][1], color[0][2]);
        }

        actor->SetProperty(prop);
      }

      // backface culling
      pxr::UsdAttribute doubleSidedAttr = geomPrim.GetDoubleSidedAttr();

      bool doubleSided;
      if (doubleSidedAttr.Get(&doubleSided) && !doubleSided)
      {
        pxr::UsdAttribute orientationAttr = geomPrim.GetOrientationAttr();

        pxr::TfToken orientation;
        if (orientationAttr && orientationAttr.Get(&orientation))
        {
          if (orientation == pxr::UsdGeomTokens->rightHanded)
          {
            actor->GetProperty()->BackfaceCullingOn();
          }
          else
          {
            actor->GetProperty()->FrontfaceCullingOn();
          }
        }
      }

      renderer->AddActor(actor);
    }

    // set mapper
    vtkNew<vtkPolyDataMapper> mapper;

    if (actor->GetProperty()->GetTexture("normalTex"))
    {
      vtkNew<vtkTriangleFilter> triangulate;
      triangulate->SetInputData(polydata);

      vtkNew<vtkPolyDataNormals> normals;
      normals->SetInputConnection(triangulate->GetOutputPort());

      vtkNew<vtkPolyDataTangents> tangents;
      tangents->SetInputConnection(normals->GetOutputPort());
      tangents->Update();
      mapper->SetInputData(tangents->GetOutput());
    }
    else
    {
      mapper->SetInputData(polydata);
    }

    if (!this->HasTimeCode())
    {
      mapper->StaticOn();
    }

    actor->SetMapper(mapper);
    actor->SetUserMatrix(mat);
  }

  void ImportNode(vtkRenderer* renderer, const pxr::UsdPrim& node, const pxr::SdfPath& path,
    vtkMatrix4x4* currentMatrix)
  {
    pxr::UsdTimeCode timeCode = this->CurrentTime * this->Stage->GetTimeCodesPerSecond();

    // simple range-for iteration
    for (pxr::UsdPrim prim : pxr::UsdPrimSiblingRange(node.GetAllChildren()))
    {
      if (prim.IsA<pxr::UsdGeomImageable>())
      {
        pxr::UsdGeomImageable imageable = pxr::UsdGeomImageable(prim);

        pxr::TfToken visibility;
        pxr::UsdAttribute visAttr = imageable.GetVisibilityAttr();
        if (visAttr && visAttr.HasAuthoredValue() && visAttr.Get(&visibility, timeCode) &&
          visibility == pxr::UsdGeomTokens->invisible)
        {
          // not visible, skip
          continue;
        }

        pxr::TfToken purpose;
        pxr::UsdAttribute purpAttr = imageable.GetPurposeAttr();
        if (purpAttr && purpAttr.HasAuthoredValue() && purpAttr.Get(&purpose, timeCode) &&
          (purpose == pxr::UsdGeomTokens->proxy || purpose == pxr::UsdGeomTokens->guide))
        {
          // proxy, skip
          continue;
        }
      }

      if (prim.IsInstance())
      {
        pxr::UsdGeomXform xform = pxr::UsdGeomXform(prim);

        auto mat = this->GetLocalTransform(xform, timeCode);
        vtkMatrix4x4::Multiply4x4(currentMatrix, mat, mat);

        this->ImportNode(renderer, prim.GetPrototype(), path.AppendChild(prim.GetName()), mat);
      }
      else if (prim.IsA<pxr::UsdGeomPointInstancer>())
      {
        pxr::UsdGeomPointInstancer glyphs = pxr::UsdGeomPointInstancer(prim);

        // TODO: Ideally, we should use the 3D glyph mapper, but it's left for future work
        // See https://github.com/f3d-app/f3d/issues/1075
        pxr::VtMatrix4dArray xforms;

        if (glyphs.ComputeInstanceTransformsAtTime(&xforms, timeCode, timeCode))
        {
          int i = 0;
          for (const pxr::GfMatrix4d& currInstMatrix : xforms)
          {
            auto mat = this->ConvertMatrix(currInstMatrix);
            vtkMatrix4x4::Multiply4x4(currentMatrix, mat, mat);

            pxr::TfToken tok(std::string("instance_") + std::to_string(i++));

            this->ImportNode(
              renderer, prim, path.AppendChild(prim.GetName()).AppendChild(tok), mat);
          }
        }
      }
      else if (prim.IsA<pxr::UsdGeomGprim>())
      {
        pxr::UsdGeomGprim geomPrim = pxr::UsdGeomGprim(prim);

        vtkSmartPointer<vtkPolyData> polydata;

        if (prim.IsA<pxr::UsdGeomMesh>())
        {
          pxr::UsdGeomMesh meshPrim = pxr::UsdGeomMesh(prim);

          vtkSmartPointer<vtkPolyData>& mappedPolydata =
            this->MeshMap[meshPrim.GetPath().GetAsString()];
          bool meshAlreadyExists = (mappedPolydata != nullptr);

          // attributes
          pxr::UsdAttribute normalsAttr = meshPrim.GetNormalsAttr();
          pxr::UsdAttribute pointsAttr = meshPrim.GetPointsAttr();
          pxr::UsdAttribute facesCountAttr = meshPrim.GetFaceVertexCountsAttr();
          pxr::UsdAttribute facesIndicesAttr = meshPrim.GetFaceVertexIndicesAttr();

          std::vector<pxr::UsdGeomPrimvar> primVars =
            pxr::UsdGeomPrimvarsAPI(meshPrim).GetPrimvars();

          auto TimeVarying = [](const auto& a) { return a.ValueMightBeTimeVarying(); };

          bool animatedAttribute = std::any_of(primVars.cbegin(), primVars.cend(), TimeVarying);
          animatedAttribute = animatedAttribute || TimeVarying(pointsAttr);
          animatedAttribute = animatedAttribute || TimeVarying(normalsAttr);
          animatedAttribute = animatedAttribute || TimeVarying(facesCountAttr);
          animatedAttribute = animatedAttribute || TimeVarying(facesIndicesAttr);

          // Check if the mesh has to be rebuilt
          if (!meshAlreadyExists || animatedAttribute)
          {
            vtkNew<vtkPolyData> newPolyData;

            // normals
            pxr::VtArray<pxr::GfVec3f> normals;
            normalsAttr.Get(&normals, timeCode);

            if (normals.size() > 0)
            {
              vtkNew<vtkFloatArray> vNormals;
              vNormals->SetName("Normals");
              vNormals->SetNumberOfComponents(3);
              vNormals->Allocate(normals.size());

              for (const pxr::GfVec3f& n : normals)
              {
                vNormals->InsertNextTuple3(n[0], n[1], n[2]);
              }

              vtkInformation* info = vNormals->GetInformation();
              info->Set(vtkF3DFaceVaryingPointDispatcher::INTERPOLATION_TYPE(),
                meshPrim.GetNormalsInterpolation() == pxr::UsdGeomTokens->faceVarying ? 1 : 0);

              newPolyData->GetPointData()->SetNormals(vNormals);
            }

            // texture coordinates
            bool firstArray = true;
            for (const pxr::UsdGeomPrimvar& primVar : primVars)
            {
              if (primVar.GetTypeName() == "texCoord2f[]" || primVar.GetTypeName() == "float2[]")
              {
                pxr::VtArray<pxr::GfVec2f> uvs;
                primVar.Get(&uvs, timeCode);

                if (uvs.size() > 0)
                {
                  std::string name = primVar.GetPrimvarName();

                  vtkNew<vtkFloatArray> texCoords;
                  texCoords->SetName(name.c_str());
                  texCoords->SetNumberOfComponents(2);

                  if (primVar.IsIndexed())
                  {
                    pxr::UsdAttribute indicesAttr = primVar.GetIndicesAttr();

                    pxr::VtArray<int> indices;
                    if (indicesAttr.Get(&indices) && indices.size() > 0)
                    {
                      texCoords->Allocate(indices.size());

                      for (int index : indices)
                      {
                        const pxr::GfVec2f& uv = uvs[index];
                        texCoords->InsertNextTuple2(uv[0], uv[1]);
                      }
                    }
                  }
                  else
                  {
                    texCoords->Allocate(uvs.size());

                    for (const pxr::GfVec2f& uv : uvs)
                    {
                      texCoords->InsertNextTuple2(uv[0], uv[1]);
                    }
                  }

                  vtkInformation* info = texCoords->GetInformation();
                  info->Set(vtkF3DFaceVaryingPointDispatcher::INTERPOLATION_TYPE(),
                    primVar.GetInterpolation() == pxr::UsdGeomTokens->faceVarying ? 1 : 0);

                  // the size of the array can be larger than the number of points if the attribute
                  // interpolation is face-varying.
                  // It will be normalized by the vtkF3DFaceVaryingPointDispatcher later
                  newPolyData->GetPointData()->AddArray(texCoords);

                  if (firstArray)
                  {
                    // sometimes we are enable to fetch the array name to use for texture mapping
                    // so we fallback to the first UV set added
                    // see https://github.com/f3d-app/f3d/issues/1184
                    firstArray = false;
                    newPolyData->GetPointData()->SetTCoords(texCoords);
                  }
                }
              }
            }

            // points
            pxr::VtArray<pxr::GfVec3f> positions;
            pointsAttr.Get(&positions, timeCode);

            vtkNew<vtkPoints> points;
            points->Allocate(positions.size());
            for (const pxr::GfVec3f& p : positions)
            {
              points->InsertNextPoint(p[0], p[1], p[2]);
            }

            newPolyData->SetPoints(points);

            // faces
            pxr::VtArray<int> counts;
            facesCountAttr.Get(&counts, timeCode);

            pxr::VtArray<int> indices;
            facesIndicesAttr.Get(&indices, timeCode);

            // add polygons
            vtkNew<vtkCellArray> cells;
            auto currentCellIt = indices.cbegin();
            std::vector<vtkIdType> indexArr;
            for (int c : counts)
            {
              indexArr.clear();
              indexArr.insert(indexArr.begin(), currentCellIt, std::next(currentCellIt, c));
              cells->InsertNextCell(c, indexArr.data());
              std::advance(currentCellIt, c);
            }

            newPolyData->SetPolys(cells);

            vtkNew<vtkF3DFaceVaryingPointDispatcher> faceVaryingFilter;
            faceVaryingFilter->SetInputData(newPolyData);
            faceVaryingFilter->Update();

            mappedPolydata = faceVaryingFilter->GetOutput();
          }

          polydata = mappedPolydata;
        }
        else if (prim.IsA<pxr::UsdGeomSphere>())
        {
          pxr::UsdGeomSphere spherePrim = pxr::UsdGeomSphere(prim);

          vtkNew<vtkSphereSource> sphere;
          sphere->SetThetaResolution(20);
          sphere->SetPhiResolution(20);

          double radius;
          if (spherePrim.GetRadiusAttr().Get(&radius))
          {
            sphere->SetRadius(radius);
          }

          sphere->Update();
          polydata = sphere->GetOutput();
        }
        else if (prim.IsA<pxr::UsdGeomCube>())
        {
          pxr::UsdGeomCube cubePrim = pxr::UsdGeomCube(prim);

          vtkNew<vtkCubeSource> cube;

          double length;
          if (cubePrim.GetSizeAttr().Get(&length))
          {
            cube->SetXLength(length);
            cube->SetYLength(length);
            cube->SetZLength(length);
          }

          cube->Update();
          polydata = cube->GetOutput();
        }
        else if (prim.IsA<pxr::UsdGeomCapsule>())
        {
          pxr::UsdGeomCapsule capsulePrim = pxr::UsdGeomCapsule(prim);

          // See https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10531
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 0)
          vtkNew<vtkCylinderSource> capsule;
          capsule->CapsuleCapOn();

          double height;
          if (capsulePrim.GetHeightAttr().Get(&height))
          {
            capsule->SetHeight(height);
          }
#else
          vtkNew<vtkCapsuleSource> capsule;

          double height;
          if (capsulePrim.GetHeightAttr().Get(&height))
          {
            capsule->SetCylinderLength(height);
          }
#endif

          double radius;
          if (capsulePrim.GetRadiusAttr().Get(&radius))
          {
            capsule->SetRadius(radius);
          }

          // In VTK, the capsule is aligned with the Y axis
          // In USD, the default is aligned with Z, but can be modified
          // Let's rotate it if needed
          vtkNew<vtkTransformFilter> transform;
          vtkNew<vtkTransform> t;
          transform->SetTransform(t);

          pxr::TfToken axisToken(pxr::UsdGeomTokens->z);
          capsulePrim.GetAxisAttr().Get(&axisToken);

          if (axisToken == pxr::UsdGeomTokens->x)
          {
            t->RotateZ(90.0);
          }
          else if (axisToken == pxr::UsdGeomTokens->z)
          {
            t->RotateX(90.0);
          }

          transform->SetInputConnection(capsule->GetOutputPort());
          transform->Update();
          polydata = vtkPolyData::SafeDownCast(transform->GetOutput());
        }
        else if (prim.IsA<pxr::UsdGeomCylinder>())
        {
          pxr::UsdGeomCylinder cylinderPrim = pxr::UsdGeomCylinder(prim);
          vtkNew<vtkCylinderSource> cylinder;
          cylinder->SetResolution(20);

          double height;
          if (cylinderPrim.GetHeightAttr().Get(&height))
          {
            cylinder->SetHeight(height);
          }

          double radius;
          if (cylinderPrim.GetRadiusAttr().Get(&radius))
          {
            cylinder->SetRadius(radius);
          }

          // In VTK, the cylinder is aligned with the Y axis
          // In USD, the default is aligned with Z, but can be modified
          // Let's rotate it if needed
          vtkNew<vtkTransformFilter> transform;
          vtkNew<vtkTransform> t;
          transform->SetTransform(t);

          pxr::TfToken axisToken(pxr::UsdGeomTokens->z);
          cylinderPrim.GetAxisAttr().Get(&axisToken);

          if (axisToken == pxr::TfToken(pxr::UsdGeomTokens->x))
          {
            t->RotateZ(90.0);
          }
          else if (axisToken == pxr::TfToken(pxr::UsdGeomTokens->z))
          {
            t->RotateX(90.0);
          }

          transform->SetInputConnection(cylinder->GetOutputPort());
          transform->Update();
          polydata = vtkPolyData::SafeDownCast(transform->GetOutput());
        }
        else if (prim.IsA<pxr::UsdGeomCone>())
        {
          pxr::UsdGeomCone conePrim = pxr::UsdGeomCone(prim);
          vtkNew<vtkConeSource> cone;
          cone->SetResolution(20);

          double height;
          if (conePrim.GetHeightAttr().Get(&height))
          {
            cone->SetHeight(height);
          }

          double radius;
          if (conePrim.GetRadiusAttr().Get(&radius))
          {
            cone->SetRadius(radius);
          }

          // In VTK, the cylinder is aligned with the X axis
          // In USD, the default is aligned with Z, but can be modified
          // Let's rotate it if needed
          vtkNew<vtkTransformFilter> transform;
          vtkNew<vtkTransform> t;
          transform->SetTransform(t);

          pxr::TfToken axisToken(pxr::UsdGeomTokens->z);
          conePrim.GetAxisAttr().Get(&axisToken);

          if (axisToken == pxr::TfToken(pxr::UsdGeomTokens->y))
          {
            t->RotateZ(90.0);
          }
          else if (axisToken == pxr::TfToken(pxr::UsdGeomTokens->z))
          {
            t->RotateY(90.0);
          }

          transform->SetInputConnection(cone->GetOutputPort());
          transform->Update();
          polydata = vtkPolyData::SafeDownCast(transform->GetOutput());
        }

        // create actors

        // get xform
        auto mat = this->GetLocalTransform(geomPrim, timeCode);
        vtkMatrix4x4::Multiply4x4(currentMatrix, mat, mat);

        std::vector<pxr::UsdGeomSubset> subsets = pxr::UsdGeomSubset::GetGeomSubsets(geomPrim);

        if (subsets.empty())
        {
          this->AddActor(renderer, path, geomPrim, prim, mat, polydata);
        }
        else
        {
          // split subsets
          for (const pxr::UsdGeomSubset& subset : subsets)
          {
            pxr::UsdAttribute indicesAttr = subset.GetIndicesAttr();

            pxr::VtArray<int> indices;
            indicesAttr.Get(&indices, timeCode);

            vtkNew<vtkPolyData> polydataSubset;
            polydataSubset->SetPoints(polydata->GetPoints());
            polydataSubset->GetPointData()->ShallowCopy(polydata->GetPointData());

            vtkCellArray* mainPolys = polydata->GetPolys();

            // add polygons
            vtkNew<vtkCellArray> cells;
            for (int cellId : indices)
            {
              vtkIdType cellSize;
              const vtkIdType* cellPoints;
              mainPolys->GetCellAtId(cellId, cellSize, cellPoints);
              cells->InsertNextCell(cellSize, cellPoints);
            }

            polydataSubset->SetPolys(cells);

            this->AddActor(renderer, path.AppendChild(pxr::TfToken(prim.GetName())), geomPrim,
              subset.GetPrim(), mat, polydataSubset);
          }
        }
      }
      else
      {
        // just traverse the node
        this->ImportNode(renderer, prim, path.AppendChild(prim.GetName()), currentMatrix);
      }
    }
  }

  void ImportRoot(vtkRenderer* renderer)
  {
    if (!this->Stage)
    {
      vtkErrorWithObjectMacro(renderer, << "Stage failed to open");
      return;
    }

    vtkNew<vtkMatrix4x4> rootTransform;

    pxr::TfToken up = pxr::UsdGeomGetStageUpAxis(this->Stage);

    if (up == pxr::UsdGeomTokens->z)
    {
      vtkWarningWithObjectMacro(
        renderer, << "This USD file up axis is +Z, internally rotate it to align with +Y axis");
      rootTransform->Zero();
      rootTransform->SetElement(0, 0, 1.0);
      rootTransform->SetElement(1, 2, 1.0);
      rootTransform->SetElement(2, 1, -1.0);
      rootTransform->SetElement(3, 3, 1.0);
    }

    this->ImportNode(renderer, this->Stage->GetPseudoRoot(), pxr::SdfPath("/"), rootTransform);
  }

  vtkSmartPointer<vtkImageData> CombineORMImage(
    vtkImageData* occlusionImage, vtkImageData* roughnessImage, vtkImageData* metallicImage)
  {
    if (!occlusionImage && !roughnessImage && !metallicImage)
    {
      return nullptr;
    }

    int maxWidth = 0;
    int maxHeight = 0;

    if (occlusionImage)
    {
      int* size = occlusionImage->GetDimensions();

      maxWidth = std::max(maxWidth, size[0]);
      maxHeight = std::max(maxHeight, size[1]);
    }

    if (roughnessImage)
    {
      int* size = roughnessImage->GetDimensions();

      maxWidth = std::max(maxWidth, size[0]);
      maxHeight = std::max(maxHeight, size[1]);
    }

    if (metallicImage)
    {
      int* size = metallicImage->GetDimensions();

      maxWidth = std::max(maxWidth, size[0]);
      maxHeight = std::max(maxHeight, size[1]);
    }

    auto ResizeAndExtractChannel = [&](vtkImageData* img) -> vtkSmartPointer<vtkImageData>
    {
      if (!img)
      {
        vtkNew<vtkImageData> emptyImage;
        emptyImage->SetDimensions(maxWidth, maxHeight, 1);
        emptyImage->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
        unsigned char* data = static_cast<unsigned char*>(emptyImage->GetScalarPointer());
        std::fill(data, data + maxWidth * maxHeight, 0xff);
        return emptyImage;
      }

      // The image should already be a single channel image, but just in case
      vtkNew<vtkImageExtractComponents> extract;
      extract->SetInputData(img);
      extract->SetComponents(0);

      vtkNew<vtkImageResize> resize;
      resize->SetInputConnection(extract->GetOutputPort());
      resize->SetOutputDimensions(maxWidth, maxHeight, 1);
      resize->Update();
      return resize->GetOutput();
    };

    vtkNew<vtkImageAppendComponents> appendChannels;
    appendChannels->SetInputData(ResizeAndExtractChannel(occlusionImage));
    appendChannels->AddInputData(ResizeAndExtractChannel(roughnessImage));
    appendChannels->AddInputData(ResizeAndExtractChannel(metallicImage));
    appendChannels->Update();

    return appendChannels->GetOutput();
  }

  vtkSmartPointer<vtkImageData> CombineColorOpacityImage(
    vtkImageData* colorImage, vtkImageData* opacityImage)
  {
    if (!opacityImage)
    {
      return colorImage;
    }

    vtkNew<vtkImageAppendComponents> appendChannels;
    appendChannels->SetInputData(colorImage);
    appendChannels->AddInputData(opacityImage);
    appendChannels->Update();

    return appendChannels->GetOutput();
  }

  // returns the image and the texture coordinate name
  vtkSmartPointer<vtkImageData> GetVTKTexture(
    const pxr::UsdShadeShader& samplerPrim, const pxr::TfToken& token)
  {
    if (!samplerPrim)
    {
      return nullptr;
    }

    pxr::TfToken idToken;
    bool defined = samplerPrim.GetIdAttr().Get(&idToken);
    if (!defined || idToken != pxr::TfToken("UsdUVTexture"))
    {
      // only UsdUVTexture supported for now
      return nullptr;
    }

    // get array name
    auto [uvset, uvtoken] = this->GetConnectedShaderPrim(samplerPrim.GetInput(pxr::TfToken("st")));
    std::string name;

    if (uvset)
    {
      pxr::UsdShadeInput arrayName = uvset.GetInput(pxr::TfToken("varname"));

      if (arrayName)
      {
        if (arrayName.GetTypeName() == "token")
        {
          pxr::TfToken tokenName;
          if (arrayName.Get(&tokenName))
          {
            name = tokenName;
          }
        }
        else if (arrayName.GetTypeName() == "string")
        {
          arrayName.Get(&name);
        }
      }
    }

    auto& tex = this->TextureMap[samplerPrim.GetPath().GetAsString()];

    if (tex == nullptr)
    {
      pxr::SdfAssetPath path;
      pxr::UsdShadeInput fileInput = samplerPrim.GetInput(pxr::TfToken("file"));
      if (fileInput && fileInput.Get(&path))
      {
        vtkSmartPointer<vtkImageReader2> reader;

// CreateImageReader2FromExtension needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/8211
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20210729)
        const std::string& assetPath = path.GetAssetPath();
        std::string ext = assetPath.substr(assetPath.find_last_of('.'));
        reader.TakeReference(vtkImageReader2Factory::CreateImageReader2FromExtension(ext.c_str()));
#endif

        if (!reader)
        {
          // cannot read the image file
          return nullptr;
        }

        const std::string& resolvedPath = path.GetResolvedPath();
        auto asset = pxr::ArGetResolver().OpenAsset(pxr::ArResolvedPath(resolvedPath));

        if (!asset)
        {
          // cannot get USD asset
          return nullptr;
        }

        auto buffer = asset->GetBuffer();

        if (!buffer)
        {
          // buffer invalid
          return nullptr;
        }

        reader->SetMemoryBuffer(buffer.get());
        reader->SetMemoryBufferLength(asset->GetSize());
        reader->Update();

        tex = reader->GetOutput();
      }
      else
      {
        return nullptr;
      }
    }

    tex->GetInformation()->Set(vtkF3DUSDImporter::TCOORDS_NAME(), name);

    // extract component based on token
    vtkNew<vtkImageExtractComponents> extract;
    extract->SetInputData(tex);

    const std::string& channels = token.GetString();

    if (channels == "rgb")
    {
      extract->SetComponents(0, 1, 2);
    }
    else if (channels == "r")
    {
      extract->SetComponents(0);
    }
    else if (channels == "g")
    {
      extract->SetComponents(1);
    }
    else if (channels == "b")
    {
      extract->SetComponents(2);
    }
    else if (channels == "a")
    {
      extract->SetComponents(3);
    }
    else
    {
      // just return the image as is
      return tex;
    }

    extract->Update();

    extract->GetOutput()->GetInformation()->Copy(tex->GetInformation());

    return extract->GetOutput();
  }

  vtkSmartPointer<vtkProperty> GetVTKProperty(const pxr::UsdShadeShader& shaderPrim)
  {
    auto& prop = this->ShaderMap[shaderPrim.GetPath().GetAsString()];

    if (prop == nullptr)
    {
      pxr::UsdAttribute attr = shaderPrim.GetIdAttr();

      if (attr)
      {
        pxr::TfToken materialToken;

        bool defined = attr.Get(&materialToken);

        if (!defined || materialToken != pxr::TfToken("UsdPreviewSurface"))
        {
          // only UsdPreviewSurface supported for now
          return nullptr;
        }

        prop = vtkSmartPointer<vtkProperty>::New();
        prop->SetInterpolationToPBR();

        vtkInformation* info = prop->GetInformation();

        // diffuseColor
        pxr::GfVec3f diffuseColorValue;
        pxr::UsdShadeInput diffuseColor = shaderPrim.GetInput(pxr::TfToken("diffuseColor"));
        if (diffuseColor && diffuseColor.Get(&diffuseColorValue))
        {
          prop->SetColor(diffuseColorValue[0], diffuseColorValue[1], diffuseColorValue[2]);
        }

        auto [diffuseColorSampler, colorToken] = this->GetConnectedShaderPrim(diffuseColor);
        vtkSmartPointer<vtkImageData> diffuseColorImage;
        if (diffuseColorSampler)
        {
          auto image = this->GetVTKTexture(diffuseColorSampler, colorToken);
          if (image)
          {
            diffuseColorImage = image;
            info->Copy(image->GetInformation());
          }
        }

        // opacity
        float opacityValue;
        pxr::UsdShadeInput opacity = shaderPrim.GetInput(pxr::TfToken("opacity"));
        if (opacity && opacity.Get(&opacityValue))
        {
          prop->SetOpacity(opacityValue);
        }

        auto [opacitySampler, opacityToken] = this->GetConnectedShaderPrim(opacity);
        vtkSmartPointer<vtkImageData> opacityImage;
        if (opacitySampler)
        {
          auto image = this->GetVTKTexture(opacitySampler, opacityToken);
          if (image)
          {
            opacityImage = image;
            info->Copy(image->GetInformation());
          }
        }

        auto baseColor = this->CombineColorOpacityImage(diffuseColorImage, opacityImage);
        if (baseColor)
        {
          vtkNew<vtkTexture> texture;
          texture->SetInputData(baseColor);

          texture->MipmapOn();
          texture->InterpolateOn();
          texture->SetColorModeToDirectScalars();

          texture->UseSRGBColorSpaceOn();

          prop->SetBaseColorTexture(texture);
        }

        // emissive
        pxr::UsdShadeInput emissive = shaderPrim.GetInput(pxr::TfToken("emissiveColor"));
        auto [emissiveSampler, emissiveToken] = this->GetConnectedShaderPrim(emissive);
        if (emissiveSampler)
        {
          auto image = this->GetVTKTexture(emissiveSampler, emissiveToken);
          if (image)
          {
            vtkNew<vtkTexture> texture;
            texture->SetInputData(image);

            texture->MipmapOn();
            texture->InterpolateOn();
            texture->SetColorModeToDirectScalars();

            texture->UseSRGBColorSpaceOn();

            prop->SetEmissiveTexture(texture);

            info->Copy(image->GetInformation());
          }
        }

        // ORM texture
        float roughnessValue;
        pxr::UsdShadeInput roughness = shaderPrim.GetInput(pxr::TfToken("roughness"));
        if (roughness && roughness.Get(&roughnessValue))
        {
          prop->SetRoughness(roughnessValue);
        }

        auto [roughnessSampler, roughnessToken] = this->GetConnectedShaderPrim(roughness);
        vtkSmartPointer<vtkImageData> roughnessImage;
        if (roughnessSampler)
        {
          prop->SetRoughness(1.0);
          auto image = this->GetVTKTexture(roughnessSampler, roughnessToken);
          if (image)
          {
            roughnessImage = image;
            info->Copy(image->GetInformation());
          }
        }

        float metallicValue;
        pxr::UsdShadeInput metallic = shaderPrim.GetInput(pxr::TfToken("metallic"));
        if (metallic && metallic.Get(&metallicValue))
        {
          prop->SetMetallic(metallicValue);
        }

        auto [metallicSampler, metallicToken] = this->GetConnectedShaderPrim(metallic);
        vtkSmartPointer<vtkImageData> metallicImage;
        if (metallicSampler)
        {
          prop->SetMetallic(1.0);
          auto image = this->GetVTKTexture(metallicSampler, metallicToken);
          if (image)
          {
            metallicImage = image;
            info->Copy(image->GetInformation());
          }
        }

        pxr::UsdShadeInput occlusion = shaderPrim.GetInput(pxr::TfToken("occlusion"));
        auto [occlusionSampler, occlusionToken] = this->GetConnectedShaderPrim(occlusion);
        vtkSmartPointer<vtkImageData> occlusionImage;
        if (occlusionSampler)
        {
          auto image = this->GetVTKTexture(occlusionSampler, occlusionToken);
          if (image)
          {
            occlusionImage = image;
            info->Copy(image->GetInformation());
          }
        }

        auto orm = this->CombineORMImage(occlusionImage, roughnessImage, metallicImage);

        if (orm)
        {
          vtkNew<vtkTexture> texture;
          texture->SetInputData(orm);

          texture->MipmapOn();
          texture->InterpolateOn();
          texture->SetColorModeToDirectScalars();

          prop->SetORMTexture(texture);
        }

        // normal
        pxr::UsdShadeInput normal = shaderPrim.GetInput(pxr::TfToken("normal"));
        auto [normalSampler, normalToken] = this->GetConnectedShaderPrim(normal);
        if (normalSampler)
        {
          auto image = this->GetVTKTexture(normalSampler, normalToken);

          if (image)
          {
            vtkNew<vtkTexture> texture;
            texture->SetInputData(image);

            texture->MipmapOn();
            texture->InterpolateOn();
            texture->SetColorModeToDirectScalars();

            prop->SetNormalTexture(texture);

            info->Copy(image->GetInformation());
          }
        }
      }
    }

    return prop;
  }

  bool HasTimeCode()
  {
    return this->Stage ? this->Stage->HasAuthoredTimeCodeRange() : false;
  }

  void SetCurrentTime(double currentTime)
  {
    this->CurrentTime = currentTime;
  }

  void GetTimeRange(double timeRange[2])
  {
    timeRange[0] = this->Stage->GetStartTimeCode() / this->Stage->GetTimeCodesPerSecond();
    timeRange[1] = this->Stage->GetEndTimeCode() / this->Stage->GetTimeCodesPerSecond();
  }

  pxr::UsdStageRefPtr Stage = nullptr;

private:
  std::unordered_map<std::string, vtkSmartPointer<vtkActor> > ActorMap;
  std::unordered_map<std::string, vtkSmartPointer<vtkPolyData> > MeshMap;
  std::unordered_map<std::string, vtkSmartPointer<vtkProperty> > ShaderMap;
  std::unordered_map<std::string, vtkSmartPointer<vtkImageData> > TextureMap;
  double CurrentTime = 0.0;
};

vtkStandardNewMacro(vtkF3DUSDImporter);

vtkInformationKeyMacro(vtkF3DUSDImporter, TCOORDS_NAME, String);

//----------------------------------------------------------------------------
vtkF3DUSDImporter::vtkF3DUSDImporter()
  : Internals(new vtkF3DUSDImporter::vtkInternals())
{
}

//----------------------------------------------------------------------------
vtkF3DUSDImporter::~vtkF3DUSDImporter() = default;

//----------------------------------------------------------------------------
int vtkF3DUSDImporter::ImportBegin()
{
  this->Internals->ReadScene(this->FileName);

  return 1;
}

//----------------------------------------------------------------------------
void vtkF3DUSDImporter::ImportActors(vtkRenderer* renderer)
{
  this->Internals->ImportRoot(renderer);
}

//----------------------------------------------------------------------------
vtkIdType vtkF3DUSDImporter::GetNumberOfAnimations()
{
  return this->Internals->HasTimeCode() ? 1 : 0;
}

// Complete GetTemporalInformation needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/7246
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20200912)
//----------------------------------------------------------------------------
bool vtkF3DUSDImporter::GetTemporalInformation(vtkIdType vtkNotUsed(animationIndex),
  double frameRate, int& nbTimeSteps, double timeRange[2], vtkDoubleArray* timeSteps)
{
  this->Internals->GetTimeRange(timeRange);

  nbTimeSteps = static_cast<int>((timeRange[1] - timeRange[0]) * frameRate);

  for (int i = 0; i < nbTimeSteps; i++)
  {
    double timestep = timeRange[0] + static_cast<double>(i) / frameRate;
    timeSteps->InsertNextTypedTuple(&timestep);
  }

  return true;
}
#endif

//----------------------------------------------------------------------------
void vtkF3DUSDImporter::UpdateTimeStep(double timeStep)
{
  this->Internals->SetCurrentTime(timeStep);
  this->Update();
}

//----------------------------------------------------------------------------
void vtkF3DUSDImporter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << "\n";
  os << indent << "AnimationEnabled: " << std::boolalpha << this->AnimationEnabled << "\n";
}
