/**
 * @class   vtkF3DMetaImporter
 * @brief
 */

#ifndef vtkF3DMetaImporter_h
#define vtkF3DMetaImporter_h

#include "F3DColoringInfoHandler.h"
#include "vtkF3DImporter.h"

#include <vtkActor.h>
#include <vtkBoundingBox.h>
#include <vtkPointGaussianMapper.h>
#include <vtkProperty.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVersion.h>
#include <vtkVolume.h>

#if VTK_VERSION_NUMBER < VTK_VERSION_CHECK(9, 3, 20240707)
#include <vtkActorCollection.h>
#endif

#include <memory>
#include <optional>
#include <string>
#include <vector>

class vtkF3DMetaImporter : public vtkF3DImporter
{
public:
  static vtkF3DMetaImporter* New();
  vtkTypeMacro(vtkF3DMetaImporter, vtkF3DImporter);

  ///@{
  /**
   * Structs used to transfer actors information to the F3D renderer
   */
  struct VolumeStruct
  {
    VolumeStruct()
    {
      this->Mapper->SetRequestedRenderModeToGPU();
      this->Prop->SetMapper(this->Mapper);
    }
    vtkNew<vtkVolume> Prop;
    vtkNew<vtkSmartVolumeMapper> Mapper;
  };

  struct PointSpritesStruct
  {
    explicit PointSpritesStruct(vtkActor* originalActor, vtkImporter* importer)
      : OriginalActor(originalActor)
      , Importer(importer)
    {
      this->Actor->vtkProp3D::ShallowCopy(originalActor);
      this->Actor->SetMapper(this->Mapper);
    }

    vtkNew<vtkActor> Actor;
    vtkNew<vtkPointGaussianMapper> Mapper;
    vtkActor* OriginalActor;
    vtkImporter* Importer;
  };

  struct ColoringStruct
  {
    explicit ColoringStruct(vtkActor* originalActor)
      : OriginalActor(originalActor)
    {
      this->Actor->GetProperty()->SetPointSize(10.0);
      this->Actor->GetProperty()->SetLineWidth(1.0);
      this->Actor->GetProperty()->SetRoughness(0.3);
      this->Actor->GetProperty()->SetBaseIOR(1.5);
      this->Actor->GetProperty()->SetInterpolationToPBR();
      this->Actor->vtkProp3D::ShallowCopy(originalActor);
      this->Actor->SetMapper(this->Mapper);
      this->Mapper->InterpolateScalarsBeforeMappingOn();
    }
    vtkNew<vtkActor> Actor;
    vtkNew<vtkPolyDataMapper> Mapper;
    vtkActor* OriginalActor;
  };
  ///@}

  /**
   * Clear all importers and internal structures
   */
  void Clear();

  /**
   * Add an importer to update when importer all actors
   */
  void AddImporter(const vtkSmartPointer<vtkImporter>& importer);

  /**
   * Get the bounding box of all geometry actors
   * Should be called after actors have been imported
   */
  const vtkBoundingBox& GetGeometryBoundingBox();

  /**
   * Get a meta data description of all imported data
   */
  std::string GetMetaDataDescription() const;

  F3DColoringInfoHandler& GetColoringInfoHandler();

  ///@{
  /**
   * API to recover information about all imported actors, point sprites and volume if any
   */
  const std::vector<ColoringStruct>& GetColoringActorsAndMappers();
  const std::vector<PointSpritesStruct>& GetPointSpritesActorsAndMappers();
  const std::vector<VolumeStruct>& GetVolumePropsAndMappers();
  ///@}

  /**
   * XXX: HIDE the vtkImporter::Update method and declare our own
   * Import each of of the add importers into the first renderer of the render window.
   * Importers that have already been imported will be skipped
   * Also handles camera index if specified
   * After import, create point sprites actors for all importers, and volume props
   * for generic importer if compatible.
   */
  bool Update();

  /**
   * Concatenate individual importers output description into one and return it
   */
  std::string GetOutputsDescription() override;

  ///@{
  /**
   * Implement vtkImporter animation API by adding animations for each individual importers one
   * after the other No input checking on animationIndex
   */
  AnimationSupportLevel GetAnimationSupportLevel() override;
  vtkIdType GetNumberOfAnimations() override;
  std::string GetAnimationName(vtkIdType animationIndex) override;
  void EnableAnimation(vtkIdType animationIndex) override;
  void DisableAnimation(vtkIdType animationIndex) override;
  bool IsAnimationEnabled(vtkIdType animationIndex) override;
  bool GetTemporalInformation(vtkIdType animationIndex, double frameRate, int& nbTimeSteps,
    double timeRange[2], vtkDoubleArray* timeSteps) override;
  ///@}

  ///@{
  /**
   * Implement vtkImporter camera API by adding cameras for each individual importers one after the
   * other No input checking on camIndex. Please note `void SetCamera(vtkIdType camIndex);` is not
   * reimplemented nor used.
   */
  vtkIdType GetNumberOfCameras() override;
  std::string GetCameraName(vtkIdType camIndex) override;
  void SetCameraIndex(std::optional<vtkIdType> camIndex);
  ///@}

  /**
   * Update each individual importer at the provided value
   */
  bool UpdateAtTimeValue(double timeValue) override;

  /**
   * Get the update mTime
   */
  vtkMTimeType GetUpdateMTime();

protected:
  vtkF3DMetaImporter();
  ~vtkF3DMetaImporter() override;

private:
  vtkF3DMetaImporter(const vtkF3DMetaImporter&) = delete;
  void operator=(const vtkF3DMetaImporter&) = delete;

  /**
   * Hide vtkImporter::SetCamera to ensure it is not being used
   */
  using vtkImporter::SetCamera;

  /**
   * Recover coloring information from each individual importer
   * and store result in internal fields
   */
  void UpdateInfoForColoring();

  struct Internals;
  std::unique_ptr<Internals> Pimpl;

#if VTK_VERSION_NUMBER < VTK_VERSION_CHECK(9, 3, 20240707)
  vtkNew<vtkActorCollection> ActorCollection;
#endif
};

#endif
