#version 430

//VTK::BitonicDefines::Dec

layout(local_size_x = WorkgroupSize) in;
layout(std430) buffer;

layout(binding = 0) buffer Keys
{
  KeyType key[];
};

layout(binding = 1) buffer Values
{
  ValueType value[];
};

layout(location = 0) uniform int count;
layout(location = 1) uniform int height;

//VTK::BitonicFunctions::Dec

void main()
{
  compare_and_swap(disperse(height));
}
