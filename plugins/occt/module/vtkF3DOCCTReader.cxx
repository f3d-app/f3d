#include "vtkF3DOCCTReader.h"

#include <BRepAdaptor_Surface.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRep_Tool.hxx>
#include <IGESControl_Reader.hxx>
#include <Message.hxx>
#include <Message_PrinterOStream.hxx>
#include <Message_ProgressIndicator.hxx>
#include <Poly.hxx>
#include <Poly_Triangulation.hxx>
#include <Quantity_Color.hxx>
#include <STEPControl_Reader.hxx>
#include <Standard_PrimitiveTypes.hxx>
#include <TColgp_Array1OfVec.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Solid.hxx>

#if F3D_PLUGIN_OCCT_XCAF
#include <IGESCAFControl_Reader.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDataStd_Name.hxx>
#include <TDocStd_Document.hxx>
#include <XCAFApp_Application.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_Location.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#endif

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCommand.h>
#include <vtkDemandDrivenPipeline.h>
#include <vtkFloatArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMatrix4x4.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtksys/SystemTools.hxx>

#include <array>
#include <numeric>
#include <unordered_map>
#include <vector>

class vtkF3DOCCTReader::vtkInternals
{
public:
  //----------------------------------------------------------------------------
  vtkInternals(vtkF3DOCCTReader* parent)
    : Parent(parent)
  {
  }

