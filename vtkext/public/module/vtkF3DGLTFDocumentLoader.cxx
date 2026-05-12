#include "vtkF3DGLTFDocumentLoader.h"

#include <vtkObjectFactory.h>
#include <vtkResourceStream.h>

#include <nlohmann/json.hpp>

#include <sstream>

vtkStandardNewMacro(vtkF3DGLTFDocumentLoader);

//----------------------------------------------------------------------------
std::vector<std::string> vtkF3DGLTFDocumentLoader::GetSupportedExtensions()
{
  std::vector<std::string> extensions = this->Superclass::GetSupportedExtensions();
  extensions.emplace_back("EXT_texture_webp");
  return extensions;
}

//----------------------------------------------------------------------------
void vtkF3DGLTFDocumentLoader::PrepareData()
{
  this->Superclass::PrepareData();

  // Read the raw GLTF JSON to extract EXT_texture_webp source overrides
  std::shared_ptr<Model> model = this->GetInternalModel();
  if (!model || !model->Stream)
  {
    return;
  }

  // Seek to start and read entire stream
  model->Stream->Seek(0, vtkResourceStream::SeekDirection::Begin);
  std::string content;
  char buf[4096];
  while (true)
  {
    vtkTypeInt64 n = model->Stream->Read(buf, sizeof(buf));
    if (n <= 0)
      break;
    content.append(buf, static_cast<size_t>(n));
  }

  // For GLB files, JSON starts after the 12-byte header + chunk header (8 bytes)
  // For GLTF files, content is plain JSON
  size_t jsonStart = 0;
  if (content.size() >= 4 &&
      content[0] == 'g' && content[1] == 'l' &&
      content[2] == 'T' && content[3] == 'F')
  {
    // GLB: skip 12-byte header + 4-byte chunk length + 4-byte chunk type
    if (content.size() > 20)
      jsonStart = 20;
    else
      return;
  }

  try
  {
    auto root = nlohmann::json::parse(content.begin() + jsonStart, content.end(),
      nullptr, false);
    if (!root.contains("textures"))
      return;

    const auto& textures = root["textures"];
    for (size_t i = 0; i < textures.size() && i < model->Textures.size(); ++i)
    {
      const auto& tex = textures[i];
      if (tex.contains("extensions") &&
          tex["extensions"].contains("EXT_texture_webp"))
      {
        const auto& webpExt = tex["extensions"]["EXT_texture_webp"];
        if (webpExt.contains("source"))
        {
          model->Textures[i].Source = webpExt["source"].get<int>();
        }
      }
    }
  }
  catch (const nlohmann::json::exception&)
  {
    // Malformed JSON — leave textures unchanged
  }
}