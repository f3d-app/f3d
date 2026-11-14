// F3D final shader: fog normalized to the visible scene's depth range.
const vec3 fogColor = vec3(0.62, 0.69, 0.75);
// Higher values delay the fog; lower values make it appear sooner.
const float fogCurve = 1.0;
// More samples improve the depth-range estimate but cost more GPU time.
const int depthSamplesX = 24;
const int depthSamplesY = 16;

float linearizeDepth(float d)
{
    float n = cameraClippingRange.x;
    float f = cameraClippingRange.y;
    if (cameraParallelProjection)
    {
        return mix(n, f, d);
    }
    return (n * f) / (f - d * (f - n));
}

// Shared screen-space grid: all fragments estimate the same visible range.
// Exclude background pixels; only geometry contributes to the gradient.
vec2 visibleDepthRange()
{
    float nearest = 1.0;
    float farthest = 0.0;
    for (int y = 0; y < depthSamplesY; ++y)
    {
        for (int x = 0; x < depthSamplesX; ++x)
        {
            vec2 sampleUV = (vec2(x, y) + 0.5) / vec2(depthSamplesX, depthSamplesY);
            float d = texture(sourceDepth, sampleUV).r;
            if (d < 1.0)
            {
                nearest = min(nearest, d);
                farthest = max(farthest, d);
            }
        }
    }
    return vec2(nearest, farthest);
}

vec4 pixel(vec2 uv)
{
    vec4 color = texture(source, uv);
    float rawDepth = texture(sourceDepth, uv).r;
    if (rawDepth >= 1.0)
    {
        return vec4(fogColor, 1.0);
    }

    vec2 range = visibleDepthRange();
    // An empty grid or a flat depth range cannot define a gradient.
    if (range.x >= range.y)
    {
        return color;
    }
    float nearDepth = linearizeDepth(range.x);
    float farDepth = linearizeDepth(range.y);
    float depth = linearizeDepth(rawDepth);
    float normalizedDepth = clamp((depth - nearDepth) / max(farDepth - nearDepth, 0.000001), 0.0, 1.0);
    float fogAmount = pow(normalizedDepth, fogCurve);
    return vec4(mix(color.rgb, fogColor, fogAmount), color.a);
}
