// ---------------------------------------------------------------------------
// Name    : Kamisuki
// Authors : ThoomasGM
// Ref     : https://www.shadertoy.com/view/7clyWr
// License : CC BY-NC-SA 3.0 (Shadertoy default, no license stated by author)
// ---------------------------------------------------------------------------

#define PI acos(-1.)

uniform float scale = 20.0;        // slider(1.0, 100.0)
uniform float speed = 1.0;         // slider(0.0, 5.0)
uniform float wavesBase = 1.0;     // slider(0.5, 10.0)
uniform float wavesAmp = 3.0;      // slider(0.0, 10.0)
uniform float maxRotation = 1.0;   // slider(0.0, 2.0)
uniform float radiusFactor = 0.5;  // slider(0.0, 2.0)
uniform float blending = 0.125;    // slider(0.01, 0.5)
uniform float threshold = 0.5;     // slider(0.0, 1.0)
uniform float edgeSmooth = 4.0;    // slider(0.5, 20.0)
uniform float shadowMult = 0.7;    // slider(0.0, 1.0)
uniform float highlightMult = 1.3; // slider(1.0, 2.5)

// "Hash without Sine" by Dave_Hoskins
// https://www.shadertoy.com/view/4djSRW
vec2 hash22(vec2 p)
{
    vec3 p3 = fract(vec3(p.xyx) * vec3(0.1031, 0.1030, 0.0973));
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.xx + p3.yz) * p3.zy);
}

// "A list of Smooth-minimums" by iq
// https://iquilezles.org/articles/smin/
float smin(float a, float b, float k)
{
    k *= 6.0;
    float h = max(k - abs(a - b), 0.0) / k;
    return min(a, b) - h * h * h * k * (1.0 / 6.0);
}

float KamisukiNoise(vec2 p, float n, float a, float r, float k)
{
    /*
        n: num of waves per kernel
        a: max rotation
        r: kernel radius
        k: blending strength
    */
    vec2 ip = round(p);
    float total = 8.0;

    for (int y = -2; y <= 2; y++)
    {
        for (int x = -2; x <= 2; x++)
        {
            vec2 cell = ip + vec2(float(x), float(y));
            vec2 o = hash22(cell + 2.0) * 2.0 - 1.0;
            vec2 lp = p.xy - cell;
            float d = length(lp) - r;
            float la = o.x * a;
            lp *= mat2(cos(la), sin(la), -sin(la), cos(la));
            d = -smin(-(sin(lp.y * PI * 2.0 * n) * 0.5 + 0.5), -d, k);
            total = smin(total, d, k);
        }
    }
    return total;
}

vec4 pixel(vec2 uv)
{
    float aspect = float(resolution.x) / float(resolution.y);
    vec2 p = (uv - 0.5) * vec2(aspect, 1.0);
    float t = time * speed;

    float n = KamisukiNoise(
        p * scale,
        (sin(t * 0.5) * 0.5 + 0.5) * wavesAmp + wavesBase,
        (PI * maxRotation) * (sin(t) * 0.5 + 0.5),
        radiusFactor * p.x,
        blending
    );

    n = smoothstep(threshold, threshold + (1.0 / float(resolution.y)) * edgeSmooth, n);

    vec4 scene = texture(source, uv);

    vec3 result = scene.rgb * mix(shadowMult, highlightMult, n);

    return vec4(result, scene.a);
}