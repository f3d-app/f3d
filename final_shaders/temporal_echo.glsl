// ---------------------------------------------------------------------------
// Name    : Temporal echo
// Authors : ThoomasGM
// Ref     : Original work
// License : BSD-3-Clause (F3D)
// ---------------------------------------------------------------------------

uniform float trigger = 0.06;      // slider(0.0, 0.5)
uniform float response = 0.12;     // slider(0.01, 1.0)
uniform float persistence = 0.75;  // slider(0.0, 1.0)
uniform float rgbSplitX = 0.003;   // slider(0.0, 0.05)
uniform float rgbSplitY = 0.0015;  // slider(0.0, 0.05)


// Temporal with the entire frame and rgb différent offsets
vec4 pixel(vec2 uv)
{
    vec4 current = texture(source, uv);
    vec4 pastClean = texture(history, uv);

    // If the model is not moving, put rgb back
    // Motion delta
    float motion = length(current.rgb - pastClean.rgb);

    if (motion < trigger)
    {
        return current;
    }

    // Speed factor [0, 1]
    float motionFactor = smoothstep(trigger, response, motion);
    
    // Trail decay (0 if still)
    float dynamicDecay = mix(0.0, persistence, motionFactor);

    // Only during movement
    vec2 offset = vec2(rgbSplitX, rgbSplitY) * motionFactor;

    float r = texture(history, uv + offset).r;
    float g = pastClean.g;
    float b = texture(history, uv - offset).b;

    // Rebuilt past frame
    vec4 past = vec4(r, g, b, 1.0);

    return mix(current, past, dynamicDecay);
}
