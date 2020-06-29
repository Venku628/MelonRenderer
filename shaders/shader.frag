#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_GOOGLE_include_directive : enable
#include "wavefront.glsl"

struct Material {
	int textureIndex;
};

layout(binding = 2, set = 0, scalar) buffer MatColorBufferObject { WaveFrontMaterial m[]; } materials[]; 
layout(binding = 3) uniform sampler2D texSampler[];

layout (location = 0) in vec2 inTexCoord;
layout (location = 1) flat in uint inMaterial;

layout (location = 0) out vec4 outColor;

void main() 
{
	outColor = texture(texSampler[inMaterial], inTexCoord);
	//outColor = vec4(1.0, 1.0, 1.0, 1.0);
}
