/**
 * @class   F3DLoader
 * @brief   The loader class
 *
 */

#ifndef F3DLoader_h
#define F3DLoader_h

#include <vtkImporter.h>
#include <vtkNew.h>
#include <vtkProgressBarRepresentation.h>
#include <vtkProgressBarWidget.h>
#include <vtkSmartPointer.h>

#include "vtkF3DRenderer.h"

struct F3DOptions;

class F3DLoader
{
public:
  static F3DLoader& GetInstance();

  void AddFiles(const std::vector<std::string>& files);
  void AddFile(const std::string& path, bool recursive = true);

  void LoadPrevious(vtkF3DRenderer* ren);
  void LoadNext(vtkF3DRenderer* ren);
  void LoadCurrentIndex(vtkF3DRenderer* ren);

protected:
  vtkSmartPointer<vtkImporter> GetImporter(const F3DOptions& options, const std::string& file);

  std::vector<std::string> FilesList;
  size_t CurrentFileIndex = 0;

private:
  F3DLoader();
  ~F3DLoader();

  F3DLoader(F3DLoader const&) = delete;
  void operator=(F3DLoader const&) = delete;
};

#endif
