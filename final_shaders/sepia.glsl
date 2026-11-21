// ---------------------------------------------------------------------------
// Name    : Sepia
// Authors : mno
// Ref     : Original work
// License : BSD-3-Clause (F3D)
// ---------------------------------------------------------------------------

vec4 pixel(vec2 uv)
{
  vec4 color;
  color.r = texture(source, uv).r * 0.393 + texture(source, uv).g * 0.869 + texture(source, uv).b * 0.189;
  color.g = texture(source, uv).r * 0.349 + texture(source, uv).g * 0.686 + texture(source, uv).b * 0.168;
  color.b = texture(source, uv).r * 0.262 + texture(source, uv).g * 0.534 + texture(source, uv).b * 0.131;

  return color;

}
