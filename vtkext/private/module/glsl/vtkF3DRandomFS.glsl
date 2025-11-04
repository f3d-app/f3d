//VTK::Color::Dec

// Copyright Spatial 05 July 2013
// https://stackoverflow.com/a/17479300
// CC BY-SA 3.0

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

// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );       // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}

// Pseudo-random value in half-open range [0:1].
float random( vec3  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( float  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( uint  v ) { return floatConstruct(hash(v)); }

float random_ign(vec2 coord, uint index)
{
    coord = floor(coord) + 5.588238 * vec2(index % 2048);
    return fract(fract(dot(coord, vec2(0.06711056, 0.00583715))) * 52.9829189);
}

float random_roberts(uvec2 p, uint index)
{  
    p.x += index % 4096;
    uvec2 umagic = uvec2(3242174889u, 2447445413u);
    return float(p.x * umagic.x + p.y * umagic.y) * exp2(-32.0);
}
