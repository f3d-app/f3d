// ---------------------------------------------------------------------------
// Name    : CRT
// Authors : Jules Caron
// Ref     : Original work
// License : BSD-3-Clause (F3D)
// ---------------------------------------------------------------------------

uniform float scan_size = 1.00; // slider(0.95, 1.045)
uniform float scan_speed = 6.50; //slider(0.0, 30.0)

vec4 pixel(vec2 uv)
{
    float wave = abs(sin(((uv * resolution.xy).y + time * scan_speed) * scan_size));
    return vec4(mix(texture(source, uv).rgb, vec3(0.0), wave), 1.0);
}
