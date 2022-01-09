#include "vtkF3DMetaReader.h"

#include "F3DReaderFactory.h"

#include <vtkDemandDrivenPipeline.h>
#include <vtkEventForwarderCommand.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>


vtkStandardNewMacro(vtkF3DMetaReader);

//----------------------------------------------------------------------------
vtkF3DMetaReader::vtkF3DMetaReader()
{
  this->SetNumberOfInputPorts(0);
}

//----------------------------------------------------------------------------
vtkF3DMetaReader::~vtkF3DMetaReader()
{
  this->SetFileName(nullptr);
}

//----------------------------------------------------------------------------
void vtkF3DMetaReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "FileName:" << this->FileName << "\n";
  if (this->InternalReader)
  {
    os << indent << "InternalReader:\n";
    this->InternalReader->PrintSelf(os, indent.GetNextIndent());
  }
  else
  {
    os << indent << "InternalReader: (none)\n";
  }
}

//----------------------------------------------------------------------------
vtkInformation* vtkF3DMetaReader::GetOutputInformation(int port)
{
  if (!this->InternalReader)
  {
    vtkErrorMacro("InternalReader has not been created yet, "
                  "make sure to set to use a supported file format and to set the FileName");
    return 0;
  }

  return this->InternalReader->GetOutputInformation(port);
}

//----------------------------------------------------------------------------
int vtkF3DMetaReader::ProcessRequest(
  vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  if (!this->InternalReader)
  {
    vtkErrorMacro("InternalReader has not been created yet, "
                  "make sure to set to use a supported file format and to set the FileName");
    return 0;
  }

  return this->InternalReader->ProcessRequest(request, inputVector, outputVector);
}

//----------------------------------------------------------------------------
int vtkF3DMetaReader::FillOutputPortInformation(int port, vtkInformation* info)
{
  if (!this->InternalReader)
  {
    vtkErrorMacro("InternalReader has not been created yet, "
                  "make sure to set to use a supported file format and to set the FileName");
    return 0;
  }

  vtkInformation* tmpInfo = this->InternalReader->GetOutputPortInformation(port);
  if (tmpInfo)
  {
    info->Copy(tmpInfo);
    return 1;
  }
  return 0;
}

//----------------------------------------------------------------------------
void vtkF3DMetaReader::SetFileName(const std::string& fileName)
{
  vtkMTimeType time = this->GetMTime();
  this->SetFileName(fileName.c_str());
  if (time == this->GetMTime())
  {
    return;
  }

  F3DReader* reader = F3DReaderFactory::GetReader(fileName);
  if (reader)
  {
    this->InternalReader = reader->CreateGeometryReader(fileName);
  }

  if (this->InternalReader)
  {
    // forward progress event
    vtkNew<vtkEventForwarderCommand> forwarder;
    forwarder->SetTarget(this);
    this->InternalReader->AddObserver(vtkCommand::ProgressEvent, forwarder);
  }
}
