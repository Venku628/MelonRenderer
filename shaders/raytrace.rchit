#version 460
#extension GL_NV_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_GOOGLE_include_directive : enable
#include "raycommon.glsl"
#include "wavefront.glsl"

layout(binding = 0, set = 0) uniform accelerationStructureNV topLevelAS;
layout(binding = 3, set = 0, scalar) buffer MatColorBufferObject { WaveFrontMaterial m[]; } materials[];
layout(binding = 4, set = 0, scalar) buffer ScnDesc { sceneDesc i[]; } scnDesc;
layout(binding = 5, set = 0) uniform sampler2D textureSamplers[];
layout(binding = 6, set = 0, scalar) buffer Vertices { Vertex v[]; } vertices[];
layout(binding = 7, set = 0) buffer Indices { uint i[]; } indices[];

layout(location = 0) rayPayloadInNV hitPayload prd;
layout(location = 1) rayPayloadNV bool isShadowed;
hitAttributeNV vec3 attribs;

layout(push_constant) uniform Constants
{
  vec4  clearColor;
  vec3  lightPosition;
  float lightIntensity;
  int   samples;
} pushC;

layout(shaderRecordNV) buffer SBTData {
  uint geometryID;
};

void main()
{
  uint objId = scnDesc.i[nonuniformEXT(geometryID)].objId;

    // Indices of the triangle
  ivec3 ind = ivec3(indices[objId].i[3 * gl_PrimitiveID + 0],  
                    indices[objId].i[3 * gl_PrimitiveID + 1],
                    indices[objId].i[3 * gl_PrimitiveID + 2]);
  // Vertex of the triangle
  Vertex v0 = vertices[objId].v[ind.x];
  Vertex v1 = vertices[objId].v[ind.y];
  Vertex v2 = vertices[objId].v[ind.z];

  //interpolate the vertices
  const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

  // Computing the normal at hit position
  vec3 normal = v0.nrm * barycentrics.x + v1.nrm * barycentrics.y + v2.nrm * barycentrics.z;
  // Transforming the normal to world space
  normal = normalize(vec3(scnDesc.i[nonuniformEXT(geometryID)].transfoIT * vec4(normal, 0.0)));

  // Computing the coordinates of the hit position
  vec3 worldPos = gl_WorldRayOriginNV + gl_WorldRayDirectionNV * gl_HitTNV;


  // Vector toward the light
  //TODO: other types of lights
  vec3 lDir = pushC.lightPosition - worldPos;
  float lightDistance   = length(lDir);
  //lightIntensity = pushC.lightIntensity / lightDistance * lightDistance; //this is a way too harsh falloff
  vec3 L = normalize(lDir);
  

  // Material of the object
  WaveFrontMaterial mat = materials[objId].m[v0.matID]; 

  // Diffuse
  vec3 diffuse;

  // Lambertian
  if(mat.illum >= 1)
  {
    float dotNL = max(dot(normal, L), 0.0);
    vec3  c     = vec3(mat.diffuse) * dotNL;
    diffuse = c + vec3(mat.ambient);
  }
  else
  {
    diffuse = vec3(0, 0, 0);
  }

  if(mat.textureId >= 0)
  {
    vec2 texCoord = v0.texCoord * barycentrics.x + v1.texCoord * barycentrics.y + v2.texCoord * barycentrics.z;
    diffuse *= texture(textureSamplers[mat.textureId], texCoord).xyz;
  }

  vec3  specular    = vec3(0);
  float attenuation = 1;

  // Tracing shadow ray only if the light is visible from the surface
  if(dot(normal, L) > 0)
  {
    float tMin   = 0.001;
    float tMax   = lightDistance;
    uint  flags = gl_RayFlagsTerminateOnFirstHitNV | gl_RayFlagsOpaqueNV | gl_RayFlagsSkipClosestHitShaderNV;
    isShadowed = true;

    traceNV(topLevelAS,  // acceleration structure
            flags,       // rayFlags
            0xFF,        // cullMask
            0,           // sbtRecordOffset
            0,           // sbtRecordStride
            1,           // missIndex
            worldPos,    // ray origin
            tMin,        // ray min range
            L,           // ray direction
            tMax,        // ray max range
            1            // payload (location = 1)
    );

    if(isShadowed)
    {
      attenuation = 0.3;
    }
    else
    {
      // Specular
      specular = computeSpecular(mat, gl_WorldRayDirectionNV, L, normal);
    }
  }
  else
  {
  //this was missing from the nvidia tutorial, no shadow for faces facing away from light otherwise
    attenuation = 0.3;
  }

  //reflection
  if(mat.illum == 3)
  {
    vec3 rayDir = reflect(gl_WorldRayDirectionNV, normal);
    prd.attenuation *= vec3(mat.specular);
    prd.done      = 0; //hit reflective surface, continue tracing rays
    prd.rayOrigin = worldPos;
    prd.rayDir    = rayDir;
  }

  prd.hitValue = vec3(pushC.lightIntensity * attenuation * (diffuse + specular));
}
