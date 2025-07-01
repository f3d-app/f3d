#include "vtkF3DPLYReader.h"

#include <vtkCellData.h>
#include <vtkCommand.h>
#include <vtkDemandDrivenPipeline.h>
#include <vtkFloatArray.h>
#include <vtkIdTypeArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkNew.h>
#include <vtkPLY.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkUnsignedCharArray.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3DPLYReader);

//----------------------------------------------------------------------------
int vtkF3DPLYReader::RequestData(
  vtkInformation*, vtkInformationVector**, vtkInformationVector* outputVector)
{
  if (this->Superclass::RequestData(nullptr, nullptr, outputVector) == 0)
  {
    return 0;
  }

  vtkPolyData* output = vtkPolyData::GetData(outputVector);

  if (output->GetNumberOfPolys() > 0)
  {
    // if it's not a point cloud, just early return
    return 1;
  }

  // since it's a point cloud, look for 3D gaussians attributes

  struct Gaussian
  {
    float f_dc_0;
    float f_dc_1;
    float f_dc_2;
    float f_rest_0;
    float f_rest_1;
    float f_rest_2;
    float f_rest_3;
    float f_rest_4;
    float f_rest_5;
    float f_rest_6;
    float f_rest_7;
    float f_rest_8;
    float f_rest_9;
    float f_rest_10;
    float f_rest_11;
    float f_rest_12;
    float f_rest_13;
    float f_rest_14;
    float f_rest_15;
    float f_rest_16;
    float f_rest_17;
    float f_rest_18;
    float f_rest_19;
    float f_rest_20;
    float f_rest_21;
    float f_rest_22;
    float f_rest_23;
    float f_rest_24;
    float f_rest_25;
    float f_rest_26;
    float f_rest_27;
    float f_rest_28;
    float f_rest_29;
    float f_rest_30;
    float f_rest_31;
    float f_rest_32;
    float f_rest_33;
    float f_rest_34;
    float f_rest_35;
    float f_rest_36;
    float f_rest_37;
    float f_rest_38;
    float f_rest_39;
    float f_rest_40;
    float f_rest_41;
    float f_rest_42;
    float f_rest_43;
    float f_rest_44;
    float opacity;
    float scale_0;
    float scale_1;
    float scale_2;
    float rot_0;
    float rot_1;
    float rot_2;
    float rot_3;
  };

  PlyProperty vertProps[] = {
    { "f_dc_0", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_dc_0)), 0, 0, 0, 0 },
    { "f_dc_1", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_dc_1)), 0, 0, 0, 0 },
    { "f_dc_2", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_dc_2)), 0, 0, 0, 0 },
    { "f_rest_0", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_0)), 0, 0, 0,
      0 },
    { "f_rest_1", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_1)), 0, 0, 0,
      0 },
    { "f_rest_2", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_2)), 0, 0, 0,
      0 },
    { "f_rest_3", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_3)), 0, 0, 0,
      0 },
    { "f_rest_4", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_4)), 0, 0, 0,
      0 },
    { "f_rest_5", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_5)), 0, 0, 0,
      0 },
    { "f_rest_6", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_6)), 0, 0, 0,
      0 },
    { "f_rest_7", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_7)), 0, 0, 0,
      0 },
    { "f_rest_8", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_8)), 0, 0, 0,
      0 },
    { "f_rest_9", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_9)), 0, 0, 0,
      0 },
    { "f_rest_10", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_10)), 0, 0, 0,
      0 },
    { "f_rest_11", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_11)), 0, 0, 0,
      0 },
    { "f_rest_12", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_12)), 0, 0, 0,
      0 },
    { "f_rest_13", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_13)), 0, 0, 0,
      0 },
    { "f_rest_14", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_14)), 0, 0, 0,
      0 },
    { "f_rest_15", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_15)), 0, 0, 0,
      0 },
    { "f_rest_16", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_16)), 0, 0, 0,
      0 },
    { "f_rest_17", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_17)), 0, 0, 0,
      0 },
    { "f_rest_18", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_18)), 0, 0, 0,
      0 },
    { "f_rest_19", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_19)), 0, 0, 0,
      0 },
    { "f_rest_20", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_20)), 0, 0, 0,
      0 },
    { "f_rest_21", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_21)), 0, 0, 0,
      0 },
    { "f_rest_22", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_22)), 0, 0, 0,
      0 },
    { "f_rest_23", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_23)), 0, 0, 0,
      0 },
    { "f_rest_24", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_24)), 0, 0, 0,
      0 },
    { "f_rest_25", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_25)), 0, 0, 0,
      0 },
    { "f_rest_26", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_26)), 0, 0, 0,
      0 },
    { "f_rest_27", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_27)), 0, 0, 0,
      0 },
    { "f_rest_28", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_28)), 0, 0, 0,
      0 },
    { "f_rest_29", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_29)), 0, 0, 0,
      0 },
    { "f_rest_30", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_30)), 0, 0, 0,
      0 },
    { "f_rest_31", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_31)), 0, 0, 0,
      0 },
    { "f_rest_32", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_32)), 0, 0, 0,
      0 },
    { "f_rest_33", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_33)), 0, 0, 0,
      0 },
    { "f_rest_34", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_34)), 0, 0, 0,
      0 },
    { "f_rest_35", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_35)), 0, 0, 0,
      0 },
    { "f_rest_36", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_36)), 0, 0, 0,
      0 },
    { "f_rest_37", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_37)), 0, 0, 0,
      0 },
    { "f_rest_38", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_38)), 0, 0, 0,
      0 },
    { "f_rest_39", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_39)), 0, 0, 0,
      0 },
    { "f_rest_40", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_40)), 0, 0, 0,
      0 },
    { "f_rest_41", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_41)), 0, 0, 0,
      0 },
    { "f_rest_42", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_42)), 0, 0, 0,
      0 },
    { "f_rest_43", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_43)), 0, 0, 0,
      0 },
    { "f_rest_44", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, f_rest_44)), 0, 0, 0,
      0 },
    { "opacity", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, opacity)), 0, 0, 0, 0 },
    { "scale_0", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, scale_0)), 0, 0, 0, 0 },
    { "scale_1", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, scale_1)), 0, 0, 0, 0 },
    { "scale_2", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, scale_2)), 0, 0, 0, 0 },
    { "rot_0", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, rot_0)), 0, 0, 0, 0 },
    { "rot_1", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, rot_1)), 0, 0, 0, 0 },
    { "rot_2", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, rot_2)), 0, 0, 0, 0 },
    { "rot_3", PLY_FLOAT, PLY_FLOAT, static_cast<int>(offsetof(Gaussian, rot_3)), 0, 0, 0, 0 },
  };

  // open a PLY file for reading
  PlyFile* ply;
  int nelems;
  char** elist;

  if (this->ReadFromInputStream)
  {
    if (!(ply = vtkPLY::ply_read(this->Stream, &nelems, &elist)))
    {
      vtkWarningMacro(<< "Could not open PLY file");
      return 0;
    }
  }
  else if (this->ReadFromInputString)
  {
    if (!(ply = vtkPLY::ply_open_for_reading_from_string(this->InputString, &nelems, &elist)))
    {
      vtkWarningMacro(<< "Could not open PLY file");
      return 0;
    }
  }
  else
  {
    if (!(ply = vtkPLY::ply_open_for_reading(this->FileName, &nelems, &elist)))
    {
      vtkWarningMacro(<< "Could not open PLY file");
      return 0;
    }
  }

  PlyElement* elem = vtkPLY::find_element(ply, "vertex");

  int numPts;
  int numProps;
  std::string elemName = "vertex";
  vtkPLY::ply_get_element_description(ply, elemName.data(), &numPts, &numProps);

  for (PlyProperty& prop : vertProps)
  {
    int index;
    if (vtkPLY::find_property(elem, prop.name, &index) == nullptr)
    {
      vtkPLY::ply_close(ply);
      return 1;
    }

    vtkPLY::ply_get_property(ply, "vertex", &prop);
  }

  vtkNew<vtkUnsignedCharArray> rgb;
  rgb->SetName("color");
  rgb->SetNumberOfComponents(4);
  rgb->SetNumberOfTuples(numPts);
  output->GetPointData()->SetScalars(rgb);

  vtkNew<vtkFloatArray> scale;
  scale->SetName("scale");
  scale->SetNumberOfComponents(3);
  scale->SetNumberOfTuples(numPts);
  output->GetPointData()->AddArray(scale);

  vtkNew<vtkFloatArray> rotation;
  rotation->SetName("rotation");
  rotation->SetNumberOfComponents(4);
  rotation->SetNumberOfTuples(numPts);
  output->GetPointData()->AddArray(rotation);

  auto initArray = [&](vtkUnsignedCharArray* shArray, const char* name) {
    shArray->SetName(name);
    shArray->SetNumberOfComponents(3);
    shArray->SetNumberOfTuples(numPts);
    output->GetPointData()->AddArray(shArray);
  };

  vtkNew<vtkUnsignedCharArray> sh1m1, sh10, sh1p1, sh2m2, sh2m1, sh20, sh2p1, sh2p2, sh3m3, sh3m2,
    sh3m1, sh30, sh3p1, sh3p2, sh3p3;
  initArray(sh1m1, "sh1m1");
  initArray(sh10, "sh10");
  initArray(sh1p1, "sh1p1");
  initArray(sh2m2, "sh2m2");
  initArray(sh2m1, "sh2m1");
  initArray(sh20, "sh20");
  initArray(sh2p1, "sh2p1");
  initArray(sh2p2, "sh2p2");
  initArray(sh3m3, "sh3m3");
  initArray(sh3m2, "sh3m2");
  initArray(sh3m1, "sh3m1");
  initArray(sh30, "sh30");
  initArray(sh3p1, "sh3p1");
  initArray(sh3p2, "sh3p2");
  initArray(sh3p3, "sh3p3");

  Gaussian gaussian;
  for (int j = 0; j < numPts; j++)
  {
    vtkPLY::ply_get_element(ply, &gaussian);

    auto sh0ToColor = [](float v) {
      return static_cast<unsigned char>(255.f * std::clamp(v * 0.282094791774f + 0.5f, 0.f, 1.f));
    };
    auto sigmoid = [](float v) { return 1.f / (1.f + std::exp(-v)); };
    auto quantizeOpacity = [](float v) { return static_cast<unsigned char>(255.f * v); };
    auto quantizeSH = [](float v) { return static_cast<unsigned char>(127.5f * (v + 1.f)); };

    // color
    rgb->SetTypedComponent(j, 0, sh0ToColor(gaussian.f_dc_0));
    rgb->SetTypedComponent(j, 1, sh0ToColor(gaussian.f_dc_1));
    rgb->SetTypedComponent(j, 2, sh0ToColor(gaussian.f_dc_2));
    rgb->SetTypedComponent(j, 3, quantizeOpacity(sigmoid(gaussian.opacity)));

    // scale
    scale->SetTypedComponent(j, 0, std::exp(gaussian.scale_0));
    scale->SetTypedComponent(j, 1, std::exp(gaussian.scale_1));
    scale->SetTypedComponent(j, 2, std::exp(gaussian.scale_2));

    // rotation
    rotation->SetTypedComponent(j, 0, gaussian.rot_0);
    rotation->SetTypedComponent(j, 1, gaussian.rot_1);
    rotation->SetTypedComponent(j, 2, gaussian.rot_2);
    rotation->SetTypedComponent(j, 3, gaussian.rot_3);

    // sherical harmonics
    auto setSHComponents = [&](vtkUnsignedCharArray* shArray, float shR, float shG, float shB) {
      shArray->SetTypedComponent(j, 0, quantizeSH(shR));
      shArray->SetTypedComponent(j, 1, quantizeSH(shG));
      shArray->SetTypedComponent(j, 2, quantizeSH(shB));
    };

    setSHComponents(sh1m1, gaussian.f_rest_0, gaussian.f_rest_15, gaussian.f_rest_30);
    setSHComponents(sh10, gaussian.f_rest_1, gaussian.f_rest_16, gaussian.f_rest_31);
    setSHComponents(sh1p1, gaussian.f_rest_2, gaussian.f_rest_17, gaussian.f_rest_32);
    setSHComponents(sh2m2, gaussian.f_rest_3, gaussian.f_rest_18, gaussian.f_rest_33);
    setSHComponents(sh2m1, gaussian.f_rest_4, gaussian.f_rest_19, gaussian.f_rest_34);
    setSHComponents(sh20, gaussian.f_rest_5, gaussian.f_rest_20, gaussian.f_rest_35);
    setSHComponents(sh2p1, gaussian.f_rest_6, gaussian.f_rest_21, gaussian.f_rest_36);
    setSHComponents(sh2p2, gaussian.f_rest_7, gaussian.f_rest_22, gaussian.f_rest_37);
    setSHComponents(sh3m3, gaussian.f_rest_8, gaussian.f_rest_23, gaussian.f_rest_38);
    setSHComponents(sh3m2, gaussian.f_rest_9, gaussian.f_rest_24, gaussian.f_rest_39);
    setSHComponents(sh3m1, gaussian.f_rest_10, gaussian.f_rest_25, gaussian.f_rest_40);
    setSHComponents(sh30, gaussian.f_rest_11, gaussian.f_rest_26, gaussian.f_rest_41);
    setSHComponents(sh3p1, gaussian.f_rest_12, gaussian.f_rest_27, gaussian.f_rest_42);
    setSHComponents(sh3p2, gaussian.f_rest_13, gaussian.f_rest_28, gaussian.f_rest_43);
    setSHComponents(sh3p3, gaussian.f_rest_14, gaussian.f_rest_29, gaussian.f_rest_44);
  }

  vtkPLY::ply_close(ply);

  return 1;
}
