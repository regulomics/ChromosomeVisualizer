#version 410 core

layout(location = 0) in vec3 vInstancePosition;
layout(location = 1) in int iInstanceFlags;
layout(location = 3) in float fInstanceRadius;
layout(location = 4) in int iMaterialID;

struct Material
{
    uint cColor;
    uint cSpecularColor;
    float fSpecularExponent;
};

layout (std140) uniform material_data
{
    uvec4 materials[50];
};

flat out vec3 gvInstancePosition;
flat out int giFlags;
flat out vec4 gcColor;
flat out vec3 gcSpecularColor;
flat out float gfSpecularExponent;
flat out float gfInstanceRadius;

void main() {
    gvInstancePosition = vInstancePosition;
    giFlags = iInstanceFlags;
    gcColor = unpackUnorm4x8(materials[iMaterialID].x).bgra;
    gcSpecularColor = unpackUnorm4x8(materials[iMaterialID].y).bgr;
    gfSpecularExponent = uintBitsToFloat(materials[iMaterialID].z);
    gfInstanceRadius = fInstanceRadius;
}
