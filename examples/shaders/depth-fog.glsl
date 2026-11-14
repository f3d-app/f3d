const vec3 fogColor = vec3(0.62, 0.69, 0.75);
// Tuned for the initial view of christmas_low-poly_scene.
const float fogStart = 150.0;
const float fogEnd = 220.0; // Must be greater than fogStart.
// Higher values delay the fog; values between 0 and 1 strengthen it.
const float fogCurve = 1.0;

vec4 pixel(vec2 uv)
{
    vec4 color = texture(source, uv);
    float rawDepth = readRawDepth(uv);
    if (rawDepth >= 1.0)
    {
        return vec4(fogColor, 1.0);
    }

    float depth = readDepth(uv);
    float normalizedDepth = clamp((depth - fogStart) / max(fogEnd - fogStart, 0.000001), 0.0, 1.0);
    float fogAmount = pow(normalizedDepth, fogCurve);
    return vec4(mix(color.rgb, fogColor, fogAmount), color.a);
}
