const float SCAN_PERIOD = 6.0;
const float SCAN_PHASE = 0.0;
const float BAND_WIDTH = 0.002;
const float TRAIL_LENGTH = 0.08;
const float TRAIL_INTENSITY = 0.25;
const float BASE_BRIGHTNESS = 0.75;
const vec3 SCAN_COLOR = vec3(0.0, 1.0, 0.0);

vec4 pixel(vec2 uv)
{
  vec4 color = texture(source, uv);

  if (readRawDepth(uv) >= 1.0)
  {
    return color;
  }

  float depth = readDepth(uv);
  float normalizedDepth = (depth - cameraClippingRange.x) / (cameraClippingRange.y - cameraClippingRange.x);

  float scan = fract(time / SCAN_PERIOD + SCAN_PHASE);
  float delta = scan - normalizedDepth;

  float line = 1.0 - smoothstep(0.0, BAND_WIDTH, abs(delta));
  float trail = step(0.0, delta) * (1.0 - smoothstep(0.0, TRAIL_LENGTH, delta));

  color.rgb = color.rgb * BASE_BRIGHTNESS + SCAN_COLOR * (line + TRAIL_INTENSITY * trail);
  return vec4(clamp(color.rgb, 0.0, 1.0), color.a);
}
