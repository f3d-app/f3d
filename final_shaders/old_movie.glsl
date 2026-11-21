// ---------------------------------------------------------------------------
// Name    : Old movie
// Authors : mno
// Ref     : Old movie by picolocolo - https://www.shadertoy.com/view/mslfD8
// License : CC BY-NC-SA 3.0 (Shadertoy default, no license stated by author)
// ---------------------------------------------------------------------------

float noise1(float seed1,float seed2){
return(
fract(seed1+12.34567*
fract(100.*(abs(seed1*0.91)+seed2+94.68)*
fract((abs(seed2*0.41)+45.46)*
fract((abs(seed2)+757.21)*
fract(seed1*0.0171))))))
* 1.0038 - 0.00185;
}


vec4 pixel(vec2 uv)
{
  vec2 normalized_uv = uv * 2.0-1.0;
  vec4 color;
  color.r = texture(source, uv + 0.002).r;
  color.g = texture(source, uv - 0.002).g;
  color.b = texture(source, uv - 0.002).b;
  color -= length(normalized_uv) * 0.1;
  float noise = noise1(uv.x * 100.0, uv.y * 100.0) * 2.0-1.0;
  noise *= 0.05;
  color += color*noise;
  return color;

}
