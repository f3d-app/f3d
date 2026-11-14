const int SAMPLES = 16;

const int BANDS = 20;

// choose the color according to the depth
vec3 depthGradient(float t)
{
    // 5 colors for the color gradient
    const int N = 5;
    vec3 stops[N] = vec3[N](
        vec3(1.00, 0.95, 0.10),
        vec3(0.30, 0.85, 0.30),
        vec3(0.10, 0.75, 1.00),
        vec3(0.10, 0.25, 0.95),
        vec3(0.02, 0.03, 0.25)
    );
    float x = clamp(t, 0.0, 1.0) * float(N - 1);
    int i = int(min(floor(x), float(N - 2)));
    return mix(stops[i], stops[i + 1], x - float(i));
}

// returns the range of the nearest point and the furthest point
vec2 visibleDistanceRange()
{
    vec2 range = vec2(1e30, -1e30);
    for (int j = 0; j < SAMPLES; j++)
    {
        for (int i = 0; i < SAMPLES; i++)
        {
            vec2 p = (vec2(i, j) + 0.5) / float(SAMPLES);
            if (readRawDepth(p) < 1.0)
            {
                float dist = readDepth(p);
                range = vec2(min(range.x, dist), max(range.y, dist));
            }
        }
    }
    return range;
}

vec4 pixel(vec2 uv)
{
    vec2 range = visibleDistanceRange();

    float t = 1.0;
    if (readRawDepth(uv) < 1.0)
    {
        float depth = readDepth(uv);
        t = clamp((depth - range.x) / max(range.y - range.x, 1e-6), 0.0, 1.0);
    }

    vec3 color = depthGradient(t);

    return vec4(color, 1.0);
}
