const float OUTLINE_WIDTH = 1.0;
const float DEPTH_THRESHOLD = 0.006;
const vec3 INK_COLOR = vec3(0.02);
const vec3 PAPER_COLOR = vec3(1.0);

float depthAt(vec2 uv)
{
    vec2 halfTexel = 0.5 / vec2(resolution);
    return texture(source, clamp(uv, halfTexel, vec2(1.0) - halfTexel)).r;
}

vec4 pixel(vec2 uv)
{
    vec2 stepUV = OUTLINE_WIDTH / vec2(resolution);

    float tl = depthAt(uv + stepUV * vec2(-1.0,  1.0));
    float tc = depthAt(uv + stepUV * vec2( 0.0,  1.0));
    float tr = depthAt(uv + stepUV * vec2( 1.0,  1.0));
    float ml = depthAt(uv + stepUV * vec2(-1.0,  0.0));
    float mr = depthAt(uv + stepUV * vec2( 1.0,  0.0));
    float bl = depthAt(uv + stepUV * vec2(-1.0, -1.0));
    float bc = depthAt(uv + stepUV * vec2( 0.0, -1.0));
    float br = depthAt(uv + stepUV * vec2( 1.0, -1.0));

    float gx = -tl - 2.0 * ml - bl + tr + 2.0 * mr + br;
    float gy = -bl - 2.0 * bc - br + tl + 2.0 * tc + tr;
    float gradient = length(vec2(gx, gy)) / 8.0;
    float edge = step(DEPTH_THRESHOLD, gradient);

    return vec4(mix(PAPER_COLOR, INK_COLOR, edge), 1.0);
}
