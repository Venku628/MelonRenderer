#version 460
#extension GL_NV_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_GOOGLE_include_directive : enable
#include "raycommon.glsl"
#include "wavefront.glsl"

layout(location = 0) rayPayloadInNV hitPayload prd;
hitAttributeNV vec3 attribs;

layout(binding = 4, set = 0, scalar) buffer ScnDesc { sceneDesc i[]; } scnDesc;
layout(binding = 5, set = 0) uniform sampler2D textureSamplers[];
layout(binding = 6, set = 0, scalar) buffer Vertices { Vertex v[]; } vertices[];
layout(binding = 7, set = 0) buffer Indices { uint i[]; } indices[];

layout(push_constant) uniform Constants
{
  vec4  clearColor;
  vec3  lightPosition;
  float lightIntensity;
  int   lightType;
} pushC;

void main()
{
//scene description not implemented yet
  uint objId = scnDesc.i[gl_InstanceID].objId;

    // Indices of the triangle
  ivec3 ind = ivec3(indices[objId].i[3 * gl_PrimitiveID + 0],   //
                    indices[objId].i[3 * gl_PrimitiveID + 1],   //
                    indices[objId].i[3 * gl_PrimitiveID + 2]);  //
  // Vertex of the triangle
  Vertex v0 = vertices[objId].v[ind.x];
  Vertex v1 = vertices[objId].v[ind.y];
  Vertex v2 = vertices[objId].v[ind.z];

  //interpolate the vertex normal
  const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
  // Computing the normal at hit position
  vec3 normal = v0.nrm * barycentrics.x + v1.nrm * barycentrics.y + v2.nrm * barycentrics.z;
  // Transforming the normal to world space
  normal = normalize(vec3(scnDesc.i[gl_InstanceID].transfoIT * vec4(normal, 0.0)));

  //alternativley, interpolate the position, like for the normal
  vec3 worldPos = gl_WorldRayOriginNV + gl_WorldRayDirectionNV * gl_HitTNV;



  // Vector toward the light
  vec3  L;
  float lightIntensity = pushC.lightIntensity;
  float lightDistance  = 100000.0;
  // Point light
  if(pushC.lightType == 0)
  {
    vec3 lDir      = pushC.lightPosition - worldPos;
    lightDistance  = length(lDir);
    lightIntensity = pushC.lightIntensity / (lightDistance * lightDistance);
    L              = normalize(lDir);
  }
  else // Directional light
  {
    L = normalize(pushC.lightPosition - vec3(0));
  }

  float dotNL = max(dot(normal, L), 0.2);

  vec2 uv = v0.texCoord * barycentrics.x + v1.texCoord * barycentrics.y + v2.texCoord * barycentrics.z;

  prd.hitValue = texture(textureSamplers[1], uv).zyx * vec3(dotNL);
}
