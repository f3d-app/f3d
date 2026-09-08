#include "vtkF3DFCStdReader.h"

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCompositeDataSet.h>
#include <vtkDataAssembly.h>
#include <vtkFileResourceStream.h>
#include <vtkFloatArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPartitionedDataSet.h>
#include <vtkPartitionedDataSetCollection.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkResourceStream.h>
#include <vtkUnsignedCharArray.h>
#include <vtkXMLDataElement.h>
#include <vtkXMLDataParser.h>

#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <Poly.hxx>
#include <Poly_Polygon3D.hxx>
#include <Poly_Triangulation.hxx>
#include <Standard_Failure.hxx>
#include <TopAbs_Orientation.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Builder.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Quaternion.hxx>
#include <gp_Trsf.hxx>

#include <miniz.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <map>
#include <numeric>
#include <optional>
#include <sstream>
#include <vector>

namespace
{
constexpr std::array<unsigned char, 3> DEFAULT_COLOR = { 204, 204, 204 };

//----------------------------------------------------------------------------
class ArchiveReader
{
public:
  ~ArchiveReader()
  {
    if (this->Opened)
    {
      mz_zip_reader_end(&this->Archive);
    }
  }

  bool Open(vtkResourceStream* stream)
  {
    if (!stream || !stream->SupportSeek())
    {
      return false;
    }
    this->Stream = stream;
    stream->Seek(0, vtkResourceStream::SeekDirection::End);
    const mz_uint64 size = static_cast<mz_uint64>(stream->Tell());

    mz_zip_zero_struct(&this->Archive);
    this->Archive.m_pRead = &ArchiveReader::ReadCallback;
    this->Archive.m_pIO_opaque = this;
    this->Opened = mz_zip_reader_init(&this->Archive, size, 0);
    return this->Opened;
  }

  bool Has(const std::string& name)
  {
    return this->Opened && mz_zip_reader_locate_file(&this->Archive, name.c_str(), nullptr, 0) >= 0;
  }

  bool Extract(const std::string& name, std::vector<char>& out)
  {
    if (!this->Opened)
    {
      return false;
    }
    const int index = mz_zip_reader_locate_file(&this->Archive, name.c_str(), nullptr, 0);
    if (index < 0)
    {
      return false;
    }
    mz_zip_archive_file_stat stat;
    if (!mz_zip_reader_file_stat(&this->Archive, index, &stat))
    {
      return false;
    }
    out.resize(static_cast<size_t>(stat.m_uncomp_size));
    return mz_zip_reader_extract_to_mem(&this->Archive, index, out.data(), out.size(), 0);
  }

private:
  static size_t ReadCallback(void* opaque, mz_uint64 fileOfs, void* buf, size_t n)
  {
    ArchiveReader* self = static_cast<ArchiveReader*>(opaque);
    self->Stream->Seek(static_cast<vtkTypeInt64>(fileOfs), vtkResourceStream::SeekDirection::Begin);
    return self->Stream->Read(buf, n);
  }

  mz_zip_archive Archive = {};
  vtkResourceStream* Stream = nullptr;
  bool Opened = false;
};

//----------------------------------------------------------------------------
struct FCObject
{
  std::string Name;
  std::string Label;
  std::string Type;
  std::string ShapeFile;
  gp_Trsf Placement;
  std::optional<bool> Visible;

  // App::Link and link array support
  std::string LinkedObject;
  bool LinkedExternal = false;
  gp_Trsf LinkPlacement;
  bool LinkTransform = false;
  std::string BaseLink;
  std::string PlacementListFile;

  // hierarchy (property "Group" of container objects)
  std::vector<std::string> Children;
  std::string Parent;

  // appearance
  std::optional<std::array<unsigned char, 3>> Color;
  std::vector<std::array<unsigned char, 4>> FaceColors;
  int Transparency = 0; // FreeCAD percentage, 0 is opaque
  int DisplayMode = 0;

  [[nodiscard]] bool IsWireframeOnly() const
  {
    return this->DisplayMode == 1 && this->Type.rfind("Part::Part2DObject", 0) == 0;
  }

