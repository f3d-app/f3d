/**
 * @class   vtkF3DUserEvents
 * @brief   An enum of F3D defined VTK events
 */

#ifndef vtkF3DUserEvents_h
#define vtkF3DUserEvents_h

#include <vtkCommand.h>

/**
 * Custom events
 */
enum vtkF3DUserEvents
{
  DropFilesEvent = vtkCommand::UserEvent + 100,
  KeyPressEvent,
  TriggerEvent,
  ShowEvent,
  HideEvent,
  SceneHierarchyChangedEvent
};

#endif
