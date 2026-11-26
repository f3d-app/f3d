// Glow: Based on https://learnopengl.com/Advanced-Lighting/Bloom
// God rays: Based on GPU Gems 3 ch.13 "Volumetric Light Scattering as a Post-Process"

// ---------------------------------------------------------------------------
// Name    : Aura
// Authors : Palpatoche8
// Ref     : Original work
// License : BSD-3-Clause (F3D)
// ---------------------------------------------------------------------------

uniform vec3 AURA_COLOR = vec3(1.0, 0.8, 0.4); 
uniform vec3 CORE_COLOR = vec3(1.0, 1.0, 0.9);
uniform float AURA_RADIUS = 0.07;             // slider(0.01, 0.5)
uniform float AURA_STRENGTH = 1.5;            // slider(0.1, 10)
uniform float RIM_STRENGTH = 0.6;             // slider(0.01, 3)
uniform float RAYS_STRENGTH = 0.30;           // slider(0.01, 5)
uniform float BG_THRESHOLD = 0.03;
uniform int DIRS = 16; 
uniform int STEPS = 6;


float objectMask(vec2 uv, vec3 bg)
{
  vec3 c = texture(source, clamp(uv, 0.0, 1.0)).rgb;
  return smoothstep(BG_THRESHOLD, BG_THRESHOLD * 3.0, distance(c, bg));
}

vec4 pixel(vec2 uv)
{
  vec3 bg = texture(source, vec2(0.002)).rgb;
  vec3 col = texture(source, uv).rgb;
  float aspect = float(resolution.x) / float(resolution.y);

  float self = objectMask(uv, bg);
  float outside = 1.0 - self;

  float glow = 0.0;
  float ring = 0.0;
  float wsum = 0.0;
  for (int i = 0; i < DIRS; i++)
  {
    float a = 6.28 * (float(i) + 0.5) / float(DIRS);
    vec2 dir = vec2(cos(a) / aspect, sin(a));
    for (int j = 1; j <= STEPS; j++)
    {
      float t = float(j) / float(STEPS);
      float w = 1.0 - t * 0.8;
      float m = objectMask(uv + dir * AURA_RADIUS * t, bg);
      glow += w * m;
      wsum += w;
      if (j == 1) 
        ring += m;
    }
  }
  glow /= wsum;
  ring /= float(DIRS);

  float pulse = 0.85 + 0.15 * sin(time * 2.0);
  float g = pow(glow, 0.6) * pulse;

  vec3 result = col;

  vec3 auraCol = mix(AURA_COLOR, CORE_COLOR, smoothstep(0.3, 0.9, g));
  result += auraCol * g * AURA_STRENGTH * outside;

  float rim = self * (1.0 - ring);
  result += CORE_COLOR * rim * RIM_STRENGTH * pulse;

  vec2 p = uv - vec2(0.5, 1.2);
  p.x *= aspect;
  float ang = atan(p.x, -p.y);
  float rays = 0.6 * pow(0.5 + 0.5 * sin(ang * 22.0 + time * 0.4), 8.0)
             + 0.4 * pow(0.5 + 0.5 * sin(ang * 9.0 - time * 0.25), 4.0);
  float fall = 1.0 - smoothstep(0.3, 1.6, length(p));
  result += AURA_COLOR * rays * fall * RAYS_STRENGTH * outside;

  return vec4(result, 1.0);
}
