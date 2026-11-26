// ---------------------------------------------------------------------------
// Name    : Radial blur
// Authors : mno
// Ref     : Full Scene Radial Blur by Shane - https://www.shadertoy.com/view/XsKGRW
//           (see original author notes below)
// License : CC BY-NC-SA 3.0 (Shadertoy default, no license stated by author)
// ---------------------------------------------------------------------------

/*
	Full Scene Radial Blur
	----------------------

	Radial blur - as a postprocessing effect - is one of the first things I considered doing 
	when the multipass system came out. I've always loved this effect. Reminds me of the early 
	demos from Aardbei et al. 

	Anyway, Shadertoy user, Passion, did a really cool radial blur on a field of spheres that
	inspired me to do my own. Radial blurs are pretty straight forward, but it was still
    helpful to have Passion's version as a guide. 

    As for the radial blur process, there's not much to it. Start off at the pixel position, 
    then radiate outwards gathering up pixels with decreased weighting. The result is a
	blurring of the image in a radial fashion, strangely enough. :)

	Inspired by:

	Blue Dream - Passion
	https://www.shadertoy.com/view/MdG3RD

	Radial Blur - IQ
	https://www.shadertoy.com/view/4sfGRn

	Rays of Blinding Light - mu6k
	https://www.shadertoy.com/view/lsf3Dn
 
  og shader
  https://www.shadertoy.com/view/XsKGRW
*/

float decay = 0.96; // slider(0.8, 1.0)
float density = 0.5; // slider(0.0, 2.0)
float weight = 0.1; // slider(0.0, 0.5)

float hash( vec2 p ){ return fract(sin(dot(p, vec2(41, 289)))*45758.5453); }

vec3 lOff(){    
    
    vec2 u = sin(vec2(1.57, 0) - time/2.);
    mat2 a = mat2(u, -u.y, u.x);
    
    vec3 l = normalize(vec3(1.5, 1., -0.5));
    l.xz = a * l.xz;
    l.xy = a * l.xy;
    
    return l;
    
}

vec4 pixel(vec2 uv)
{
    vec3 l = lOff();
    vec2 tuv = uv - 0.5 - l.xy * 0.45;
    vec2 dTuv = tuv * density / 24.0;
    vec4 col = texture(source, uv) * 0.25;
    uv += dTuv*(hash(uv + fract(time))*2. - 1.);

    for(float i=0.; i < 24.0f; i++){
    
        uv -= dTuv;
        col += texture(source, uv) * weight;
        weight *= decay;
        
    }
    col *= (1. - dot(tuv, tuv)*.75);
    return sqrt(smoothstep(0., 1., col));  


} 
