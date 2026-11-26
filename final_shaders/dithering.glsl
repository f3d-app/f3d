// ---------------------------------------------------------------------------
// Name    : Dithering
// Authors : Erwan Le Grand
// Ref     : The Art of Dithering and Retro Shading for the Web by Maxime Heckel
//           https://blog.maximeheckel.com/posts/the-art-of-dithering-and-retro-shading-web/
//           (code from https://github.com/MaximeHeckel/blog.maximeheckel.com,
//            core/components/MDX/Widgets/Retro/pixel.ts)
// License : MIT (original code), see notice below
// ---------------------------------------------------------------------------
// Copyright (c) 2024 Maxime Heckel
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// ---------------------------------------------------------------------------

uniform float colorNum = 2.0; // slider(2.0, 16.0)
uniform float bias = 0.88; // slider(-5.0, 5.0)
uniform float pixelSize = 1.0; // slider(1.0, 16.0)

const float bayerMatrix8x8[64] = float[64](
    0.0/ 64.0, 48.0/ 64.0, 12.0/ 64.0, 60.0/ 64.0,  3.0/ 64.0, 51.0/ 64.0, 15.0/ 64.0, 63.0/ 64.0,
    32.0/ 64.0, 16.0/ 64.0, 44.0/ 64.0, 28.0/ 64.0, 35.0/ 64.0, 19.0/ 64.0, 47.0/ 64.0, 31.0/ 64.0,
    8.0/ 64.0, 56.0/ 64.0,  4.0/ 64.0, 52.0/ 64.0, 11.0/ 64.0, 59.0/ 64.0,  7.0/ 64.0, 55.0/ 64.0,
    40.0/ 64.0, 24.0/ 64.0, 36.0/ 64.0, 20.0/ 64.0, 43.0/ 64.0, 27.0/ 64.0, 39.0/ 64.0, 23.0/ 64.0,
    2.0/ 64.0, 50.0/ 64.0, 14.0/ 64.0, 62.0/ 64.0,  1.0/ 64.0, 49.0/ 64.0, 13.0/ 64.0, 61.0/ 64.0,
    34.0/ 64.0, 18.0/ 64.0, 46.0/ 64.0, 30.0/ 64.0, 33.0/ 64.0, 17.0/ 64.0, 45.0/ 64.0, 29.0/ 64.0,
    10.0/ 64.0, 58.0/ 64.0,  6.0/ 64.0, 54.0/ 64.0,  9.0/ 64.0, 57.0/ 64.0,  5.0/ 64.0, 53.0/ 64.0,
    42.0/ 64.0, 26.0/ 64.0, 38.0/ 64.0, 22.0/ 64.0, 41.0/ 64.0, 25.0/ 64.0, 37.0/ 64.0, 21.0 / 64.0
);

vec3 dither(vec2 uv, vec3 color) {
    int x = int(uv.x * resolution.x) % 8;
    int y = int(uv.y * resolution.y) % 8;
    float threshold = bayerMatrix8x8[y * 8 + x] - bias;

    color.rgb += threshold;
    color.r = floor(color.r * (colorNum - 1.0) + 0.5) / (colorNum - 1.0);
    color.g = floor(color.g * (colorNum - 1.0) + 0.5) / (colorNum - 1.0);
    color.b = floor(color.b * (colorNum - 1.0) + 0.5) / (colorNum - 1.0);

    return color;
}

vec4 pixel(vec2 uv) {
    vec2 normalizedPixelSize = pixelSize / resolution;  
    vec2 uvPixel = normalizedPixelSize * floor(uv / normalizedPixelSize);
    vec4 color = texture(source, uvPixel);
    color.rgb = dither(uvPixel, color.rgb);

    return color;
}