/*
 * Generates the mime types list for
 * desktop integration in linux
 */
#include "F3DReaderFactory.h"
#include "F3DReaderInstantiator.h"

#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char** argv)
{
  if (argc < 1)
  {
    return EXIT_FAILURE;
  }
  std::ofstream mimeTypesFile;
  mimeTypesFile.open(argv[1]);

  if (!mimeTypesFile.is_open())
  {
    std::cerr << "Error! Unable to create mime types files!" << std::endl;
    return EXIT_FAILURE;
  }

  F3DReaderInstantiator instantiator;

  auto readers = F3DReaderFactory::GetInstance()->GetReaders();
  for (const auto reader : readers)
  {
    for (const auto& mimeType : reader->GetMimeTypes())
    {
      mimeTypesFile << mimeType << ";";
    }
  }

  mimeTypesFile.close();
  return (readers.size() != 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