  [[nodiscard]] bool IsLink() const
  {
    return !this->LinkedObject.empty();
  }
  [[nodiscard]] bool IsGeoFeatureGroup() const
  {
    return this->Type == "App::Part" || this->Type == "Assembly::AssemblyObject";
  }
};

//----------------------------------------------------------------------------
uint32_t ReadLEUint32(const char* data)
{
  uint32_t value;
  std::memcpy(&value, data, sizeof(uint32_t));
#ifdef VTK_WORDS_BIGENDIAN
  value = ((value & 0xFF000000) >> 24) | ((value & 0x00FF0000) >> 8) | ((value & 0x0000FF00) << 8) |
    ((value & 0x000000FF) << 24);
#endif
  return value;
}

//----------------------------------------------------------------------------
double ReadLEDouble(const char* data)
{
#ifdef VTK_WORDS_BIGENDIAN
  char swapped[sizeof(double)];
  for (size_t i = 0; i < sizeof(double); i++)
  {
    swapped[i] = data[sizeof(double) - 1 - i];
  }
  data = swapped;
#endif
  double value;
  std::memcpy(&value, data, sizeof(double));
  return value;
}

//----------------------------------------------------------------------------
// FreeCAD packs colors as 0xRRGGBBAA
std::array<unsigned char, 3> UnpackColor(uint32_t packed)
{
  return { static_cast<unsigned char>((packed >> 24) & 0xFF),
    static_cast<unsigned char>((packed >> 16) & 0xFF),
    static_cast<unsigned char>((packed >> 8) & 0xFF) };
}

//----------------------------------------------------------------------------
gp_Trsf ParsePlacement(vtkXMLDataElement* elem)
{
  gp_Trsf trsf;
  double px = 0., py = 0., pz = 0., q0 = 0., q1 = 0., q2 = 0., q3 = 1.;
  elem->GetScalarAttribute("Px", px);
  elem->GetScalarAttribute("Py", py);
  elem->GetScalarAttribute("Pz", pz);
  elem->GetScalarAttribute("Q0", q0);
  elem->GetScalarAttribute("Q1", q1);
  elem->GetScalarAttribute("Q2", q2);
  elem->GetScalarAttribute("Q3", q3);
  const gp_Quaternion quat(q0, q1, q2, q3);
  if (quat.SquareNorm() > 0.)
  {
    trsf.SetRotation(quat);
  }
  trsf.SetTranslationPart(gp_Vec(px, py, pz));
  return trsf;
}

//----------------------------------------------------------------------------
vtkXMLDataElement* FindNested(vtkXMLDataElement* elem, const char* name)
{
  for (int i = 0; i < elem->GetNumberOfNestedElements(); i++)
  {
    vtkXMLDataElement* nested = elem->GetNestedElement(i);
    if (nested->GetName() && std::strcmp(nested->GetName(), name) == 0)
    {
      return nested;
    }
  }
  return nullptr;
}
}

//----------------------------------------------------------------------------
class vtkF3DFCStdReader::vtkInternals
{
public:
  explicit vtkInternals(vtkF3DFCStdReader* parent)
    : Parent(parent)
  {
  }

  //----------------------------------------------------------------------------
  bool ParseXML(const std::vector<char>& content, vtkNew<vtkXMLDataParser>& parser)
  {
    std::istringstream stream(std::string(content.data(), content.size()));
    parser->SetStream(&stream);
    return parser->Parse() != 0 && parser->GetRootElement() != nullptr;
  }

  //----------------------------------------------------------------------------
  bool ParseDocument(const std::vector<char>& content)
  {
    vtkNew<vtkXMLDataParser> parser;
    if (!this->ParseXML(content, parser))
    {
      return false;
    }
    vtkXMLDataElement* root = parser->GetRootElement();

    // in files written before FreeCAD 1.1, the last byte of packed colors is a
    // transparency, not an alpha
    const char* version = root->GetAttribute("ProgramVersion");
    if (version)
    {
      int major = 0, minor = 0;
      std::sscanf(version, "%d.%d", &major, &minor);
      this->LegacyAlpha = major < 1 || (major == 1 && minor < 1);
    }

    vtkXMLDataElement* objects = FindNested(root, "Objects");
    if (!objects)
    {
      return false;
    }
    for (int i = 0; i < objects->GetNumberOfNestedElements(); i++)
    {
      vtkXMLDataElement* elem = objects->GetNestedElement(i);
      if (!elem->GetName() || std::strcmp(elem->GetName(), "Object") != 0)
      {
        continue;
      }
      const char* name = elem->GetAttribute("name");
      const char* type = elem->GetAttribute("type");
      if (name && type)
      {
        FCObject& obj = this->Objects[name];
        obj.Name = name;
        obj.Label = name;
        obj.Type = type;
        this->ObjectOrder.emplace_back(name);
      }
    }

    vtkXMLDataElement* objectData = FindNested(root, "ObjectData");
    if (!objectData)
    {
      return false;
    }
    for (int i = 0; i < objectData->GetNumberOfNestedElements(); i++)
    {
      vtkXMLDataElement* elem = objectData->GetNestedElement(i);
      if (!elem->GetName() || std::strcmp(elem->GetName(), "Object") != 0)
      {
        continue;
      }
      const char* name = elem->GetAttribute("name");
      auto it = name ? this->Objects.find(name) : this->Objects.end();
      if (it == this->Objects.end())
      {
        continue;
      }
      vtkXMLDataElement* props = FindNested(elem, "Properties");
      if (props)
      {
        this->ParseObjectProperties(props, it->second);
      }
    }

    for (const auto& [name, obj] : this->Objects)
    {
      for (const std::string& child : obj.Children)
      {
        auto childIt = this->Objects.find(child);
        if (childIt != this->Objects.end())
        {
          childIt->second.Parent = name;
        }
      }
    }
    return true;
  }

