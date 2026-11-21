// ---------------------------------------------------------------------------
// Name    : Comic
// Authors : Kae712635
// Ref     : Original work
// License : BSD-3-Clause (F3D)
// ---------------------------------------------------------------------------

uniform float saturation = 1.3;      // slider(0.0, 3.0)
uniform float levels = 5.0;          // slider(2.0, 16.0)
uniform float edge_threshold = 0.25; // slider(0.05, 0.5)

float lum(vec3 c) {
    return dot(c, vec3(0.299, 0.587, 0.114));
}

vec4 pixel(vec2 uv) {
    vec2 p = 1.0 / vec2(resolution);

    // sobel tap
    float s[8];
    s[0] = lum(texture(source, uv + vec2(-p.x, -p.y)).rgb);
    s[1] = lum(texture(source, uv + vec2( 0.0, -p.y)).rgb);
    s[2] = lum(texture(source, uv + vec2( p.x, -p.y)).rgb);
    s[3] = lum(texture(source, uv + vec2(-p.x,  0.0)).rgb);
    s[4] = lum(texture(source, uv + vec2( p.x,  0.0)).rgb);
    s[5] = lum(texture(source, uv + vec2(-p.x,  p.y)).rgb);
    s[6] = lum(texture(source, uv + vec2( 0.0,  p.y)).rgb);
    s[7] = lum(texture(source, uv + vec2( p.x,  p.y)).rgb);

    float gx = (s[2] + 2.0*s[4] + s[7]) - (s[0] + 2.0*s[3] + s[5]);
    float gy = (s[5] + 2.0*s[6] + s[7]) - (s[0] + 2.0*s[1] + s[2]);
    float ink = smoothstep(0.1, edge_threshold, length(vec2(gx, gy)));

    // posterize
    vec3 col = texture(source, uv).rgb;
    col = floor(col * levels + 0.5) / levels;

    // saturation boost
    col = clamp(mix(vec3(lum(col)), col, saturation), 0.0, 1.0);

    return vec4(col * (1.0 - ink), 1.0);
}
