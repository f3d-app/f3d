// ---------------------------------------------------------------------------
// Name    : Bloom
// Authors : lance-leau
// Ref     : Original work
// License : BSD-3-Clause (F3D)
// ---------------------------------------------------------------------------

int radius = 15; // slider(1, 30)
uniform float threshold = 0.55; // slider(0.0, 1.0)
uniform float intensity = 1.5;  // slider(0.0, 10.0)
uniform float spread = 1.0;     // slider(0.0, 1.0)

vec4 pixel(vec2 uv)
{
  vec4 couleur = texture(source, uv);

  vec2 texelSize = 1.0 / vec2(textureSize(source, 0));

  int radius = 15;

  vec3 bloom = vec3(0.0);
  float totalWeight = 0.0;
  float sigma = float(radius) / 3.0;

  for (int x = -radius; x <= radius; x++)
  {
    for (int y = -radius; y <= radius; y++)
    {
      vec2 offset = vec2(float(x), float(y)) * texelSize * spread;
      vec3 neighbor = texture(source, uv + offset).rgb;

      float brightness = dot(neighbor, vec3(0.299, 0.587, 0.114));

      float factor = smoothstep(threshold, threshold + 0.15, brightness);
      vec3 brightColor = neighbor * factor;

      float distSq = float(x * x + y * y);
      float weight = exp(-distSq / (2.0 * sigma * sigma));

      bloom += brightColor * weight;
      totalWeight += weight;
    }
  }

  bloom /= totalWeight;

  return vec4(couleur.rgb + bloom * intensity, couleur.a);
}