  //----------------------------------------------------------------------------
  void ParseObjectProperties(vtkXMLDataElement* props, FCObject& obj)
  {
    for (int i = 0; i < props->GetNumberOfNestedElements(); i++)
    {
      vtkXMLDataElement* prop = props->GetNestedElement(i);
      if (!prop->GetName() || std::strcmp(prop->GetName(), "Property") != 0)
      {
        continue;
      }
      const char* propName = prop->GetAttribute("name");
      if (!propName)
      {
        continue;
      }

      if (std::strcmp(propName, "Shape") == 0)
      {
        vtkXMLDataElement* part = FindNested(prop, "Part");
        const char* file = part ? part->GetAttribute("file") : nullptr;
        if (file && file[0] != '\0')
        {
          obj.ShapeFile = file;
        }
      }
      else if (std::strcmp(propName, "Label") == 0)
      {
        vtkXMLDataElement* str = FindNested(prop, "String");
        const char* value = str ? str->GetAttribute("value") : nullptr;
        if (value && value[0] != '\0')
        {
          obj.Label = value;
        }
      }
      else if (std::strcmp(propName, "Placement") == 0)
      {
        vtkXMLDataElement* placement = FindNested(prop, "PropertyPlacement");
        if (placement)
        {
          obj.Placement = ParsePlacement(placement);
        }
      }
      else if (std::strcmp(propName, "Visibility") == 0)
      {
        vtkXMLDataElement* boolean = FindNested(prop, "Bool");
        const char* value = boolean ? boolean->GetAttribute("value") : nullptr;
        if (value)
        {
          obj.Visible = std::strcmp(value, "true") == 0;
        }
      }
      else if (std::strcmp(propName, "LinkedObject") == 0)
      {
        vtkXMLDataElement* xlink = FindNested(prop, "XLink");
        if (xlink)
        {
          const char* linkName = xlink->GetAttribute("name");
          const char* linkFile = xlink->GetAttribute("file");
          if (linkName)
          {
            obj.LinkedObject = linkName;
            obj.LinkedExternal = linkFile && linkFile[0] != '\0';
          }
        }
      }
      else if (std::strcmp(propName, "LinkPlacement") == 0)
      {
        vtkXMLDataElement* placement = FindNested(prop, "PropertyPlacement");
        if (placement)
        {
          obj.LinkPlacement = ParsePlacement(placement);
        }
      }
      else if (std::strcmp(propName, "LinkTransform") == 0)
      {
        vtkXMLDataElement* boolean = FindNested(prop, "Bool");
        const char* value = boolean ? boolean->GetAttribute("value") : nullptr;
        obj.LinkTransform = value && std::strcmp(value, "true") == 0;
      }
      else if (std::strcmp(propName, "Base") == 0)
      {
        vtkXMLDataElement* link = FindNested(prop, "Link");
        const char* value = link ? link->GetAttribute("value") : nullptr;
        if (value && value[0] != '\0')
        {
          obj.BaseLink = value;
        }
      }
      else if (std::strcmp(propName, "PlacementList") == 0)
      {
        vtkXMLDataElement* list = FindNested(prop, "PlacementList");
        const char* file = list ? list->GetAttribute("file") : nullptr;
        if (file && file[0] != '\0')
        {
          obj.PlacementListFile = file;
        }
      }
      else if (std::strcmp(propName, "Group") == 0)
      {
        vtkXMLDataElement* list = FindNested(prop, "LinkList");
        if (list)
        {
          for (int j = 0; j < list->GetNumberOfNestedElements(); j++)
          {
            vtkXMLDataElement* link = list->GetNestedElement(j);
            const char* value = link->GetAttribute("value");
            if (value && value[0] != '\0')
            {
              obj.Children.emplace_back(value);
            }
          }
        }
      }
    }
  }

  //----------------------------------------------------------------------------
  void ParseGuiDocument(const std::vector<char>& content, ArchiveReader& archive)
  {
    vtkNew<vtkXMLDataParser> parser;
    if (!this->ParseXML(content, parser))
    {
      return;
    }
    vtkXMLDataElement* root = parser->GetRootElement();
    vtkXMLDataElement* viewProviderData = FindNested(root, "ViewProviderData");
    if (!viewProviderData)
    {
      return;
    }
    for (int i = 0; i < viewProviderData->GetNumberOfNestedElements(); i++)
    {
      vtkXMLDataElement* elem = viewProviderData->GetNestedElement(i);
      if (!elem->GetName() || std::strcmp(elem->GetName(), "ViewProvider") != 0)
      {
        continue;
      }
      const char* name = elem->GetAttribute("name");
      auto it = name ? this->Objects.find(name) : this->Objects.end();
      if (it == this->Objects.end())
      {
        continue;
      }
      vtkXMLDataElement* props = FindNested(elem, "Properties");
      if (props)
      {
        this->ParseViewProviderProperties(props, it->second, archive);
      }
    }
  }

