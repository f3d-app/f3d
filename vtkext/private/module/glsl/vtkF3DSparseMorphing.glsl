// Requires OpenGL 4.3. Binding 0 is reserved for skinning matrices.
layout(std430, binding = 1) readonly buffer BlendShapeOffsets
{
  vec4 blendShapeOffsets[];
};
layout(std430, binding = 2) readonly buffer BlendShapeWeights
{
  float blendShapeWeights[];
};

vec3 f3dSparseMorphPosition(vec3 bindPosition, ivec2 range)
{
  vec3 position = bindPosition;
  for (int i = range.x; i < range.y; ++i)
  {
    vec4 offset = blendShapeOffsets[i];
    position += offset.xyz * blendShapeWeights[int(offset.w)];
  }
  return position;
}
