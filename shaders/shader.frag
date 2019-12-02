#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

struct Material {
	int textureIndex;
};

layout(binding = 1) uniform sampler2D texSampler[];

layout (location = 0) in vec4 inColor;
layout (location = 1) in vec2 inTexCoord;
layout (location = 2) flat in Material inMaterial;

layout (location = 0) out vec4 outColor;

void main() 
{
	outColor = texture(texSampler[inMaterial.textureIndex], inTexCoord);
}
