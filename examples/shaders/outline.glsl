const float DEPTH_THRESHOLD = 0.0205;
const float OUTLINE_WIDTH = 2.0;
const bool OPENING_ENABLED = true;
const int OPENING_MIN_PIXELS = 8;
const vec3 INK_COLOR = vec3(0.02);
const bool OUTLINE_ONLY = false;

float depthAt(vec2 uv)
{
    vec2 halfTexel = 0.5 / vec2(textureSize(sourceDepth, 0));
    float depth = readDepth(clamp(uv, halfTexel, vec2(1.0) - halfTexel));
    // Keep planar faces contour-free; readDepth already supplies linear camera-space depth.
    return cameraParallelProjection ? depth : 1.0 / depth;
}

float edgeAt(vec2 uv)
{
    if (any(lessThan(uv, vec2(0.0))) || any(greaterThanEqual(uv, vec2(1.0))))
        return 0.0;

    float rawDepth = readRawDepth(uv);
    if (rawDepth >= 1.0)
    {
        return 0.0;
    }

    vec2 texel = 1.0 / vec2(resolution);
    float center = depthAt(uv);
    float tc = depthAt(uv + texel * vec2( 0.0,  1.0));
    float ml = depthAt(uv + texel * vec2(-1.0,  0.0));
    float mr = depthAt(uv + texel * vec2( 1.0,  0.0));
    float bc = depthAt(uv + texel * vec2( 0.0, -1.0));

    // Opposite differences cancel a regular planar slope instead of outlining its whole face.
    vec2 change = vec2(ml + mr, tc + bc) - 2.0 * center;
    float discontinuity = length(change) / (2.0 * max(abs(center), 1e-20));
    return step(DEPTH_THRESHOLD, discontinuity);
}

float openedEdgeAt(vec2 uv)
{
    float center = edgeAt(uv);
    if (!OPENING_ENABLED || OPENING_MIN_PIXELS <= 1 || center == 0.0)
        return center;

    vec2 texel = 1.0 / vec2(resolution);
    ivec2 component[OPENING_MIN_PIXELS];
    component[0] = ivec2(0);
    int count = 1;

    // Area opening: stop as soon as the 8-connected component is large enough.
    // This removes tiny components without eroding thin curves or their endpoints.
    for (int head = 0; head < count; ++head)
    {
        for (int y = -1; y <= 1; ++y)
        {
            for (int x = -1; x <= 1; ++x)
            {
                ivec2 next = component[head] + ivec2(x, y);
                bool seen = false;
                for (int i = 0; i < count; ++i)
                    if (all(equal(next, component[i])))
                        seen = true;

                if (seen || edgeAt(uv + vec2(next) * texel) == 0.0)
                    continue;

                component[count++] = next;
                if (count >= OPENING_MIN_PIXELS)
                    return 1.0;
            }
        }
    }
    return 0.0;
}

vec4 pixel(vec2 uv)
{
    vec4 color = texture(source, uv);
    vec2 texel = 1.0 / vec2(resolution);
    int width = max(0, int(round(OUTLINE_WIDTH)));
    int low = (width - 1) / 2;
    int high = width / 2;
    float edge = 0.0;

    // Only widen the already binary mask; detection always samples adjacent pixels.
    for (int y = -low; y <= high && width > 0 && edge == 0.0; ++y)
        for (int x = -low; x <= high && edge == 0.0; ++x)
            edge = openedEdgeAt(uv + vec2(x, y) * texel);

    vec3 base = OUTLINE_ONLY ? vec3(1.0) : color.rgb;
    float alpha = OUTLINE_ONLY || edge > 0.0 ? 1.0 : color.a;
    return vec4(mix(base, INK_COLOR, edge), alpha);
}
