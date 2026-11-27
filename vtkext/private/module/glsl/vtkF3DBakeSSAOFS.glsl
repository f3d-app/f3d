//VTK::System::Dec

//VTK::Output::Dec

in vec2 texCoord;

uniform sampler2D texDepth;
uniform sampler2D texNormal;
uniform sampler2D texNoise;

uniform mat4 matProjection;
uniform mat4 matInvProjection;
uniform float kernelRadius;
uniform float kernelBias;

//VTK::KernelSize::Decl
uniform vec3 samples[KERNEL_SIZE];

vec3 viewPosition(vec2 uv, float depth)
{
  vec4 view = matInvProjection * vec4(vec3(uv, depth) * 2.0 - 1.0, 1.0);
  return view.xyz / view.w;
}

vec3 octahedralDecode(vec2 e)
{
  e = e * 2.0 - 1.0;
  vec3 n = vec3(e, 1.0 - abs(e.x) - abs(e.y));
  if (n.z < 0.0)
  {
    n.xy = (1.0 - abs(n.yx)) * vec2(n.x >= 0.0 ? 1.0 : -1.0, n.y >= 0.0 ? 1.0 : -1.0);
  }
  return normalize(n);
}

void main()
{
  float occlusion = 0.0;
  float depth = texture(texDepth, texCoord).r;

  // background has nothing to occlude
  if (depth < 1.0)
  {
    vec3 fragPosVC = viewPosition(texCoord, depth);
    vec3 normal = octahedralDecode(texture(texNormal, texCoord).rg);

    // blue noise texture tiled over the screen
    vec2 tilingShift = vec2(textureSize(texDepth, 0)) / vec2(textureSize(texNoise, 0));
    float randomAngle = 6.283185 * texture(texNoise, texCoord * tilingShift).r;
    vec3 randomVec = vec3(cos(randomAngle), sin(randomAngle), 0.0);
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    for (int i = 0; i < KERNEL_SIZE; i++)
    {
      vec3 sampleVC = fragPosVC + TBN * samples[i] * kernelRadius;
      vec4 sampleDC = matProjection * vec4(sampleVC, 1.0);
      vec2 sampleUV = (sampleDC.xy / sampleDC.w) * 0.5 + 0.5;

      float sampleDepth = texture(texDepth, sampleUV).r;
      float sampleZ = viewPosition(sampleUV, sampleDepth).z;

      float rangeCheck = smoothstep(0.0, 1.0, kernelRadius / abs(fragPosVC.z - sampleZ));
      occlusion += (sampleZ >= sampleVC.z + kernelBias ? 1.0 : 0.0) * rangeCheck;
    }
    occlusion /= float(KERNEL_SIZE);
  }

  gl_FragData[0] = vec4(vec3(1.0 - clamp(occlusion, 0.0, 1.0)), 1.0);
}
