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
  for (uint h = 1; h <= WorkgroupSize; h *= 2)
  {
    barrier();
    compare_and_swap(flip(h));

    for (uint hh = h; hh >= 1; hh /= 2)
    {
      barrier();
      compare_and_swap(disperse(hh));
    }
  }
}
