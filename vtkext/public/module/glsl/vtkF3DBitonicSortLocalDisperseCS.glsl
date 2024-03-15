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

//VTK::BitonicFunctions::Dec

void main()
{
  for (uint h = WorkgroupSize; h >= 1; h /= 2)
  {
    barrier();
    compare_and_swap(disperse(h));
  }
}
