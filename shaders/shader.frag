#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_GOOGLE_include_directive : enable
#include "wavefront.glsl"

layout(binding = 2, set = 0, scalar) buffer MatColorBufferObject { WaveFrontMaterial m[]; } materials[]; 
layout(binding = 3) uniform sampler2D texSampler[];

layout(push_constant) uniform Constants
{
  vec4  lightPosition;
  float lightIntensity;
} pushC;

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inViewPos;
layout (location = 3) in vec2 inTexCoord;
layout (location = 4) flat in uint inMaterial;
layout (location = 5) flat in uint inObjId;

layout (location = 0) out vec4 outColor;

void main() 
{
	WaveFrontMaterial mat = materials[inObjId].m[inMaterial];

	vec3 lightDir = normalize(vec3(pushC.lightPosition) - inPos);

	vec3 diffuse = texture(texSampler[mat.textureId], inTexCoord).xyz * max(dot(inNormal, lightDir), 0.0) * vec3(1.0, 1.0, 1.0);

	vec3 specular = computeSpecular(mat, normalize(inViewPos - inPos), lightDir, inNormal);

	outColor = vec4( specular, 1.0);
}
