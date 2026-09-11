#include "vtkF3DFCStdReader.h"

#include "F3DOCCTPolyData.h"
#include "vtkF3DArchiveReader.h"

#include <vtkByteSwap.h>
#include <vtkCompositeDataSet.h>
#include <vtkDataAssembly.h>
#include <vtkFileResourceStream.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPartitionedDataSet.h>
#include <vtkPartitionedDataSetCollection.h>
#include <vtkPolyData.h>
#include <vtkResourceStream.h>
#include <vtkXMLDataElement.h>
#include <vtkXMLDataParser.h>

#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <Standard_Failure.hxx>
#include <TopExp.hxx>
#include <TopLoc_Location.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS_Builder.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Quaternion.hxx>
#include <gp_Trsf.hxx>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <map>
#include <optional>
#include <sstream>
#include <vector>

namespace
{
constexpr std::array<unsigned char, 3> DEFAULT_COLOR = { 204, 204, 204 };
// guard against cyclic containers when walking the FreeCAD tree
constexpr int MAX_HIERARCHY_DEPTH = 16;

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
  vtkByteSwap::Swap4LE(&value);
  return value;
}

//----------------------------------------------------------------------------
double ReadLEDouble(const char* data)
{
  double value;
  std::memcpy(&value, data, sizeof(double));
  vtkByteSwap::Swap8LE(&value);
  return value;
}

//----------------------------------------------------------------------------
// FreeCAD packs colors as 0xRRGGBBAA. The last byte is ignored: the opacity of an object
// comes from its Transparency property and the opacity of a face from the color lists.
std::array<unsigned char, 3> UnpackRGB(uint32_t packed)
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
  void ParseGuiDocument(const std::vector<char>& content, vtkF3DArchiveReader& archive)
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
  void ParseViewProviderProperties(
    vtkXMLDataElement* props, FCObject& obj, vtkF3DArchiveReader& archive)
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
          obj.Color = UnpackRGB(static_cast<uint32_t>(packed));
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
  void ReadColorList(const std::string& file, vtkF3DArchiveReader& archive, FCObject& obj)
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
      obj.Color = UnpackRGB(ReadLEUint32(data.data() + sizeof(uint32_t)));
    }
    else
    {
      obj.FaceColors.resize(count, { 0, 0, 0, 255 });
      for (uint32_t i = 0; i < count; i++)
      {
        const uint32_t packed = ReadLEUint32(data.data() + sizeof(uint32_t) * (1 + i));
        const std::array<unsigned char, 3> rgb = UnpackRGB(packed);
        const unsigned char lastByte = static_cast<unsigned char>(packed & 0xFF);
        const unsigned char alpha =
          this->LegacyAlpha ? static_cast<unsigned char>(255 - lastByte) : lastByte;
        obj.FaceColors[i] = { rgb[0], rgb[1], rgb[2], alpha };
      }
    }
  }

  //----------------------------------------------------------------------------
  void ReadMaterialList(const std::string& file, vtkF3DArchiveReader& archive, FCObject& obj)
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
      const std::array<unsigned char, 3> rgb = UnpackRGB(ReadLEUint32(material + sizeof(uint32_t)));
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
    if (depth >= MAX_HIERARCHY_DEPTH)
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
  std::vector<gp_Trsf> ReadPlacements(const std::string& file, vtkF3DArchiveReader& archive)
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
  std::optional<TopoDS_Shape> ReadShape(const std::string& file, vtkF3DArchiveReader& archive)
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
    F3DOCCTPolyData::MeshingOptions options;
    options.LinearDeflection = this->Parent->GetLinearDeflection();
    options.AngularDeflection = this->Parent->GetAngularDeflection();
    options.RelativeDeflection = this->Parent->GetRelativeDeflection();
    options.ReadWire = this->Parent->GetReadWire();
    options.ReadFaces = !obj.IsWireframeOnly();

    TopTools_IndexedMapOfShape faceMap;
    TopExp::MapShapes(shape, TopAbs_FACE, faceMap);
    const unsigned char alpha = static_cast<unsigned char>(255 * (100 - obj.Transparency) / 100);

    F3DOCCTPolyData::ColorProviders colors;
    colors.Face = [&faceMap, &obj, alpha](const TopoDS_Face& face)
    {
      const int faceIndex = faceMap.FindIndex(face) - 1;
      if (faceIndex >= 0 && static_cast<size_t>(faceIndex) < obj.FaceColors.size())
      {
        F3DOCCTPolyData::Color rgba = obj.FaceColors[faceIndex];
        rgba[3] = std::min(rgba[3], alpha);
        return rgba;
      }
      const std::array<unsigned char, 3> objColor = obj.Color.value_or(DEFAULT_COLOR);
      return F3DOCCTPolyData::Color{ objColor[0], objColor[1], objColor[2], alpha };
    };
    return F3DOCCTPolyData::Create(shape, options, colors);
  }

  //----------------------------------------------------------------------------
  int AddAssemblyNode(vtkDataAssembly* assembly, const std::string& name,
    std::map<std::string, int>& nodes, int depth = 0)
  {
    auto existing = nodes.find(name);
    if (existing != nodes.end())
    {
      return existing->second;
    }
    const FCObject& obj = this->Objects[name];
    int parent = assembly->GetRootNode();
    if (!obj.Parent.empty() && depth < MAX_HIERARCHY_DEPTH && this->Objects.count(obj.Parent) > 0)
    {
      parent = this->AddAssemblyNode(assembly, obj.Parent, nodes, depth + 1);
    }
    const int node =
      assembly->AddNode(vtkDataAssembly::MakeValidNodeName(obj.Label.c_str()).c_str(), parent);
    assembly->SetAttribute(node, "label", obj.Label.c_str());
    nodes[name] = node;
    return node;
  }

  //----------------------------------------------------------------------------
  bool BuildOutput(vtkF3DArchiveReader& archive, vtkPartitionedDataSetCollection* output)
  {
    vtkNew<vtkDataAssembly> assembly;
    assembly->SetRootNodeName("FCStd");
    std::map<std::string, int> nodes;
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
      assembly->AddDataSetIndex(this->AddAssemblyNode(assembly, name, nodes), index);
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
  vtkNew<vtkF3DArchiveReader> archive;
  return archive->Open(stream) && archive->Has("Document.xml");
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

  vtkNew<vtkF3DArchiveReader> archive;
  if (!archive->Open(stream))
  {
    vtkErrorMacro("Cannot open FCStd archive");
    return 0;
  }

  this->Internals = std::make_unique<vtkInternals>(this);

  std::vector<char> document;
  if (!archive->Extract("Document.xml", document) || !this->Internals->ParseDocument(document))
  {
    vtkErrorMacro("Cannot parse Document.xml");
    return 0;
  }

  std::vector<char> guiDocument;
  if (archive->Extract("GuiDocument.xml", guiDocument))
  {
    this->Internals->ParseGuiDocument(guiDocument, *archive);
  }

  if (!this->Internals->BuildOutput(*archive, output))
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
