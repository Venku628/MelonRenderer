#version 450

layout (std140, binding = 0) uniform bufferVals {
	mat4 viewProjection;
} myBufferVals;
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inTexCoord;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec2 outTexCoord;

layout(push_constant) uniform PER_OBJECT
{
    mat4 model;
} obj;

void main() 
{
	vec4 transformedPos = vec4(pos, 1) * obj.model;

	gl_Position = myBufferVals.viewProjection * transformedPos;
	outColor = vec4(inColor, 1);
	outTexCoord = inTexCoord;
}