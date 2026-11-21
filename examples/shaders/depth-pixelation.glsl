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

float pixelationLevel(float depth)
{
    const float distanceScale = 2500.0;
    float t = max(depth, 0.0) / (max(depth, 0.0) + distanceScale);
    return floor(t * 6.0 + 0.5);
}

vec4 pixel(vec2 uv)
{
    vec2 size = vec2(resolution);
    for (int level = 6; level >= 0; --level)
    {
        float blockSize = exp2(float(level));
        vec2 origin = floor(uv * size / blockSize) * blockSize;
        vec4 colorSum = vec4(0.0);
        float sampleCount = 0.0;
        float closestDepth = 1.0;
        for (int y = 0; y < 3; ++y)
        {
            for (int x = 0; x < 3; ++x)
            {
                vec2 offset = mix(vec2(0.5), vec2(blockSize - 0.5), vec2(x, y) * 0.5);
                vec2 candidateUV = clamp((origin + offset) / size, 0.5 / size, (size - 0.5) / size);
                float candidateDepth = texture(sourceDepth, candidateUV).r;
                if (candidateDepth < 1.0)
                {
                    colorSum += texture(source, candidateUV);
                    sampleCount += 1.0;
                }
                if (candidateDepth < closestDepth)
                {
                    closestDepth = candidateDepth;

                }
            }
        }
        if (closestDepth < 1.0 &&
            float(level) <= pixelationLevel(linearizeDepth(closestDepth)))
        {
            return colorSum / sampleCount;
        }
    }
    return texture(source, uv);
}