  //----------------------------------------------------------------------------
  vtkSmartPointer<vtkPolyData> CreateShape(const TopoDS_Shape& shape)
  {
    vtkNew<vtkPoints> points;
    vtkNew<vtkFloatArray> normals;
    normals->SetNumberOfComponents(3);
    normals->SetName("Normal");
    vtkNew<vtkFloatArray> uvs;
    uvs->SetNumberOfComponents(2);
    uvs->SetName("UV");
    vtkNew<vtkUnsignedCharArray> colors;
    colors->SetNumberOfComponents(3);
    colors->SetName("Colors");
    vtkNew<vtkCellArray> trianglesCells;
    vtkNew<vtkCellArray> linesCells;

    Standard_Integer shift = 0;

    if (this->Parent->GetReadWire())
    {
      // Add all edges to polydata
      for (TopExp_Explorer exEdge(shape, TopAbs_EDGE); exEdge.More(); exEdge.Next())
      {
        TopoDS_Edge edge = TopoDS::Edge(exEdge.Current());

        TopLoc_Location location;
        const auto& poly = BRep_Tool::Polygon3D(edge, location);

        if (poly.IsNull() || poly->Nodes().Length() <= 0)
        {
          // meshing
          BRepMesh_IncrementalMesh(edge, this->Parent->GetLinearDeflection(),
            this->Parent->GetRelativeDeflection(), this->Parent->GetAngularDeflection(),
            Standard_True);
        }

        if (poly.IsNull())
        {
          continue;
        }

        Standard_Integer nbV = poly->NbNodes();

        // Points
        const TColgp_Array1OfPnt& aNodes = poly->Nodes();
        for (Standard_Integer i = 1; i <= nbV; i++)
        {
          gp_Pnt pt = aNodes(i).Transformed(location);
          points->InsertNextPoint(pt.X(), pt.Y(), pt.Z());

          // normals and uvs make no sense for lines
          float fn[3] = { 0.0, 0.0, 1.0 };
          normals->InsertNextTypedTuple(fn);
          uvs->InsertNextTypedTuple(fn);
        }

        std::vector<vtkIdType> polyline(nbV - 1);
        std::iota(polyline.begin(), polyline.end(), shift);
        linesCells->InsertNextCell(polyline.size(), polyline.data());

#if F3D_PLUGIN_OCCT_XCAF
        std::array<unsigned char, 3> rgb = { 0, 0, 0 };
        Quantity_Color aColor;
        if (this->ColorTool->GetColor(edge, XCAFDoc_ColorCurv, aColor))
        {
          rgb[0] = static_cast<unsigned char>(255.0 * aColor.Red());
          rgb[1] = static_cast<unsigned char>(255.0 * aColor.Green());
          rgb[2] = static_cast<unsigned char>(255.0 * aColor.Blue());
        }
        colors->InsertNextTypedTuple(rgb.data());
#endif

        shift += nbV;
      }
    }

    // Add all faces to polydata
    for (TopExp_Explorer exFace(shape, TopAbs_FACE); exFace.More(); exFace.Next())
    {
      TopoDS_Face face = TopoDS::Face(exFace.Current());

      TopLoc_Location location;
      const auto& poly = BRep_Tool::Triangulation(face, location);

      if (poly.IsNull() || poly->NbTriangles() <= 0)
      {
        // meshing
        BRepMesh_IncrementalMesh(face, this->Parent->GetLinearDeflection(),
          this->Parent->GetRelativeDeflection(), this->Parent->GetAngularDeflection(),
          Standard_True);
      }

      if (poly.IsNull())
      {
        continue;
      }

      Poly::ComputeNormals(poly);
      TopAbs_Orientation faceOrientation = face.Orientation();

      Standard_Integer nbT = poly->NbTriangles();
      Standard_Integer nbV = poly->NbNodes();

      // Points
      for (Standard_Integer i = 1; i <= nbV; i++)
      {
        gp_Pnt pt = poly->Node(i).Transformed(location);
        points->InsertNextPoint(pt.X(), pt.Y(), pt.Z());
      }

      // Normals
      if (poly->HasNormals())
      {
        for (Standard_Integer i = 1; i <= nbV; i++)
        {
          gp_Dir n = poly->Normal(i);
          float fn[3] = { static_cast<float>(n.X()), static_cast<float>(n.Y()),
            static_cast<float>(n.Z()) };
          if (faceOrientation == TopAbs_Orientation::TopAbs_REVERSED)
          {
            vtkMath::MultiplyScalar(fn, -1.f);
          }
          normals->InsertNextTypedTuple(fn);
        }
      }
      else
      {
        // just in case a face does not have normals, add a dummy normal
        float fn[3] = { 0.0, 0.0, 1.0 };
        for (Standard_Integer i = 1; i <= nbV; i++)
        {
          normals->InsertNextTypedTuple(fn);
        }
      }

      // UVs
      if (poly->HasUVNodes())
      {
        for (Standard_Integer i = 1; i <= nbV; i++)
        {
          gp_Pnt2d uv = poly->UVNode(i);
          float fn[2] = { static_cast<float>(uv.X()), static_cast<float>(uv.Y()) };
          uvs->InsertNextTypedTuple(fn);
        }
      }
      else
      {
        float fn[3] = { 0.0, 0.0 };
        for (Standard_Integer i = 1; i <= nbV; i++)
        {
          uvs->InsertNextTypedTuple(fn);
        }
      }

      for (int i = 1; i <= nbT; i++)
      {
        int n1, n2, n3;
        poly->Triangle(i).Get(n1, n2, n3);

        vtkIdType cell[3] = { shift + n1 - 1, shift + n2 - 1, shift + n3 - 1 };
        if (faceOrientation != TopAbs_Orientation::TopAbs_FORWARD)
        {
          std::swap(cell[0], cell[2]);
        }
        trianglesCells->InsertNextCell(3, cell);

#if F3D_PLUGIN_OCCT_XCAF
        std::array<unsigned char, 3> rgb = { 255, 255, 255 };
        Quantity_Color aColor;
        if (this->ColorTool->GetColor(face, XCAFDoc_ColorSurf, aColor))
        {
          rgb[0] = static_cast<unsigned char>(255.0 * aColor.Red());
          rgb[1] = static_cast<unsigned char>(255.0 * aColor.Green());
          rgb[2] = static_cast<unsigned char>(255.0 * aColor.Blue());
        }
        colors->InsertNextTypedTuple(rgb.data());
#endif
      }

      shift += nbV;
    }

    vtkNew<vtkPolyData> polydata;
    polydata->SetPoints(points);
    polydata->GetPointData()->SetNormals(normals);
    polydata->GetPointData()->SetTCoords(uvs);
    polydata->SetPolys(trianglesCells);
    polydata->SetLines(linesCells);

#if F3D_PLUGIN_OCCT_XCAF
    polydata->GetCellData()->SetScalars(colors);
#endif

    polydata->Squeeze();
    return polydata;
  }

#if F3D_PLUGIN_OCCT_XCAF
  //----------------------------------------------------------------------------
  void AddLabel(const TDF_Label& label, vtkMatrix4x4* position, vtkMultiBlockDataSet* mb)
  {
    if (this->ShapeTool->IsSimpleShape(label) && this->ShapeTool->IsTopLevel(label))
    {
      vtkPolyData* polydata = this->ShapeMap[this->GetHash(label)];
      if (polydata && polydata->GetNumberOfCells() > 0)
      {
        vtkNew<vtkTransformFilter> transfoFilter;
        vtkNew<vtkTransform> transfo;
        transfo->SetMatrix(position);
        transfoFilter->SetTransform(transfo);
        transfoFilter->SetInputData(polydata);
        transfoFilter->Update();

        vtkIdType blockId = mb->GetNumberOfBlocks();
        mb->SetBlock(blockId, transfoFilter->GetOutput());

        vtkInformation* info = mb->GetMetaData(blockId);
        info->Set(vtkMultiBlockDataSet::NAME(), this->GetName(label));
      }
    }

    for (TDF_ChildIterator it(label); it.More(); it.Next())
    {
      TDF_Label child = it.Value();

      vtkNew<vtkMatrix4x4> mat;
      this->GetLocation(child, mat);
      vtkMatrix4x4::Multiply4x4(position, mat, mat);

      vtkNew<vtkMultiBlockDataSet> childMb;

      vtkIdType blockId = mb->GetNumberOfBlocks();
      mb->SetBlock(blockId, childMb);

      vtkInformation* info = mb->GetMetaData(blockId);
      info->Set(vtkMultiBlockDataSet::NAME(), this->GetName(child));

      if (this->ShapeTool->IsReference(child))
      {
        TDF_Label ref;
        this->ShapeTool->GetReferredShape(child, ref);

        vtkNew<vtkMatrix4x4> refMat;
        this->GetLocation(ref, refMat);
        vtkMatrix4x4::Multiply4x4(mat, refMat, mat);
        this->AddLabel(ref, mat, childMb);
      }
      else
      {
        this->AddLabel(child, mat, childMb);
      }
    }
  }

