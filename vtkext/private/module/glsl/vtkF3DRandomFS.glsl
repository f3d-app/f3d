//VTK::Color::Dec

// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

uint hash( uvec3 v ) { return hash( v.x ^ hash(v.y ^ hash(v.z))); }

float random_ign(vec2 coord, uint index)
{
    coord = floor(coord) + 5.588238 * vec2(index % 4096u);
    return fract(fract(dot(coord, vec2(0.06711056, 0.00583715))) * 52.9829189);
}
