/**
 * @class   F3DNodeInfo
 * @brief   Structure to represent a node in the scene hierarchy
 *
 * F3DNodeInfo contains information about a scene node including its name,
 * associated VTK prop, children nodes, and display name for the UI.
 */

#ifndef F3DNodeInfo_h
#define F3DNodeInfo_h

#include <string>
#include <vector>

class vtkProp;

struct F3DNodeInfo
{
  std::string name;
  vtkProp* prop = nullptr;
  std::vector<F3DNodeInfo> children;
  std::string displayName;
};

#endif // F3DNodeInfo_h
