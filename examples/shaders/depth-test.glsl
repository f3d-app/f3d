vec4 pixel(vec2 uv)
{
    float rawDepth = readRawDepth(uv);

    if (rawDepth >= 1.0)
    {
        return vec4(0.0, 0.0, 0.0, 1.0);
    }

    float depth = readDepth(uv);

    float start = 500.0;
    float end = 2000.0;

    float normalizedDepth = clamp((depth - start) / (end - start), 0.0, 1.0);

    vec3 color = mix(vec3(0.0, 0.0, 1.0), vec3(1.0, 0.0, 0.0), normalizedDepth);
    return vec4(color, 1.0);
}