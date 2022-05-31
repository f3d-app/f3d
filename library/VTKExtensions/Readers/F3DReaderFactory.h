/**
 * @class   F3DReaderFactory
 * @brief   The factory that manages the reader class instances
 *
 * This class describes the singleton object that manages the declared readers
 * in F3D.
 * To make it simple and automatic, you just have to declare each reader class
 * with the CMake macro declare_f3d_reader(). Then, at configure time, CMake
 * generates a cxx file that instantiates all the reader classes and registers
 * them to the factory.
 */

#ifndef F3DReaderFactory_h
#define F3DReaderFactory_h

#include "F3DReader.h"

#include <list>

class F3DReaderFactory
{
public:
  static F3DReaderFactory* GetInstance();

  /*
   * Register a reader class to the factory
   */
  static void Register(F3DReader*);

  /*
   * Unregister a reader class from the factory
   */
  static void Unregister(F3DReader*);

  /*
   * Get the reader that can read the given file, nullptr if none
   */
  static F3DReader* GetReader(const std::string& fileName);

  /*
   * Get the list of the registered reader classes
   */
  static const std::list<F3DReader*>& GetReaders();

protected:
  F3DReaderFactory() = default;
  virtual ~F3DReaderFactory() = default;

  std::list<F3DReader*> Readers;
};

#endif
