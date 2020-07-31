#version 450

layout(binding = 0) uniform CameraProperties
{
mat4 view;
mat4 projection;
mat4 viewInverse;
mat4 projectionInverse;
} cam;

layout (binding = 1) uniform ObjectData {
  mat4 transfo;
  mat4 transfoIT;
  uint  objId;
  uint  txtOffset;
} object;

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in uint matID;

layout (location = 0) out vec3 outPos;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outViewPos;
layout (location = 3) out vec2 outTexCoord;
layout (location = 4) flat out uint outMaterial;
layout (location = 5) flat out uint outObjId;

void main() 
{
	vec4 transformedPos = vec4(pos, 1);

	gl_Position = cam.projection * cam.view * object.transfo * transformedPos;

	//redundant calculation?
	outPos = vec3(object.transfo * transformedPos);
	outNormal = normal;
	outViewPos = cam.view[3].xyz;
	outViewPos = vec3(0, 0, 0); //debug, specular calculation not working correctly
	outTexCoord = inTexCoord;
	outMaterial = matID;
	outObjId = object.objId;
}