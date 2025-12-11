#include <regex> // slow, but thats ok for a test
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkDataSet.h>
#include <vtkMapper.h>
#include <vtkMemoryResourceStream.h>
#include <vtkNew.h>
#include <vtkRenderer.h>
#include <vtkTestUtilities.h>

#include "vtkF3DQuakeMDLImporter.h"

#include <iostream>

struct ExpectedError
{
  const std::string fileName;
  const std::string lineNumber;
  const std::string errorString;
  bool correctResult = true;
};

bool ErrorCodeParser(const std::string& errorStr, const std::string& fileName,
  const std::string& lineNumber, const std::string& errorString)
{
  std::regex errorRegex(R"(ERROR: In (.+?), line (\d+)\n.+: (.+))");
  std::smatch matches;
  if (std::regex_search(errorStr, matches, errorRegex))
  {
    std::string pFileName = matches[1].str();
    std::string pLineNumber = matches[2].str(); // Convert string to int
    std::string pErrorString = matches[3].str();

    printf(
      "Parsed error\n    file name : \"%s\"\n    line number : \"%s\"\n    error string : \"%s\"\n",
      pFileName.c_str(), pLineNumber.c_str(), pErrorString.c_str());

    if (pFileName != fileName)
    {
      std::cout << "File name does not match! \"" << fileName << "\"\n";
      return false;
    }
    if (pLineNumber != lineNumber)
    {
      std::cout << "Line number mismatch! \"" << lineNumber << "\"\n";
      return false;
    }
    if (pErrorString != errorString)
    {
      std::cout << "Error string mismatch! \"" << errorString << "\"\n";
      return false;
    }

    return true;
  }
  std::cout << "no match results for \"" << errorStr << "\"";
  return false;
}

void ErrorCallbackFunc(
  vtkObject* caller, long unsigned int vtkNotUsed(eventId), void* clientData, void* callData)
{
  auto expected = static_cast<ExpectedError*>(clientData);
  expected->correctResult = ErrorCodeParser(
    static_cast<char*>(callData), expected->fileName, expected->lineNumber, expected->errorString);
}

int TestF3DQuakeMDLParser(int vtkNotUsed(argc), char* argv[])
{
  // Setup
  vtkNew<vtkCallbackCommand> errorCallback;
  errorCallback->SetCallback(ErrorCallbackFunc);

  // Test binary data
  std::vector<uint8_t> test_0bytes = {};

  // Run tests
  {
    // Test "0bytes"
    // 0 byte .mdl file

    // Error data we expect to recive from calling importer on this "file"
    ExpectedError expected{ .fileName = "vtkF3DQuakeMDLImporter.cxx",
      .lineNumber = "573",
      .errorString = "Unable to read header, aborting." };

    errorCallback->SetClientData(&expected);

    vtkNew<vtkMemoryResourceStream> stream;
    stream->SetBuffer(test_0bytes);

    vtkNew<vtkF3DQuakeMDLImporter> importer;
    importer->SetStream(stream);
    importer->AddObserver(vtkCommand::ErrorEvent, errorCallback);

    importer->Update();

    if (!expected.correctResult)
    {
      std::cerr << "Test failed testing 0byte file\n";
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
