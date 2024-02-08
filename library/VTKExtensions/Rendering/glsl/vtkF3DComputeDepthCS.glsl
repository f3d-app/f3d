#version 430
layout(local_size_x = 32) in;
layout(std430) buffer;

struct vertex
{
  float x;
  float y;
  float z;
};

layout(binding = 0) readonly buffer Points
{
  vertex point[];
};

layout(binding = 1) readonly buffer Indices
{
  uint index[];
};

layout(binding = 2) writeonly buffer Depths
{
  float depth[];
};

layout (location = 0) uniform vec3 viewDirection;
layout (location = 1) uniform int count;

void main()
{
  uint i = gl_GlobalInvocationID.x;
  if (i < count)
  {
    vertex v = point[index[i]];
    depth[i] = dot(viewDirection, vec3(v.x, v.y, v.z));
  }
}
