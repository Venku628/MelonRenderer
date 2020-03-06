struct hitPayload
{
  vec3 hitValue;
};

struct sceneDesc
{
  int  objId;
  int  txtOffset;
  mat4 transfo;
  mat4 transfoIT;
};

struct Vertex
{
  vec3 pos;
  vec3 nrm;
  vec2 texCoord;
};