/**
 * @class   vtkF3DLYSReader
 * @brief   Reader for the LYS file format used by Lychee Slicer.
 *
 * The LYS format is a custom binary container with the following structure:
 *
 * Container header (16 bytes, all fields uint32 little-endian):
 *   bytes  0- 3 : container format version (observed value: 4)
 *   bytes  4- 7 : padded JSON block size (includes trailing alignment padding)
 *   bytes  8-11 : secondary size field (not used by this reader)
 *   bytes 12-15 : actual JSON content length
 *
 * JSON manifest (jsonLen bytes starting at byte 16):
 *   UTF-8 encoded JSON object. The "mangoFiles" key maps entry names to objects
 *   with "offset" (string, byte offset into the data section) and "size" (integer).
 *
 * Data section (starts at byte 16 + padded JSON block size):
 *   Raw binary blobs. The geometry entry is the "mangoFiles" key whose name ends
 *   in ".bin" (with at least one character before the extension) and is not "scene.bin".
 *
 * Geometry blob header (12 bytes, all fields uint32 little-endian):
 *   bytes 0- 3 : indexCount  - number of uint32 triangle indices
 *   bytes 4- 7 : coordCount  - number of float32 values (nVertices * 3)
 *   bytes 8-11 : reserved    - ignored
 *
 * Index buffer: indexCount x uint32 immediately after the 12-byte mesh header.
 * Vertex buffer: coordCount x float32 immediately after the index buffer.
 * Vertex layout: XYZ float32 triplets (coordCount / 3 points total).
 */

#ifndef vtkF3DLYSReader_h
#define vtkF3DLYSReader_h

#include <vtkAbstractPolyDataReader.h>

class vtkResourceStream;

class vtkF3DLYSReader : public vtkAbstractPolyDataReader
{
public:
  static vtkF3DLYSReader* New();
  vtkTypeMacro(vtkF3DLYSReader, vtkPolyDataAlgorithm);

  /**
   * Return true if the stream looks like a valid LYS container.
   * Checks the 16-byte header and verifies the JSON manifest contains the "mangoFiles" key.
   */
  static bool CanReadFile(vtkResourceStream* stream);

protected:
  vtkF3DLYSReader();

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkF3DLYSReader(const vtkF3DLYSReader&) = delete;
  void operator=(const vtkF3DLYSReader&) = delete;
};

#endif
