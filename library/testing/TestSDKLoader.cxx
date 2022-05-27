#include <engine.h>
#include <interactor.h>
#include <loader.h>
#include <window.h>

#include <iostream>

int TestSDKLoader(int argc, char* argv[])
{
  f3d::engine eng(f3d::engine::FLAGS_NONE);
  f3d::loader& load = eng.getLoader();

  std::string cowFilename = "cow.vtp";
  std::string dragonFilename = "dragon.vtu";
  std::string suzanneFilename = "suzanne.stl";
  std::string cow = std::string(argv[1]) + "data/" + cowFilename;
  std::string dragon = std::string(argv[1]) + "data/" + dragonFilename;
  std::string suzanne = std::string(argv[1]) + "data/" + suzanneFilename;
  std::vector<std::string> filesVec{ dragon, suzanne };
  std::vector<std::string> filesVecCheck{ cow, dragon, suzanne };

  load.addFile(cow);
  load.addFiles(filesVec);

  // Test file list logic
  std::string filePath, fileInfo;
  int idx;

  load.loadFile(f3d::loader::LoadFileEnum::LOAD_CURRENT);
  load.getFileInfo(f3d::loader::LoadFileEnum::LOAD_CURRENT, idx, filePath, fileInfo);
  if (filePath != cow)
  {
    std::cout << "Unexpected file loaded on LOAD_CURRENT: " << filePath << std::endl;
    return EXIT_FAILURE;
  }

  load.loadFile(f3d::loader::LoadFileEnum::LOAD_LAST);
  load.getFileInfo(f3d::loader::LoadFileEnum::LOAD_CURRENT, idx, filePath, fileInfo);
  if (filePath != suzanne)
  {
    std::cout << "Unexpected file loaded on LOAD_LAST: " << filePath << std::endl;
    return EXIT_FAILURE;
  }

  load.loadFile(f3d::loader::LoadFileEnum::LOAD_FIRST);
  load.getFileInfo(f3d::loader::LoadFileEnum::LOAD_CURRENT, idx, filePath, fileInfo);
  if (filePath != cow)
  {
    std::cout << "Unexpected file loaded on LOAD_FIRST: " << filePath << std::endl;
    return EXIT_FAILURE;
  }

  load.loadFile(f3d::loader::LoadFileEnum::LOAD_NEXT);
  load.getFileInfo(f3d::loader::LoadFileEnum::LOAD_CURRENT, idx, filePath, fileInfo);
  if (filePath != dragon)
  {
    std::cout << "Unexpected file loaded on LOAD_NEXT: " << filePath << std::endl;
    return EXIT_FAILURE;
  }

  load.loadFile(f3d::loader::LoadFileEnum::LOAD_NEXT);
  load.getFileInfo(f3d::loader::LoadFileEnum::LOAD_CURRENT, idx, filePath, fileInfo);
  if (filePath != suzanne)
  {
    std::cout << "Unexpected file loaded on second LOAD_NEXT: " << filePath << std::endl;
    return EXIT_FAILURE;
  }

  if (load.getCurrentFileIndex() != 2)
  {
    std::cout << "Unexpected file index after LOAD_NEXT: " << load.getCurrentFileIndex()
              << std::endl;
    return EXIT_FAILURE;
  }

  // Check current index
  load.setCurrentFileIndex(1);
  load.loadFile(f3d::loader::LoadFileEnum::LOAD_CURRENT);
  load.getFileInfo(f3d::loader::LoadFileEnum::LOAD_CURRENT, idx, filePath, fileInfo);
  if (filePath != dragon)
  {
    std::cout << "Unexpected file loaded on LOAD_CURRENT after setCurrentFileIndex: " << filePath
              << std::endl;
    return EXIT_FAILURE;
  }

  // Check files vec
  if (load.getFiles() != filesVecCheck)
  {
    std::cout << "Unexpected getFiles result" << std::endl;
    return EXIT_FAILURE;
  }

  // Check folder non-recursive
  std::string folder = std::string(argv[1]) + "data/mb";
  load.addFile(folder, false);
  if (load.getFiles().size() != 7)
  {
    std::cout << "Unexpected getFiles result after non-recursive add: " << load.getFiles().size()
              << std::endl;
    return EXIT_FAILURE;
  }

  // Check folder recursive
  load.addFile(folder, true);
  if (load.getFiles().size() != 11)
  {
    std::cout << "Unexpected getFiles result after recursive add: " << load.getFiles().size()
              << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