  //----------------------------------------------------------------------------
  void ParseViewProviderProperties(vtkXMLDataElement* props, FCObject& obj, ArchiveReader& archive)
  {
    for (int i = 0; i < props->GetNumberOfNestedElements(); i++)
    {
      vtkXMLDataElement* prop = props->GetNestedElement(i);
      if (!prop->GetName() || std::strcmp(prop->GetName(), "Property") != 0)
      {
        continue;
      }
      const char* propName = prop->GetAttribute("name");
      if (!propName)
      {
        continue;
      }

      if (std::strcmp(propName, "Visibility") == 0)
      {
        // Document.xml visibility has priority, GuiDocument.xml is the
        // fallback.
        if (!obj.Visible.has_value())
        {
          vtkXMLDataElement* boolean = FindNested(prop, "Bool");
          const char* value = boolean ? boolean->GetAttribute("value") : nullptr;
          if (value)
          {
            obj.Visible = std::strcmp(value, "true") == 0;
          }
        }
      }
      else if (std::strcmp(propName, "ShapeColor") == 0)
      {
        vtkXMLDataElement* color = FindNested(prop, "PropertyColor");
        unsigned long packed = 0;
        if (color && color->GetScalarAttribute("value", packed))
        {
          obj.Color = UnpackColor(static_cast<uint32_t>(packed));
        }
      }
      else if (std::strcmp(propName, "Transparency") == 0)
      {
        vtkXMLDataElement* value = FindNested(prop, "Integer");
        int transparency = 0;
        if (value && value->GetScalarAttribute("value", transparency))
        {
          obj.Transparency = std::clamp(transparency, 0, 100);
        }
      }
      else if (std::strcmp(propName, "DisplayMode") == 0)
      {
        vtkXMLDataElement* value = FindNested(prop, "Integer");
        int mode = 0;
        if (value && value->GetScalarAttribute("value", mode))
        {
          obj.DisplayMode = mode;
        }
      }
      else if (std::strcmp(propName, "DiffuseColor") == 0)
      {
        vtkXMLDataElement* list = FindNested(prop, "ColorList");
        const char* file = list ? list->GetAttribute("file") : nullptr;
        if (file && file[0] != '\0')
        {
          this->ReadColorList(file, archive, obj);
        }
      }
      else if (std::strcmp(propName, "ShapeAppearance") == 0)
      {
        vtkXMLDataElement* list = FindNested(prop, "MaterialList");
        const char* file = list ? list->GetAttribute("file") : nullptr;
        if (file && file[0] != '\0')
        {
          this->ReadMaterialList(file, archive, obj);
        }
      }
    }
  }

  //----------------------------------------------------------------------------
  void ReadColorList(const std::string& file, ArchiveReader& archive, FCObject& obj)
  {
    std::vector<char> data;
    if (!archive.Extract(file, data) || data.size() < sizeof(uint32_t))
    {
      return;
    }
    const uint32_t count = ReadLEUint32(data.data());
    if (data.size() < sizeof(uint32_t) * (1 + static_cast<size_t>(count)))
    {
      return;
    }
    if (count == 1)
    {
      obj.Color = UnpackColor(ReadLEUint32(data.data() + sizeof(uint32_t)));
    }
    else
    {
      obj.FaceColors.resize(count, { 0, 0, 0, 255 });
      for (uint32_t i = 0; i < count; i++)
      {
        const uint32_t packed = ReadLEUint32(data.data() + sizeof(uint32_t) * (1 + i));
        const std::array<unsigned char, 3> rgb = UnpackColor(packed);
        const unsigned char lastByte = static_cast<unsigned char>(packed & 0xFF);
        const unsigned char alpha =
          this->LegacyAlpha ? static_cast<unsigned char>(255 - lastByte) : lastByte;
        obj.FaceColors[i] = { rgb[0], rgb[1], rgb[2], alpha };
      }
    }
  }

  //----------------------------------------------------------------------------
  void ReadMaterialList(const std::string& file, ArchiveReader& archive, FCObject& obj)
  {
    std::vector<char> data;
    if (!archive.Extract(file, data) || data.size() < sizeof(uint32_t))
    {
      return;
    }
    const uint32_t count = ReadLEUint32(data.data());
    constexpr size_t materialSize = 4 * sizeof(uint32_t) + 2 * sizeof(float);
    if (data.size() < sizeof(uint32_t) + materialSize * static_cast<size_t>(count))
    {
      return;
    }
    std::vector<std::array<unsigned char, 4>> colors(count);
    std::vector<float> transparencies(count);
    for (uint32_t i = 0; i < count; i++)
    {
      const char* material = data.data() + sizeof(uint32_t) + materialSize * i;
      const std::array<unsigned char, 3> rgb =
        UnpackColor(ReadLEUint32(material + sizeof(uint32_t)));
      float transparency;
      const uint32_t transparencyBits =
        ReadLEUint32(material + 4 * sizeof(uint32_t) + sizeof(float));
      std::memcpy(&transparency, &transparencyBits, sizeof(float));
      transparency = std::clamp(transparency, 0.f, 1.f);
      transparencies[i] = transparency;
      colors[i] = { rgb[0], rgb[1], rgb[2],
        static_cast<unsigned char>(255.f * (1.f - transparency) + 0.5f) };
    }
    if (count == 1)
    {
      obj.Color = { colors[0][0], colors[0][1], colors[0][2] };
      obj.Transparency =
        std::max(obj.Transparency, static_cast<int>(transparencies[0] * 100.f + 0.5f));
    }
    else if (count > 1)
    {
      obj.FaceColors = std::move(colors);
    }
  }

