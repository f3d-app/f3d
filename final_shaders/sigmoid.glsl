// ---------------------------------------------------------------------------
// Name    : Sigmoid contrast
// Authors : mno
// Ref     : Sigmoid contrast by Loadus - https://www.shadertoy.com/view/MtX3z2
// License : CC BY-NC-SA 3.0 (Shadertoy default, no license stated by author)
// ---------------------------------------------------------------------------

float gamma = 0.5; // slider(0.1, 3.0)

float cubic(float value)
{
    if (value < 0.5)
    {
        return value * value * value * value * value * 16.0;
    }
    value -=1.0;
    return value * value * value * value * value * 16.0 + 1.0;

}

float Sigmoid(float x)
{
    return 1.0 / (1.0 + exp(-(x - 0.5) * 14));
}

vec4 pixel(vec2 uv)
{
    vec4 c = texture(source, uv);
    vec4 a = c;
    c = vec4(cubic(c.r), cubic(c.g), cubic(c.b), 1.0);
    c = pow(c, vec4(gamma));
    return c;
}