  //----------------------------------------------------------------------------
  static std::string GetName(const TDF_Label& label)
  {
    Handle(TDataStd_Name) name;
    std::string part_name = "Unnamed";
    if (label.FindAttribute(TDataStd_Name::GetID(), name))
    {
      TCollection_ExtendedString extstr = name->Get();

      char* str = new char[extstr.LengthOfCString() + 1];
      extstr.ToUTF8CString(str);
      part_name = str;
      delete[] str;

      if (part_name.empty())
      {
        part_name = "(empty)";
      }
    }
    return part_name;
  }

  //----------------------------------------------------------------------------
  int GetHash(const TDF_Label& label)
  {
    TopoDS_Shape aShape;
    return this->ShapeTool->GetShape(label, aShape) ? aShape.HashCode(INT_MAX) : 0;
  }

  //----------------------------------------------------------------------------
  static void GetMatrix(const TopLoc_Location& loc, vtkMatrix4x4* mat)
  {
    const gp_Trsf& transfo = loc.Transformation();
    gp_Mat vecto = transfo.VectorialPart();
    gp_XYZ trans = transfo.TranslationPart();

    mat->Identity();

    for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 3; j++)
      {
        mat->SetElement(i, j, vecto(i + 1, j + 1));
      }
    }
    mat->SetElement(0, 3, trans.X());
    mat->SetElement(1, 3, trans.Y());
    mat->SetElement(2, 3, trans.Z());
  }

  //----------------------------------------------------------------------------
  static void GetLocation(const TDF_Label& label, vtkMatrix4x4* location)
  {
    Handle(XCAFDoc_Location) hLoc;
    if (label.FindAttribute(XCAFDoc_Location::GetID(), hLoc))
    {
      GetMatrix(hLoc->Get(), location);
    }
  }

  std::unordered_map<int, vtkSmartPointer<vtkPolyData> > ShapeMap;
  Handle(XCAFDoc_ShapeTool) ShapeTool;
  Handle(XCAFDoc_ColorTool) ColorTool;
#endif

  vtkF3DOCCTReader* Parent;
};

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3DOCCTReader);

//----------------------------------------------------------------------------
vtkF3DOCCTReader::vtkF3DOCCTReader()
  : Internals(new vtkF3DOCCTReader::vtkInternals(this))
{
  this->SetNumberOfInputPorts(0);
}

//----------------------------------------------------------------------------
vtkF3DOCCTReader::~vtkF3DOCCTReader() = default;

//----------------------------------------------------------------------------
class ProgressIndicator : public Message_ProgressIndicator
{
public:
  ProgressIndicator(vtkF3DOCCTReader* reader) { this->Reader = reader; }

protected:
  void Show(const Message_ProgressScope&, const Standard_Boolean) override
  {
    double currentPosition = this->GetPosition();
    if (currentPosition - this->LastPosition > 0.01)
    {
      double localProgress = 0.5 * currentPosition;
      this->Reader->InvokeEvent(vtkCommand::ProgressEvent, &localProgress);
      this->LastPosition = currentPosition;
    }
  }

private:
  double LastPosition = 0.0;
  vtkF3DOCCTReader* Reader = nullptr;
};

