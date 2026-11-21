// ---------------------------------------------------------------------------
// Name    : Gray
// Authors : mno
// Ref     : Original work
// License : BSD-3-Clause (F3D)
// ---------------------------------------------------------------------------

vec4 pixel(vec2 uv)
{
  vec4 color;
  float sum = texture(source, uv).r + texture(source, uv).g + texture(source, uv).b;
  sum /= 3.0;
  color.r = sum;
  color.g = sum;
  color.b = sum;
  color.w = 1.0;
  return color;
}
