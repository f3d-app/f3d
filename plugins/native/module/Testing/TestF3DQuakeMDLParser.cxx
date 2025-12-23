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

// .mdl file with no data
std::vector<uint8_t> testNoData = {};

// .mdl file with only header (but incorrect magic number)
std::vector<uint8_t> testNonmagicHeader = { 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0xe3,
  0x62, 0xb2, 0x3d, 0xa4, 0xb3, 0x14, 0x3e, 0xc2, 0x27, 0x0e, 0x3e, 0x7e, 0xcd, 0x26, 0xc1, 0xc6,
  0x18, 0x94, 0xc1, 0x00, 0x00, 0x00, 0x00, 0xe6, 0x9c, 0x26, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0xc8, 0x00, 0x00, 0x00, 0xc8,
  0x00, 0x00, 0x00, 0x8d, 0x00, 0x00, 0x00, 0x94, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x6e, 0x8d, 0x8d, 0x41 };

// .mdl file with only header (correct magic number, but no body data)
std::vector<uint8_t> testNormalHeader = { 0x49, 0x44, 0x50, 0x4f, 0x06, 0x00, 0x00, 0x00, 0xe3,
  0x62, 0xb2, 0x3d, 0xa4, 0xb3, 0x14, 0x3e, 0xc2, 0x27, 0x0e, 0x3e, 0x7e, 0xcd, 0x26, 0xc1, 0xc6,
  0x18, 0x94, 0xc1, 0x00, 0x00, 0x00, 0x00, 0xe6, 0x9c, 0x26, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0xc8, 0x00, 0x00, 0x00, 0xc8,
  0x00, 0x00, 0x00, 0x8d, 0x00, 0x00, 0x00, 0x94, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x6e, 0x8d, 0x8d, 0x41 };

// .mdl file with MAXED data i.e.
// max numSkins, skinWidth, skinHeight, numVertices, numTriangles, numFrames, size
std::vector<uint8_t> testMaxedHeader = { 0x49, 0x44, 0x50, 0x4f, 0x6, 0x0, 0x0, 0x0, 0xe3, 0x62,
  0xb2, 0x3d, 0xa4, 0xb3, 0x14, 0x3e, 0xc2, 0x27, 0xe, 0x3e, 0x7e, 0xcd, 0x26, 0xc1, 0xc6, 0x18,
  0x94, 0xc1, 0x0, 0x0, 0x0, 0x0, 0xe6, 0x9c, 0x26, 0x42, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0x7f, 0xff,
  0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0x7f, 0x0, 0x0, 0x0, 0x0, 0x8, 0x0,
  0x0, 0x0, 0xff, 0xff, 0x7f, 0x7f };

// .mdl file with MINED data i.e.
// min numSkins, skinWidth, skinHeight, numVertices, numTriangles, numFrames, size
std::vector<uint8_t> testMinedHeader{ 0x49, 0x44, 0x50, 0x4f, 0x6, 0x0, 0x0, 0x0, 0xe3, 0x62, 0xb2,
  0x3d, 0xa4, 0xb3, 0x14, 0x3e, 0xc2, 0x27, 0xe, 0x3e, 0x7e, 0xcd, 0x26, 0xc1, 0xc6, 0x18, 0x94,
  0xc1, 0x0, 0x0, 0x0, 0x0, 0xe6, 0x9c, 0x26, 0x42, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x80, 0x0, 0x0, 0x0, 0x80, 0x0, 0x0, 0x0, 0x80, 0x0, 0x0, 0x0, 0x80,
  0x0, 0x0, 0x0, 0x80, 0x0, 0x0, 0x0, 0x80, 0x0, 0x0, 0x0, 0x0, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x80,
  0x0 };

struct Error
{
  bool parsed = false;
  std::string fileName;
  std::string lineNumber;
  std::string errorString;
};

Error ErrorCodeParser(const std::string& errorStr)
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

    return {
      .parsed = true, .fileName = pFileName, .lineNumber = pLineNumber, .errorString = pErrorString
    };
  }
  return { .parsed = false };
}

void ErrorCallbackFunc(
  vtkObject* caller, long unsigned int vtkNotUsed(eventId), void* clientData, void* callData)
{
  auto errors = static_cast<std::vector<Error>*>(clientData);

  errors->push_back(ErrorCodeParser(static_cast<char*>(callData)));
}

