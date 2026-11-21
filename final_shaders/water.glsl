// ---------------------------------------------------------------------------
// Name    : Water
// Authors : Jules Caron
// Ref     : Tileable water caustic by David Hoskins - https://www.shadertoy.com/view/MdlXz8
//           (original water turbulence effect by joltz0r, GLSL Sandbox)
// License : CC BY-NC-SA 3.0 (Shadertoy default, no license stated by author)
// ---------------------------------------------------------------------------

float TAU = 6.28318530;
int MAX_ITER = 5;
uniform float light_intensity = 0.005; // slider(0.002, 0.006)

vec4 pixel(vec2 uv) 
{
    vec2 p = mod(uv*TAU, TAU)-250.0;

	vec2 i = vec2(p);
	float c = 1.0;

	for (int n = 0; n < MAX_ITER; n++) 
	{
		float t = (time * 0.5 + 20) * (1.0 - (3.5 / float(n+1)));
		i = p + vec2(cos(t - i.x) + sin(t + i.y), sin(t - i.y) + cos(t + i.x));
		c += 1.0/length(vec2(p.x / (sin(i.x+t) / light_intensity),p.y / (cos(i.y+t) / light_intensity)));
	}
	c /= float(MAX_ITER);
	c = 1.17-pow(c, 1.4);
	vec3 colour = vec3(pow(abs(c), 8.0)) + texture(source, uv).rgb;
    colour = clamp(colour + vec3(0.0, 0.35, 0.5), 0.0, 1.0);
    
	return vec4(colour, 1.0);
}