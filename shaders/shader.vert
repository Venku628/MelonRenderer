#version 450

struct Material {
	int textureIndex;
};

layout (binding = 0) uniform UBOViewProjection {
	mat4 viewProjection;
} camera;

layout (binding = 1) uniform UBODynamicTransform {
	mat3x4 model;
} dynamicTransform;

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inTexCoord;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec2 outTexCoord;
layout (location = 2) flat out Material inMaterial;

void main() 
{
	//vec4 transformedPos = vec4(pos, 1) * obj.model;
	vec4 transformedPos = vec4(pos, 1) * mat4(dynamicTransform.model);

	gl_Position = camera.viewProjection * transformedPos;

	outColor = vec4(inColor, 1);
	outTexCoord = inTexCoord;

	//inMaterial.textureIndex = obj.textureIndex;
	inMaterial.textureIndex = 0;
}