int TestF3DQuakeMDLParser(int vtkNotUsed(argc), char* argv[])
{
  // Setup
  vtkNew<vtkCallbackCommand> errorCallback;
  errorCallback->SetCallback(ErrorCallbackFunc);

  std::string testName = "";

  // Run tests

  // Test "0bytes"
  {
    testName = "0bytes";
    std::cout << "Running test \"" << testName << "\"\n";

    std::vector<Error> errors;
    errorCallback->SetClientData(&errors);

    vtkNew<vtkMemoryResourceStream> stream;
    stream->SetBuffer(testNoData);

    vtkNew<vtkF3DQuakeMDLImporter> importer;
    importer->SetStream(stream);
    importer->AddObserver(vtkCommand::ErrorEvent, errorCallback);

    importer->Update();

    if (errors.size() != 1 || errors[0].errorString != "Unable to read header, aborting.")
    {
      std::cerr << "Test \"" << testName << "\" failed\n";
      return EXIT_FAILURE;
    }
  }

  // Test "non-magic header"
  {
    testName = "non-magic header";
    std::cout << "Running test \"" << testName << "\"\n";

    std::vector<Error> errors;
    errorCallback->SetClientData(&errors);

    vtkNew<vtkMemoryResourceStream> stream;
    stream->SetBuffer(testNonmagicHeader);

    vtkNew<vtkF3DQuakeMDLImporter> importer;
    importer->SetStream(stream);
    importer->AddObserver(vtkCommand::ErrorEvent, errorCallback);

    importer->Update();

    if (errors.size() != 1 || errors[0].errorString != "Header Magic number incorrect, aborting.")
    {
      std::cerr << "Test \"" << testName << "\" failed\n";
      return EXIT_FAILURE;
    }
  }

  // Test "magic header"
  {
    testName = "magic header";
    std::cout << "Running test \"" << testName << "\"\n";

    std::vector<Error> errors;
    errorCallback->SetClientData(&errors);

    vtkNew<vtkMemoryResourceStream> stream;
    stream->SetBuffer(testNormalHeader);

    vtkNew<vtkF3DQuakeMDLImporter> importer;
    importer->SetStream(stream);
    importer->AddObserver(vtkCommand::ErrorEvent, errorCallback);

    importer->Update();

    if (errors.size() != 2 ||
      errors[0].errorString != "CreateTexture Accessed data out of range, aborting." ||
      errors[1].errorString != "Unable to read a texture, aborting.")
    {
      std::cerr << "Test \"" << testName << "\" failed\n";
      return EXIT_FAILURE;
    }
  }

  // Test "maxed header"
  {
    testName = "maxed header";
    std::cout << "Running test \"" << testName << "\"\n";

    std::vector<Error> errors;
    errorCallback->SetClientData(&errors);

    vtkNew<vtkMemoryResourceStream> stream;
    stream->SetBuffer(testMaxedHeader);

    vtkNew<vtkF3DQuakeMDLImporter> importer;
    importer->SetStream(stream);
    importer->AddObserver(vtkCommand::ErrorEvent, errorCallback);

    importer->Update();

    if (errors.size() != 2 ||
      errors[0].errorString != "CreateTexture Accessed data out of range, aborting." ||
      errors[1].errorString != "Unable to read a texture, aborting.")
    {
      std::cerr << "Test \"" << testName << "\" failed\n";
      return EXIT_FAILURE;
    }
  }

  // Test "mined header"
  {
    testName = "mined header";
    std::cout << "Running test \"" << testName << "\"\n";

    std::vector<Error> errors;
    errorCallback->SetClientData(&errors);

    vtkNew<vtkMemoryResourceStream> stream;
    stream->SetBuffer(testMinedHeader);

    vtkNew<vtkF3DQuakeMDLImporter> importer;
    importer->SetStream(stream);
    importer->AddObserver(vtkCommand::ErrorEvent, errorCallback);

    importer->Update();

    if (errors.size() != 2 ||
      errors[0].errorString != "CreateMesh Accessed data out of range, aborting." ||
      errors[1].errorString != "No frame read, there is nothing to display in this file.")
    {
      std::cerr << "Test \"" << testName << "\" failed\n";
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
