#include <engine.h>
#include <interactor.h>
#include <loader.h>
#include <log.h>
#include <window.h>

#include <iostream>

int TestSDKLoader(int argc, char* argv[])
{
  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  f3d::engine eng(f3d::window::Type::NONE);
  f3d::loader& load = eng.getLoader();

  std::string filePath, fileInfo;
  int idx;

  // Test empty file list
  std::string empty = "";
  load.addFile(empty);
  load.getFileInfo(f3d::loader::LoadFileEnum::LOAD_CURRENT, idx, filePath, fileInfo);
  if (idx != -1)
  {
    std::cerr << "Unexpected idx with empty filelist: " << idx << std::endl;
    return EXIT_FAILURE;
  }
  if (load.loadFile())
  {
    std::cerr << "Unexpected loaded file with empty filelist" << std::endl;
    return EXIT_FAILURE;
  }

  // Test file list logic
  std::string dummyFilename = "dummy.foo";
  std::string cowFilename = "cow.vtp";
  std::string dragonFilename = "dragon.vtu";
  std::string suzanneFilename = "suzanne.stl";
  std::string dummy = std::string(argv[1]) + "data/" + dummyFilename;
  std::string cow = std::string(argv[1]) + "data/" + cowFilename;
  std::string dragon = std::string(argv[1]) + "data/" + dragonFilename;
  std::string suzanne = std::string(argv[1]) + "data/" + suzanneFilename;
  std::vector<std::string> filesVec{ dragon, suzanne };
  std::vector<std::string> filesVecCheck{ cow, dragon, suzanne };

  load.addFile(dummy).addFile(cow).addFiles(filesVec);
  load.getFileInfo(f3d::loader::LoadFileEnum::LOAD_CURRENT, idx, filePath, fileInfo);
  if (filePath != cow)
  {
    std::cerr << "Unexpected file loaded on LOAD_CURRENT: " << filePath << std::endl;
    return EXIT_FAILURE;
  }
  if (!load.loadFile())
  {
    std::cerr << "Failed to load a file on LOAD_CURRENT: " << filePath << std::endl;
    return EXIT_FAILURE;
  }

  load.loadFile(f3d::loader::LoadFileEnum::LOAD_LAST);
  load.getFileInfo(f3d::loader::LoadFileEnum::LOAD_CURRENT, idx, filePath, fileInfo);
  if (filePath != suzanne)
  {
    std::cerr << "Unexpected file loaded on LOAD_LAST: " << filePath << std::endl;
    return EXIT_FAILURE;
  }

  load.loadFile(f3d::loader::LoadFileEnum::LOAD_FIRST);
  load.getFileInfo(f3d::loader::LoadFileEnum::LOAD_CURRENT, idx, filePath, fileInfo);
  if (filePath != cow)
  {
    std::cerr << "Unexpected file loaded on LOAD_FIRST: " << filePath << std::endl;
    return EXIT_FAILURE;
  }

  load.loadFile(f3d::loader::LoadFileEnum::LOAD_NEXT);
  load.getFileInfo(f3d::loader::LoadFileEnum::LOAD_CURRENT, idx, filePath, fileInfo);
  if (filePath != dragon)
  {
    std::cerr << "Unexpected file loaded on LOAD_NEXT: " << filePath << std::endl;
    return EXIT_FAILURE;
  }

  load.loadFile(f3d::loader::LoadFileEnum::LOAD_NEXT);
  load.getFileInfo(f3d::loader::LoadFileEnum::LOAD_CURRENT, idx, filePath, fileInfo);
  if (filePath != suzanne)
  {
    std::cerr << "Unexpected file loaded on second LOAD_NEXT: " << filePath << std::endl;
    return EXIT_FAILURE;
  }

  if (load.getCurrentFileIndex() != 2)
  {
    std::cerr << "Unexpected file index after LOAD_NEXT: " << load.getCurrentFileIndex()
              << std::endl;
    return EXIT_FAILURE;
  }

  // Check current index
  load.setCurrentFileIndex(1).loadFile();
  load.getFileInfo(f3d::loader::LoadFileEnum::LOAD_CURRENT, idx, filePath, fileInfo);
  if (filePath != dragon)
  {
    std::cerr << "Unexpected file loaded on LOAD_CURRENT after setCurrentFileIndex: " << filePath
              << std::endl;
    return EXIT_FAILURE;
  }

  // Check files vec
  if (load.getFiles() != filesVecCheck)
  {
    std::cerr << "Unexpected getFiles result" << std::endl;
    return EXIT_FAILURE;
  }

  // Check folder non-recursive
  std::string folder = std::string(argv[1]) + "data/mb";
  load.addFile(folder, false);
  if (load.getFiles().size() != 7)
  {
    std::cerr << "Unexpected getFiles result after non-recursive add: " << load.getFiles().size()
              << std::endl;
    return EXIT_FAILURE;
  }

  // Check folder recursive
  load.addFile(folder, true);
  if (load.getFiles().size() != 11)
  {
    std::cerr << "Unexpected getFiles result after recursive add: " << load.getFiles().size()
              << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
