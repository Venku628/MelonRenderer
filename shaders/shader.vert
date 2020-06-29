#version 450

layout(binding = 0) uniform CameraProperties
{
mat4 view;
mat4 projection;
mat4 viewInverse;
mat4 projectionInverse;
} cam;

layout (binding = 1) uniform ObjectData {
  int  objId;
  int  txtOffset;
  mat4 transfo;
  mat4 transfoIT;
} object;

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in uint matID;

layout (location = 0) out vec2 outTexCoord;
layout (location = 1) flat out uint outMaterial;

void main() 
{
	vec4 transformedPos = vec4(pos, 1) * object.transfo;

	gl_Position = cam.projection * cam.view * transformedPos;

	outTexCoord = inTexCoord;
	outMaterial = matID;
}