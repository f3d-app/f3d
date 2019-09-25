#include "vtkF3DOpenGLGridMapper.h"

#include <vtkObjectFactory.h>
#include <vtkShaderProgram.h>

vtkStandardNewMacro(vtkF3DOpenGLGridMapper);

const char* geometryShader = R"glsl(//VTK::System::Dec
// VTK::PositionVC::Dec// VTK::TCoord::Dec

layout(points) in;
layout(triangle_strip, max_vertices = 12) out;

in vec4 positionMCVSOutput[];
out vec4 positionMCGSOutput;

uniform mat4 MCDCMatrix;

void main()
{
  vec4 center = positionMCVSOutput[0];
  vec4 corner1 = vec4(1.0, 0.0, 0.0, 0.0);
  vec4 corner2 = vec4(0.0, 0.0, 1.0, 0.0);
  vec4 corner3 = vec4(-1.0, 0.0, 0.0, 0.0);
  vec4 corner4 = vec4(0.0, 0.0, -1.0, 0.0);

  positionMCGSOutput = center;
  gl_Position = MCDCMatrix * center;
  EmitVertex();
  positionMCGSOutput = corner1;
  gl_Position = MCDCMatrix * corner1;
  EmitVertex();
  positionMCGSOutput = corner2;
  gl_Position = MCDCMatrix * corner2;
  EmitVertex();
  EndPrimitive();

  positionMCGSOutput = center;
  gl_Position = MCDCMatrix * center;
  EmitVertex();
  positionMCGSOutput = corner2;
  gl_Position = MCDCMatrix * corner2;
  EmitVertex();
  positionMCGSOutput = corner3;
  gl_Position = MCDCMatrix * corner3;
  EmitVertex();
  EndPrimitive();

  positionMCGSOutput = center;
  gl_Position = MCDCMatrix * center;
  EmitVertex();
  positionMCGSOutput = corner3;
  gl_Position = MCDCMatrix * corner3;
  EmitVertex();
  positionMCGSOutput = corner4;
  gl_Position = MCDCMatrix * corner4;
  EmitVertex();
  EndPrimitive();

  positionMCGSOutput = center;
  gl_Position = MCDCMatrix * center;
  EmitVertex();
  positionMCGSOutput = corner4;
  gl_Position = MCDCMatrix * corner4;
  EmitVertex();
  positionMCGSOutput = corner1;
  gl_Position = MCDCMatrix * corner1;
  EmitVertex();
  EndPrimitive();
})glsl";

//----------------------------------------------------------------------------
void vtkF3DOpenGLGridMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkF3DOpenGLGridMapper::ReplaceShaderValues(
  std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor* actor)
{
  std::string VSSource = shaders[vtkShader::Vertex]->GetSource();
  std::string FSSource = shaders[vtkShader::Fragment]->GetSource();

  vtkShaderProgram::Substitute(
    VSSource, "//VTK::PositionVC::Dec", "out vec4 positionMCVSOutput;\n");

  vtkShaderProgram::Substitute(
    VSSource, "//VTK::PositionVC::Impl", "positionMCVSOutput = vertexMC;\n");

  vtkShaderProgram::Substitute(FSSource, "//VTK::PositionVC::Dec",
    "in vec4 positionMCGSOutput;\n"
    "uniform float fadeDist;\n");

  vtkShaderProgram::Substitute(FSSource, "  //VTK::Color::Impl",
    "  vec2 coord = positionMCGSOutput.xz / positionMCGSOutput.w;\n"
    "  vec2 grid = abs(fract(coord - 0.5) - 0.5) / fwidth(coord);\n"
    "  float line = min(grid.x, grid.y);\n"
    "  float dist2 = coord.x * coord.x + coord.y * coord.y;\n"
    "  float ratio = max(0.0, (1.0 - min(line, 1.0)) * (1.0 - dist2 / (fadeDist * fadeDist)));\n"
    "  gl_FragData[0] = vec4(diffuseColorUniform, ratio);\n");

  shaders[vtkShader::Vertex]->SetSource(VSSource);
  shaders[vtkShader::Geometry]->SetSource(geometryShader);
  shaders[vtkShader::Fragment]->SetSource(FSSource);

  // add camera uniforms declaration
  this->ReplaceShaderPositionVC(shaders, ren, actor);

  // add color uniforms declaration
  this->ReplaceShaderColor(shaders, ren, actor);
}

//----------------------------------------------------------------------------
void vtkF3DOpenGLGridMapper::SetMapperShaderParameters(
  vtkOpenGLHelper& cellBO, vtkRenderer* ren, vtkActor* actor)
{
  this->Superclass::SetMapperShaderParameters(cellBO, ren, actor);

  cellBO.Program->SetUniformf("fadeDist", this->FadeDistance);
}
