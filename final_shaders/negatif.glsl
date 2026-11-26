// ---------------------------------------------------------------------------
// Name    : Negatif
// Authors : lance-leau
// Ref     : Original work
// License : BSD-3-Clause (F3D)
// ---------------------------------------------------------------------------

vec4 pixel(vec2 uv)
{
  return vec4(vec3(1.0, 1.0, 1.0) - texture(source, uv).rgb, 1.0);
}
