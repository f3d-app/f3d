/**
 * @class   NodeInfo
 * @brief   Structure to represent a node in the scene hierarchy
 *
 * NodeInfo contains information about a scene node including its name,
 * associated VTK prop, children nodes, and display name for the UI.
 */

#ifndef F3DNodeInfo_h
#define F3DNodeInfo_h

#include <string>
#include <vector>

class vtkProp;

struct NodeInfo
{
  std::string name;
  vtkProp* prop = nullptr;
  std::vector<NodeInfo> children;
  std::string displayName;
};

#endif // F3DNodeInfo_h
