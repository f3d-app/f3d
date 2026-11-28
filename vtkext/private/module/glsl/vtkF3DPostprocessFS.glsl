//VTK::System::Dec

//VTK::Output::Dec

in vec2 texCoord;

uniform sampler2D texColor;
uniform sampler2D texDepth;
uniform sampler2D texNormal;
uniform sampler2D texColorMap;

uniform ivec2 resolution;
uniform float time;

uniform float ssaaScale;

//#define ENABLE_TONEMAPPING
//#define ENABLE_SSAA

vec4 toStraightAlpha(vec4 color)
{
  if (color.a > 0.0)
    color.rgb /= color.a;
  return color;
}

vec4 toSRGB(vec4 color)
{
  return vec4(pow(color.rgb, vec3(1.0/2.2)), color.a);
}

#ifdef ENABLE_SSAA
// Gaussian downsampling of the supersampled color texture
vec4 sampleColor(vec2 uv)
{
  ivec2 srcSize = textureSize(texColor, 0);
  vec2 center = uv * vec2(srcSize);
  ivec2 base = ivec2(floor(center));
  int radius = int(ceil(ssaaScale));

  vec4 sum = vec4(0.0);
  float weightSum = 0.0;
  for (int j = -radius; j <= radius; j++)
  {
    for (int i = -radius; i <= radius; i++)
    {
      ivec2 texel = clamp(base + ivec2(i, j), ivec2(0), srcSize - 1);
      vec2 d = (vec2(texel) + 0.5 - center) / ssaaScale;
      float w = exp(-2.0 * dot(d, d));
      sum += w * texelFetch(texColor, texel, 0);
      weightSum += w;
    }
  }
  return sum / weightSum;
}
#else
vec4 sampleColor(vec2 uv)
{
  return texture(texColor, uv);
}
#endif

// Khronos neutral tonemapping function
vec3 tonemap(vec3 color)
{
  const float startCompression = 0.8 - 0.04;
  const float desaturation = 0.15;
  float x = min(color.r, min(color.g, color.b));
  float offset = x < 0.08 ? x - 6.25 * x * x : 0.04;
  vec3 toned = color - vec3(offset);
  float peak = max(toned.r, max(toned.g, toned.b));
  if (peak >= startCompression)
  {
    const float d = 1. - startCompression;
    float newPeak = 1. - d * d / (peak + d - startCompression);
    toned *= newPeak / peak;
    float g = 1. - 1. / (desaturation * (peak - newPeak) + 1.);
    toned = mix(toned, newPeak * vec3(1, 1, 1), g);
  }
  return toned;
}

vec4 getLinearColor(vec2 uv)
{
  vec4 color = toStraightAlpha(sampleColor(uv));

  // Convert color to linear space if needed
  //VTK::ColorSpace::Impl

#ifdef ENABLE_TONEMAPPING
  color.rgb = tonemap(color.rgb);
#endif

  return color;
}

//VTK::User::Dec

void main()
{
  gl_FragData[0] = toSRGB(pixel(texCoord));
}