#if F3D_PLUGIN_OCCT_XCAF
//----------------------------------------------------------------------------
template<typename T>
bool TransferToDocument(vtkF3DOCCTReader* that, T& reader, Handle(TDocStd_Document) doc)
{
  reader.SetColorMode(true);
  reader.SetNameMode(true);
  reader.SetLayerMode(true);

  if (reader.ReadFile(that->GetFileName().c_str()) == IFSelect_RetDone)
  {
    ProgressIndicator pi(that);
    return reader.Transfer(doc, pi.Start());
  }
  else
  {
    vtkErrorWithObjectMacro(that, "Failed opening file " << that->GetFileName());
    return false;
  }
}
#endif

//----------------------------------------------------------------------------
int vtkF3DOCCTReader::RequestData(
  vtkInformation*, vtkInformationVector**, vtkInformationVector* outputVector)
{
  vtkMultiBlockDataSet* output = vtkMultiBlockDataSet::GetData(outputVector);

  Message::DefaultMessenger()->RemovePrinters(STANDARD_TYPE(Message_PrinterOStream));

#if F3D_PLUGIN_OCCT_XCAF
  Handle(TDocStd_Document) doc;
  XCAFApp_Application::GetApplication()->NewDocument("MDTV-XCAF", doc);
  if (this->FileFormat == FILE_FORMAT::STEP)
  {
    STEPCAFControl_Reader reader;
    TransferToDocument(this, reader, doc);
  }
  else // FILE_FORMAT::IGES
  {
    IGESCAFControl_Reader reader;
    TransferToDocument(this, reader, doc);
  }

  this->Internals->ShapeTool = XCAFDoc_DocumentTool::ShapeTool(doc->Main());
  this->Internals->ColorTool = XCAFDoc_DocumentTool::ColorTool(doc->Main());

  TDF_LabelSequence topLevelShapes;

  // create polydata leaves
  this->Internals->ShapeTool->GetShapes(topLevelShapes);

  for (Standard_Integer iLabel = 1; iLabel <= topLevelShapes.Length(); ++iLabel)
  {
    TDF_Label label = topLevelShapes.Value(iLabel);

    TopoDS_Shape shape;
    this->Internals->ShapeTool->GetShape(label, shape);

    this->Internals->ShapeMap[this->Internals->GetHash(label)] =
      this->Internals->CreateShape(shape);

    double progress = 0.5 + static_cast<double>(iLabel) / topLevelShapes.Length();
    this->InvokeEvent(vtkCommand::ProgressEvent, &progress);
  }

  // create multiblock
  this->Internals->ShapeTool->GetFreeShapes(topLevelShapes);

  vtkNew<vtkMatrix4x4> mat;
  mat->Identity();
  for (Standard_Integer iLabel = 1; iLabel <= topLevelShapes.Length(); ++iLabel)
  {
    this->Internals->AddLabel(topLevelShapes.Value(iLabel), mat, output);
  }
#else
  XSControl_Reader* reader = nullptr;
  if (this->FileFormat == FILE_FORMAT::STEP)
  {
    reader = new STEPControl_Reader();
  }
  else // FILE_FORMAT::IGES
  {
    reader = new IGESControl_Reader();
  }

  if (reader && reader->ReadFile(this->GetFileName().c_str()) == IFSelect_RetDone)
  {
    ProgressIndicator pi(this);
    reader->TransferRoots(pi.Start());

    Standard_Integer nbShapes = reader->NbShapes();

    output->SetNumberOfBlocks(nbShapes);

    for (int iShape = 1; iShape <= nbShapes; iShape++)
    {
      TopoDS_Shape shape = reader->Shape(iShape);
      vtkSmartPointer<vtkPolyData> polydata = this->Internals->CreateShape(shape);

      if (polydata && polydata->GetNumberOfCells() > 0)
      {
        output->SetBlock(iShape, polydata);
      }
    }
  }

  delete reader;
#endif
  return 1;
}

//----------------------------------------------------------------------------
void vtkF3DOCCTReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << (this->FileName.empty() ? "(none)" : this->FileName) << "\n";
  os << indent << "LinearDeflection: " << this->LinearDeflection << "\n";
  os << indent << "AngularDeflection: " << this->AngularDeflection << "\n";
  os << indent << "RelativeDeflection: " << (this->RelativeDeflection ? "true" : "false") << "\n";
  os << indent << "ReadWire: " << (this->ReadWire ? "true" : "false") << "\n";
  os << indent << "FileFormat: " << (this->FileFormat == FILE_FORMAT::STEP ? "STEP" : "IGES")
     << "\n";
}
