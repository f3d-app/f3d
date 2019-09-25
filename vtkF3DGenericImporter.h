/*=========================================================================
  Program:   Visualization Toolkit
  Module:    vtkF3DGenericImporter.h
  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/
/**
 * @class   vtkF3DGenericImporter
 * @brief   create a scene from the meta reader
 */

#ifndef vtkF3DGenericImporter_h
#define vtkF3DGenericImporter_h

#include <vtkImporter.h>

#include "vtkF3DMetaReader.h"

class vtkF3DGenericImporter : public vtkImporter
{
public:
  static vtkF3DGenericImporter* New();

  vtkTypeMacro(vtkF3DGenericImporter, vtkImporter);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /**
   * Specify the name of the file to read.
   */
  void SetFileName(const char* arg);

protected:
  vtkF3DGenericImporter() = default;
  ~vtkF3DGenericImporter() override = default;

  void ImportActors(vtkRenderer*) override;
  void ImportLights(vtkRenderer*) override;
  void ImportProperties(vtkRenderer*) override;

  vtkNew<vtkF3DMetaReader> Reader;

private:
  vtkF3DGenericImporter(const vtkF3DGenericImporter&) = delete;
  void operator=(const vtkF3DGenericImporter&) = delete;
};

#endif