  //----------------------------------------------------------------------------
  // Some containers, like Arch BuildingPart, store an aggregated copy of the
  // geometry of their children. FreeCAD renders the children only, so when at
  // least one visible child brings its own geometry, the container shape is a
  // duplicate and must not be rendered. This does not affect PartDesign::Body
  // whose children are hidden.
  bool IsRedundantContainerShape(const FCObject& obj)
  {
    if (obj.ShapeFile.empty() || obj.Children.empty())
    {
      return false;
    }
    return this->HasVisibleGeometryInSubtree(obj, 0);
  }

  //----------------------------------------------------------------------------
  // The children geometry can be nested in intermediate groups, e.g. the
  // "Walls" and "Slabs" groups of an Arch BuildingPart floor
  bool HasVisibleGeometryInSubtree(const FCObject& obj, int depth)
  {
    constexpr int maxDepth = 16;
    if (depth >= maxDepth)
    {
      return false;
    }
    for (const std::string& child : obj.Children)
    {
      auto it = this->Objects.find(child);
      if (it == this->Objects.end())
      {
        continue;
      }
      if (!it->second.ShapeFile.empty() && this->IsEffectivelyVisible(it->second))
      {
        return true;
      }
      if (this->HasVisibleGeometryInSubtree(it->second, depth + 1))
      {
        return true;
      }
    }
    return false;
  }

  //----------------------------------------------------------------------------
  bool IsEffectivelyVisible(const FCObject& obj)
  {
    if (!obj.Visible.value_or(true))
    {
      return false;
    }
    // a hidden GeoFeatureGroup ancestor hides its whole subtree
    const FCObject* current = &obj;
    while (!current->Parent.empty())
    {
      auto it = this->Objects.find(current->Parent);
      if (it == this->Objects.end())
      {
        break;
      }
      current = &it->second;
      if (current->IsGeoFeatureGroup() && !current->Visible.value_or(true))
      {
        return false;
      }
    }
    return true;
  }

  //----------------------------------------------------------------------------
  gp_Trsf AncestorsPlacement(const FCObject& obj)
  {
    gp_Trsf trsf;
    const FCObject* current = &obj;
    while (!current->Parent.empty())
    {
      auto it = this->Objects.find(current->Parent);
      if (it == this->Objects.end())
      {
        break;
      }
      current = &it->second;
      if (current->IsGeoFeatureGroup())
      {
        trsf = current->Placement * trsf;
      }
    }
    return trsf;
  }

  //----------------------------------------------------------------------------
  std::vector<gp_Trsf> ReadPlacements(const std::string& file, ArchiveReader& archive)
  {
    std::vector<gp_Trsf> placements;
    std::vector<char> data;
    if (!archive.Extract(file, data) || data.size() < sizeof(uint32_t))
    {
      return placements;
    }
    const uint32_t count = ReadLEUint32(data.data());
    constexpr size_t placementSize = 7 * sizeof(double);
    if (data.size() < sizeof(uint32_t) + placementSize * static_cast<size_t>(count))
    {
      return placements;
    }
    placements.reserve(count);
    for (uint32_t i = 0; i < count; i++)
    {
      const char* p = data.data() + sizeof(uint32_t) + placementSize * i;
      const gp_Quaternion quat(ReadLEDouble(p + 3 * sizeof(double)),
        ReadLEDouble(p + 4 * sizeof(double)), ReadLEDouble(p + 5 * sizeof(double)),
        ReadLEDouble(p + 6 * sizeof(double)));
      gp_Trsf trsf;
      if (quat.SquareNorm() > 0.)
      {
        trsf.SetRotation(quat);
      }
      trsf.SetTranslationPart(gp_Vec(
        ReadLEDouble(p), ReadLEDouble(p + sizeof(double)), ReadLEDouble(p + 2 * sizeof(double))));
      placements.emplace_back(trsf);
    }
    return placements;
  }

  //----------------------------------------------------------------------------
  std::optional<TopoDS_Shape> ReadShape(const std::string& file, ArchiveReader& archive)
  {
    auto cached = this->ShapeCache.find(file);
    if (cached != this->ShapeCache.end())
    {
      return cached->second;
    }
    std::vector<char> data;
    if (!archive.Extract(file, data))
    {
      return std::nullopt;
    }
    std::istringstream stream(std::string(data.data(), data.size()));
    TopoDS_Shape shape;
    BRep_Builder builder;
    try
    {
      BRepTools::Read(shape, stream, builder);
    }
    catch (const Standard_Failure&)
    {
      return std::nullopt;
    }
    if (shape.IsNull())
    {
      return std::nullopt;
    }
    this->ShapeCache[file] = shape;
    return shape;
  }

