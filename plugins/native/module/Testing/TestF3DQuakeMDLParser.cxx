#include <climits>
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

// Header definition : cloned from plugins/native/module/vtkF3DQuakeMDLImporter.cxx
struct mdl_header_t
{
  int IDPO;
  int version;
  float scale[3];
  float translation[3];
  float boundingRadius;
  float eyePosition[3];
  int numSkins;
  int skinWidth;
  int skinHeight;
  int numVertices;
  int numTriangles;
  int numFrames;
  int syncType;
  int stateFlags;
  float size;
};

// Buffer typecasting helper
template<typename TYPE>
static TYPE* PeekFromVector(std::vector<uint8_t>& buffer, const size_t& offset)
{
  static_assert(std::is_pod<TYPE>::value, "Vector typecast requires POD input");

  if (offset + sizeof(TYPE) > buffer.size())
  {
    return nullptr;
  }

  return reinterpret_cast<TYPE*>(buffer.data() + offset);
}

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
  // Read in example file, can theoretically be any mdl file
  std::string path = std::string(argv[1]) + "data/armor.mdl";
  std::ifstream file(path, std::ios::binary);
  std::vector<uint8_t> defaultFile(
    (std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

  // setup vectors
  // .mdl file with no data
  std::vector<uint8_t> testNoData = {};
  std::vector<uint8_t> testNonmagicHeader = {};
  std::vector<uint8_t> testNormalHeader = {};
  std::vector<uint8_t> testMaxedHeader = {};
  std::vector<uint8_t> testMinedHeader = {};

  // .mdl file with only header (but incorrect magic number)
  {
    testNonmagicHeader = defaultFile;
    // modify header
    auto header = PeekFromVector<mdl_header_t>(testNonmagicHeader, 0);
    header->IDPO = 0;
  }

  // .mdl file with only header (correct magic number, but no body data)
  {
    testNormalHeader = defaultFile;
    // modify header
    // no need, default file should have valid IPDO
  }

  // .mdl file with MAXED data
  {
    testMaxedHeader = defaultFile;
    // modify header
    // max numSkins, skinWidth, skinHeight, numVertices, numTriangles, numFrames, size
    auto header = PeekFromVector<mdl_header_t>(testMaxedHeader, 0);
    header->numSkins = INT_MAX;
    header->skinWidth = INT_MAX;
    header->skinHeight = INT_MAX;
    header->numVertices = INT_MAX;
    header->numTriangles = INT_MAX;
    header->numFrames = INT_MAX;
  }

  // .mdl file with MINED data
  {
    testMinedHeader = defaultFile;
    // modify header
    // max numSkins, skinWidth, skinHeight, numVertices, numTriangles, numFrames, size
    auto header = PeekFromVector<mdl_header_t>(testMinedHeader, 0);
    header->numSkins = INT_MIN;
    header->skinWidth = INT_MIN;
    header->skinHeight = INT_MIN;
    header->numVertices = INT_MIN;
    header->numTriangles = INT_MIN;
    header->numFrames = INT_MIN;
  }

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

    if (errors.size() != 0)
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
