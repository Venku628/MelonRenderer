struct hitPayload
{
  vec3 attenuation;
  vec3 rayOrigin;
  vec3 rayDir;
  vec3 hitValue;
  int done;
  int depth;
};

struct sceneDesc
{
  mat4 transfo;
  mat4 transfoIT;
  int  objId;
  int  txtOffset;
};

struct Vertex
{
  vec3 pos;
  vec3 nrm;
  vec2 texCoord;
  uint matID;
};