// This file is intended to be included in the point splat mapper fragment shader
// It defines signed distance field functions used to represent anisotropic gaussians

float sdCircle(vec2 p, float radius)
{
  return length(p) - radius;
}

float sdRoundBox(vec2 p, vec2 halfSize, float radius)
{
    vec2 q = abs(p) - halfSize + radius;
    return length(max(q, 0.0)) - radius + min(max(q.x, q.y), 0.0);
}

float sdSegment(vec2 p, vec2 a, vec2 b)
{
    vec2 pa = p - a;
    vec2 ba = b - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - ba * h);
}

float sdCross(in vec2 p, in vec2 b, float r)
{
    p = abs(p);
    p = (p.y > p.x) ? p.yx : p.xy;
    vec2 q = p - b;
    float k = max(q.y,q.x);
    vec2 w = (k>0.0) ? q : vec2(b.y-p.x,-k);
    return sign(k)*length(max(w,0.0)) + r;
}

float opMerge(float shape1, float shape2)
{
    return min(shape1, shape2);
}

float opRoundMerge(float shape1, float shape2, float radius)
{
    vec2 intersectionSpace = vec2(shape1 - radius, shape2 - radius);
    intersectionSpace = min(intersectionSpace, 0.0);
    float insideDistance = -length(intersectionSpace);
    float simpleUnion = opMerge(shape1, shape2);
    float outsideDistance = max(simpleUnion, radius);
    return  insideDistance + outsideDistance;
}

float opShift(float sd, float shift)
{
  return sd - shift;
}

float strokePx(float sd, float thickness)
{
  float halfStroke = 0.5 * thickness;

  return 1.0 - smoothstep(halfStroke - 1.0, halfStroke + 1.0, abs(sd));
}

float stroke(float sd, float thickness)
{
  float w = fwidth(sd);

  float halfStroke = 0.5 * thickness * w;

  return 1.0 - smoothstep(halfStroke - w, halfStroke + w, abs(sd));
}

float fillPx(float sd)
{
    return 1.0 - smoothstep(0.0, 1.0, sd);
}

float fill(float sd)
{
    float w = fwidth(sd);

    return 1.0 - smoothstep(0.0, w, sd);
}
