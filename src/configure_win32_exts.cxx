/*
 * Generates the registration/unregistration NSIS scripts for
 * the management of every available file extensions of F3D
 */
#include "F3DReaderFactory.h"
#include "F3DReaderInstantiator.h"

#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char** argv)
{
  if (argc < 3)
  {
    return EXIT_FAILURE;
  }
  std::ofstream registerFile, unregisterFile, extensionsFile;
  registerFile.open(argv[1]);
  unregisterFile.open(argv[2]);
  if (argc > 3)
  {
    extensionsFile.open(argv[3]);
    extensionsFile << "#pragma once\n\nconst std::vector<std::wstring> extensions =\n{\n";
  }

  if (!registerFile.is_open() || !unregisterFile.is_open())
  {
    std::cerr << "Error! Unable to create nsh script files!" << std::endl;
    return EXIT_FAILURE;
  }

  F3DReaderInstantiator instantiator;

  auto readers = F3DReaderFactory::GetInstance()->GetReaders();
  for (const auto reader : readers)
  {
    for (const auto& ext : reader->GetExtensions())
    {
      // ${RegisterExtension} '$INSTDIR\bin\f3d.exe' '.vtk' 'VTK Legacy Data Format'
      registerFile << "${RegisterExtension} '$INSTDIR\\bin\\f3d.exe' '" << ext << "' '"
                   << reader->GetLongDescription() << "'\n";

      // ${UnRegisterExtension} '.vtk' 'VTK Legacy Data Format'
      unregisterFile << "${UnRegisterExtension} '" << ext << "' '" << reader->GetLongDescription()
                     << "'\n";

      if (extensionsFile.is_open())
      {
        // L".vtk",
        extensionsFile << "  L\"" << ext << "\",\n";
      }
    }
  }

  registerFile.close();
  unregisterFile.close();
  if (extensionsFile.is_open())
  {
    extensionsFile << "};\n";
    extensionsFile.close();
  }

  return (readers.size() != 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
