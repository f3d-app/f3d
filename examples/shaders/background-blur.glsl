const float FOCUS_DISTANCE = 0.0;
const vec2 AUTO_FOCUS_UV = vec2(0.5, 0.5);
const float BLUR_TRANSITION = 0.16;
const float MAX_BLUR_RADIUS = 24.0;
const int BLUR_SAMPLES = 16;

float getFocusDistance()
{
  if (FOCUS_DISTANCE > 0.0)
  {
    return FOCUS_DISTANCE;
  }

  float d = readRawDepth(AUTO_FOCUS_UV);
  if (d < 1.0)
  {
    return readDepth(AUTO_FOCUS_UV);
  }

  return mix(cameraClippingRange.x, cameraClippingRange.y, 0.5);
}

vec3 toLinear(vec3 color)
{
  return pow(max(color, vec3(0.0)), vec3(2.2));
}

vec4 pixel(vec2 uv)
{
  vec4 centerColor = texture(source, uv);
  float rawDepth = readRawDepth(uv);
  float focusDistance = getFocusDistance();
  float centerDepth = readDepth(uv);

  float defocus = max(centerDepth - focusDistance, 0.0) / focusDistance;
  float blurAmount = smoothstep(0.0, BLUR_TRANSITION, defocus);
  float radius = rawDepth >= 1.0 ? MAX_BLUR_RADIUS : MAX_BLUR_RADIUS * blurAmount;

  if (radius < 0.5)
  {
    return centerColor;
  }

  vec2 pixelSize = 1.0 / vec2(resolution);
  vec2 minUV = pixelSize * 0.5;
  vec2 maxUV = vec2(1.0) - minUV;
  float depthTolerance = focusDistance * 0.02;
  vec3 sum = toLinear(centerColor.rgb);
  float totalWeight = 1.0;

  for (int i = 0; i < BLUR_SAMPLES; ++i)
  {
    float sampleRadius = sqrt((float(i) + 0.5) / float(BLUR_SAMPLES));
    float angle = float(i) * 2.39996323;
    vec2 offset = vec2(cos(angle), sin(angle)) * sampleRadius * radius * pixelSize;
    vec2 sampleUV = clamp(uv + offset, minUV, maxUV);

    float sampleDepth = readDepth(sampleUV);

    float weight = 1.0 - smoothstep(0.0, depthTolerance, centerDepth - sampleDepth);
    sum += toLinear(texture(source, sampleUV).rgb) * weight;
    totalWeight += weight;
  }

  vec3 blurred = pow(sum / totalWeight, vec3(1.0 / 2.2));
  return vec4(blurred, centerColor.a);
}