  //----------------------------------------------------------------------------
  vtkSmartPointer<vtkPolyData> CreatePolyData(const TopoDS_Shape& shape, const FCObject& obj)
  {
    vtkNew<vtkPoints> points;
    points->SetDataTypeToDouble();
    vtkNew<vtkFloatArray> normals;
    normals->SetNumberOfComponents(3);
    normals->SetName("Normal");
    vtkNew<vtkFloatArray> uvs;
    uvs->SetNumberOfComponents(2);
    uvs->SetName("UV");
    vtkNew<vtkUnsignedCharArray> colors;
    colors->SetNumberOfComponents(4);
    colors->SetName("Colors");
    const unsigned char alpha = static_cast<unsigned char>(255 * (100 - obj.Transparency) / 100);
    vtkNew<vtkCellArray> triangles;
    vtkNew<vtkCellArray> lines;

    BRepMesh_IncrementalMesh(shape, this->Parent->GetLinearDeflection(),
      this->Parent->GetRelativeDeflection(), this->Parent->GetAngularDeflection(), true);

    vtkIdType shift = 0;

    if (this->Parent->GetReadWire())
    {
      std::vector<TopoDS_Edge> edges;
      {
        TopoDS_Builder builder;
        TopoDS_Compound compound;
        builder.MakeCompound(compound);
        for (TopExp_Explorer exEdge(shape, TopAbs_EDGE); exEdge.More(); exEdge.Next())
        {
          const TopoDS_Edge edge = TopoDS::Edge(exEdge.Current());
          builder.Add(compound, edge);
          edges.push_back(edge);
        }
        BRepMesh_IncrementalMesh(compound, this->Parent->GetLinearDeflection(),
          this->Parent->GetRelativeDeflection(), this->Parent->GetAngularDeflection(), true);
      }

      constexpr std::array<unsigned char, 4> edgeColor = { 0, 0, 0, 255 };
      for (const TopoDS_Edge& edge : edges)
      {
        TopLoc_Location location;
        const auto& poly = BRep_Tool::Polygon3D(edge, location);
        if (poly.IsNull())
        {
          continue;
        }

        const int nbV = poly->NbNodes();
        const NCollection_Array1<gp_Pnt>& nodes = poly->Nodes();
        for (int i = 1; i <= nbV; i++)
        {
          const gp_Pnt pt = nodes(i).Transformed(location);
          points->InsertNextPoint(pt.X(), pt.Y(), pt.Z());

          // normals and uvs make no sense for lines
          const float fn[3] = { 0.0, 0.0, 1.0 };
          normals->InsertNextTypedTuple(fn);
          uvs->InsertNextTypedTuple(fn);
        }

        std::vector<vtkIdType> polyline(nbV);
        std::iota(polyline.begin(), polyline.end(), shift);
        lines->InsertNextCell(static_cast<vtkIdType>(polyline.size()), polyline.data());
        colors->InsertNextTypedTuple(edgeColor.data());
        shift += nbV;
      }
    }

    TopTools_IndexedMapOfShape faceMap;
    TopExp::MapShapes(shape, TopAbs_FACE, faceMap);

    const bool skipFaces = obj.IsWireframeOnly();
    for (TopExp_Explorer exFace(shape, TopAbs_FACE); !skipFaces && exFace.More(); exFace.Next())
    {
      const TopoDS_Face face = TopoDS::Face(exFace.Current());
      const int faceIndex = faceMap.FindIndex(face) - 1;
      TopLoc_Location location;
      const auto& poly = BRep_Tool::Triangulation(face, location);
      if (poly.IsNull())
      {
        continue;
      }

      Poly::ComputeNormals(poly);
      const TopAbs_Orientation faceOrientation = face.Orientation();
      const gp_Trsf& trsf = location.Transformation();
      const int nbT = poly->NbTriangles();
      const int nbV = poly->NbNodes();

      for (int i = 1; i <= nbV; i++)
      {
        const gp_Pnt pt = poly->Node(i).Transformed(location);
        points->InsertNextPoint(pt.X(), pt.Y(), pt.Z());
      }

      if (poly->HasNormals())
      {
        for (int i = 1; i <= nbV; i++)
        {
          gp_Dir n = poly->Normal(i).Transformed(trsf);
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
        const float fn[3] = { 0.0, 0.0, 1.0 };
        for (int i = 1; i <= nbV; i++)
        {
          normals->InsertNextTypedTuple(fn);
        }
      }

      if (poly->HasUVNodes())
      {
        for (int i = 1; i <= nbV; i++)
        {
          const gp_Pnt2d uv = poly->UVNode(i);
          const float fuv[2] = { static_cast<float>(uv.X()), static_cast<float>(uv.Y()) };
          uvs->InsertNextTypedTuple(fuv);
        }
      }
      else
      {
        for (int i = 1; i <= nbV; i++)
        {
          const float fuv[2] = { 0.f, 0.f };
          uvs->InsertNextTypedTuple(fuv);
        }
      }

      std::array<unsigned char, 4> rgb;
      if (faceIndex >= 0 && static_cast<size_t>(faceIndex) < obj.FaceColors.size())
      {
        rgb = obj.FaceColors[faceIndex];
        rgb[3] = std::min(rgb[3], alpha);
      }
      else
      {
        const std::array<unsigned char, 3> objColor = obj.Color.value_or(DEFAULT_COLOR);
        rgb = { objColor[0], objColor[1], objColor[2], alpha };
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
        triangles->InsertNextCell(3, cell);
        colors->InsertNextTypedTuple(rgb.data());
      }

      shift += nbV;
    }

    vtkNew<vtkPolyData> polydata;
    polydata->SetPoints(points);
    polydata->GetPointData()->SetNormals(normals);
    polydata->GetPointData()->SetTCoords(uvs);
    polydata->SetLines(lines);
    polydata->SetPolys(triangles);
    polydata->GetCellData()->SetScalars(colors);
    return polydata;
  }

  //----------------------------------------------------------------------------
  bool BuildOutput(ArchiveReader& archive, vtkPartitionedDataSetCollection* output)
  {
    vtkNew<vtkDataAssembly> assembly;
    assembly->SetRootNodeName("FCStd");
    unsigned int index = 0;

    for (const std::string& name : this->ObjectOrder)
    {
      FCObject& obj = this->Objects[name];
      if (!this->IsEffectivelyVisible(obj))
      {
        continue;
      }
      if (this->IsRedundantContainerShape(obj))
      {
        continue;
      }

      const FCObject* shapeOwner = &obj;
      TopoDS_Shape placedShape;
      FCObject arrayStyle;

      // link arrays (Draft arrays, App::Link with an element count) do not
      // store their shape: the base object is instanced at the stored
      // placements
      const std::string& arrayBase = obj.IsLink() ? obj.LinkedObject : obj.BaseLink;
      std::vector<gp_Trsf> arrayPlacements;
      if (obj.ShapeFile.empty() && !obj.LinkedExternal && !arrayBase.empty() &&
        !obj.PlacementListFile.empty())
      {
        arrayPlacements = this->ReadPlacements(obj.PlacementListFile, archive);
      }

      if (!arrayPlacements.empty())
      {
        auto targetIt = this->Objects.find(arrayBase);
        if (targetIt == this->Objects.end() || targetIt->second.ShapeFile.empty())
        {
          continue;
        }
        std::optional<TopoDS_Shape> baseShape =
          this->ReadShape(targetIt->second.ShapeFile, archive);
        if (!baseShape.has_value() || baseShape->IsNull())
        {
          continue;
        }
        TopoDS_Builder builder;
        TopoDS_Compound compound;
        builder.MakeCompound(compound);
        for (const gp_Trsf& placement : arrayPlacements)
        {
          gp_Trsf location = obj.Placement * placement;
          if (obj.LinkTransform)
          {
            location = location * targetIt->second.Placement;
          }
          TopoDS_Shape instance = *baseShape;
          instance.Location(TopLoc_Location(location));
          builder.Add(compound, instance);
        }
        placedShape = compound;
        arrayStyle = obj;
        arrayStyle.FaceColors.clear();
        if (!arrayStyle.Color.has_value())
        {
          arrayStyle.Color = targetIt->second.Color;
        }
        if (arrayStyle.Transparency == 0)
        {
          arrayStyle.Transparency = targetIt->second.Transparency;
        }
        shapeOwner = &arrayStyle;
      }
      else
      {
        gp_Trsf overriddenLocation;
        bool overrideLocation = false;
        if (obj.IsLink())
        {
          if (obj.LinkedExternal)
          {
            vtkWarningWithObjectMacro(
              this->Parent, "Link to external document is not supported: " << obj.Label);
            continue;
          }
          auto targetIt = this->Objects.find(obj.LinkedObject);
          if (targetIt == this->Objects.end() || targetIt->second.ShapeFile.empty())
          {
            continue;
          }
          shapeOwner = &targetIt->second;
          // the stored shape of the target has the target placement baked in
          // its top location; a link replaces it with its own placement,
          // composed with the target one when LinkTransform is enabled
          overriddenLocation =
            obj.LinkTransform ? obj.LinkPlacement * shapeOwner->Placement : obj.LinkPlacement;
          overrideLocation = true;
        }

        if (shapeOwner->ShapeFile.empty())
        {
          continue;
        }
        std::optional<TopoDS_Shape> shape = this->ReadShape(shapeOwner->ShapeFile, archive);
        if (!shape.has_value() || shape->IsNull())
        {
          continue;
        }

        placedShape = *shape;
        if (overrideLocation)
        {
          placedShape.Location(TopLoc_Location(overriddenLocation));
        }
      }
      const gp_Trsf ancestors = this->AncestorsPlacement(obj);
      if (ancestors.Form() != gp_Identity)
      {
        placedShape.Move(TopLoc_Location(ancestors));
      }

      vtkSmartPointer<vtkPolyData> polydata;
      try
      {
        polydata = this->CreatePolyData(placedShape, *shapeOwner);
      }
      catch (const Standard_Failure&)
      {
        vtkWarningWithObjectMacro(this->Parent, "Failed to mesh object: " << obj.Label);
        continue;
      }
      if (polydata->GetNumberOfCells() == 0)
      {
        continue;
      }

      output->SetPartition(index, 0, polydata);
      output->GetMetaData(index)->Set(vtkCompositeDataSet::NAME(), obj.Label);
      const int node =
        assembly->AddNode(vtkDataAssembly::MakeValidNodeName(obj.Label.c_str()).c_str());
      assembly->SetAttribute(node, "label", obj.Label.c_str());
      assembly->AddDataSetIndex(node, index);
      index++;
    }

    output->SetDataAssembly(assembly);
    return index > 0;
  }

  bool LegacyAlpha = true;
  std::map<std::string, FCObject> Objects;
  std::vector<std::string> ObjectOrder;
  std::map<std::string, TopoDS_Shape> ShapeCache;
  vtkF3DFCStdReader* Parent;
};

vtkStandardNewMacro(vtkF3DFCStdReader);

//----------------------------------------------------------------------------
vtkF3DFCStdReader::vtkF3DFCStdReader()
  : Internals(new vtkF3DFCStdReader::vtkInternals(this))
{
  this->vtkAlgorithm::SetNumberOfInputPorts(0);
}

//----------------------------------------------------------------------------
vtkF3DFCStdReader::~vtkF3DFCStdReader() = default;

//----------------------------------------------------------------------------
void vtkF3DFCStdReader::SetStream(vtkResourceStream* stream)
{
  if (this->Stream != stream)
  {
    this->Stream = stream;
    this->Modified();
  }
}

//----------------------------------------------------------------------------
vtkResourceStream* vtkF3DFCStdReader::GetStream()
{
  return this->Stream;
}

//----------------------------------------------------------------------------
vtkMTimeType vtkF3DFCStdReader::GetMTime()
{
  vtkMTimeType mtime = this->Superclass::GetMTime();
  if (this->Stream)
  {
    mtime = std::max(mtime, this->Stream->GetMTime());
  }
  return mtime;
}

//----------------------------------------------------------------------------
bool vtkF3DFCStdReader::CanReadFile(vtkResourceStream* stream)
{
  if (!stream || !stream->SupportSeek())
  {
    return false;
  }
  stream->Seek(0, vtkResourceStream::SeekDirection::Begin);
  unsigned char magic[4];
  if (stream->Read(magic, sizeof(magic)) != sizeof(magic) || magic[0] != 'P' || magic[1] != 'K' ||
    magic[2] != 0x03 || magic[3] != 0x04)
  {
    return false;
  }
  ArchiveReader archive;
  return archive.Open(stream) && archive.Has("Document.xml");
}

//----------------------------------------------------------------------------
int vtkF3DFCStdReader::RequestData(
  vtkInformation*, vtkInformationVector**, vtkInformationVector* outputVector)
{
  vtkPartitionedDataSetCollection* output = vtkPartitionedDataSetCollection::GetData(outputVector);

  vtkSmartPointer<vtkResourceStream> stream = this->Stream;
  if (!stream)
  {
    vtkNew<vtkFileResourceStream> fileStream;
    if (this->FileName.empty() || !fileStream->Open(this->FileName.c_str()))
    {
      vtkErrorMacro("Cannot open file: " << this->FileName);
      return 0;
    }
    stream = fileStream;
  }

  ArchiveReader archive;
  if (!archive.Open(stream))
  {
    vtkErrorMacro("Cannot open FCStd archive");
    return 0;
  }

  this->Internals = std::make_unique<vtkInternals>(this);

  std::vector<char> document;
  if (!archive.Extract("Document.xml", document) || !this->Internals->ParseDocument(document))
  {
    vtkErrorMacro("Cannot parse Document.xml");
    return 0;
  }

  std::vector<char> guiDocument;
  if (archive.Extract("GuiDocument.xml", guiDocument))
  {
    this->Internals->ParseGuiDocument(guiDocument, archive);
  }

  if (!this->Internals->BuildOutput(archive, output))
  {
    vtkWarningMacro("No visible geometry found in FCStd file");
  }
  return 1;
}

//----------------------------------------------------------------------------
void vtkF3DFCStdReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << "\n";
  os << indent << "LinearDeflection: " << this->LinearDeflection << "\n";
  os << indent << "AngularDeflection: " << this->AngularDeflection << "\n";
  os << indent << "RelativeDeflection: " << this->RelativeDeflection << "\n";
  os << indent << "ReadWire: " << this->ReadWire << "\n";
}
