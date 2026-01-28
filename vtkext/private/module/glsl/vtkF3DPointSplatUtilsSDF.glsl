// This file is intended to be included in the point splat mapper fragment shader
// It defines signed distance field functions used to represent anisotropic gaussians

// return the signed distance from a point to a circle of given radius
float sdCircle(vec2 p, float radius)
{
  return length(p) - radius;
}

// return the signed distance from a point to a box of given half size and corner radius
float sdRoundBox(vec2 p, vec2 halfSize, float radius)
{
    vec2 q = abs(p) - halfSize + radius;
    return length(max(q, 0.0)) - radius + min(max(q.x, q.y), 0.0);
}

// return the positive distance from a point to a line segment defined by points a and b
float sdSegment(vec2 p, vec2 a, vec2 b)
{
    vec2 pa = p - a;
    vec2 ba = b - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - ba * h);
}

// boolean union operation between two SDFs
float opMerge(float shape1, float shape2)
{
    return min(shape1, shape2);
}

// boolean union operation between two SDFs with smooth blending
float opRoundMerge(float shape1, float shape2, float radius)
{
    vec2 intersectionSpace = vec2(shape1 - radius, shape2 - radius);
    intersectionSpace = min(intersectionSpace, 0.0);
    float insideDistance = -length(intersectionSpace);
    float simpleUnion = opMerge(shape1, shape2);
    float outsideDistance = max(simpleUnion, radius);
    return  insideDistance + outsideDistance;
}

// shift operation to expand or contract an SDF
float opShift(float sd, float shift)
{
  return sd - shift;
}

// stroke a SDF with given thickness
// signed distance and thickness is expressed in pixels
float strokePx(float sd, float thickness)
{
  float halfStroke = 0.5 * thickness;

  return 1.0 - smoothstep(halfStroke - 1.0, halfStroke + 1.0, abs(sd));
}

// stroke a SDF with given thickness
// signed distance and thickness is expressed in object space units
float stroke(float sd, float thickness)
{
  float w = fwidth(sd);

  float halfStroke = 0.5 * thickness * w;

  return 1.0 - smoothstep(halfStroke - w, halfStroke + w, abs(sd));
}

// fill a SDF
// signed distance is expressed in pixels
float fillPx(float sd)
{
    return 1.0 - smoothstep(0.0, 1.0, sd);
}

// fill a SDF
// signed distance is expressed in object space units
float fill(float sd)
{
    float w = fwidth(sd);

    return 1.0 - smoothstep(0.0, w, sd);
}
