#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkPNGReader.h>
#include <vtksys/SystemTools.hxx>

#include <fstream>

int main(int argc, char* argv[])
{
  vtkNew<vtkPNGReader> reader;
  reader->SetFileName(argv[1]);
  reader->Update();

  vtkImageData* icon = reader->GetOutput();

  if (!icon)
  {
    return 1;
  }

  int dim[3];
  icon->GetDimensions(dim);
  int nbComp = icon->GetNumberOfScalarComponents();

  std::ofstream outfile(vtksys::SystemTools::ConvertToOutputPath(argv[2]));

  outfile << "const int F3DIconDimX = " << dim[0] << ";\n";
  outfile << "const int F3DIconDimY = " << dim[1] << ";\n";
  outfile << "const int F3DIconNbComp = " << nbComp << ";\n";

  outfile << "const unsigned char F3DIconBuffer[] = {\n";
  unsigned char* p = static_cast<unsigned char*>(icon->GetScalarPointer(0, 0, 0));

  for (int i = 0; i < dim[0] * dim[1] * nbComp; i++)
  {
    outfile << std::hex << "  0x" << static_cast<unsigned int>(p[i]) << ",\n";
  }

  outfile << std::dec << "};\n";

  return 0;
}
