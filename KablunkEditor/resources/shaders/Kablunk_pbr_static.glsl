#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Binormal;
layout(location = 4) in vec2 a_TexCoord;

// Transform buffer
layout(location = 5) in vec4 a_MRow0;
layout(location = 6) in vec4 a_MRow1;
layout(location = 7) in vec4 a_MRow2;

layout(std140, set = 1, binding = 0) uniform Camera
{
    mat4 u_ViewProjectionMatrix;
    mat4 u_InverseViewProjectionMatrix;
    mat4 u_ProjectionMatrix;
    mat4 u_ViewMatrix;
};

layout (std140, set = 1, binding = 7) uniform ShadowCascadesData
{
    mat4 DirLightViewMat[4];
} u_DirShadowCascades;

struct VertexOutput
{
    vec3 WorldPosition;
    vec3 Normal;
    vec2 TexCoord;
    mat3 WorldNormals;
    mat3 WorldTransform;
    vec3 Binormal;

    mat3 CameraView;
    vec3 CameraPosition;

    vec3 ViewPosition;
    vec3 ShadowMapCoords[4];
};

layout(location = 0) out VertexOutput v_Output;

void main()
{
    mat4 transform = mat4(
            vec4(a_MRow0.x, a_MRow1.x, a_MRow2.x, 0.0),
            vec4(a_MRow0.y, a_MRow1.y, a_MRow2.y, 0.0),
            vec4(a_MRow0.z, a_MRow1.z, a_MRow2.z, 0.0),
            vec4(a_MRow0.w, a_MRow1.w, a_MRow2.w, 1.0)
        );

    vec4 worldPosition = transform * vec4(a_Position, 1.0);
    v_Output.WorldPosition = worldPosition.xyz;
    v_Output.Normal = a_Normal;
    v_Output.TexCoord = vec2(a_TexCoord.x, 1.0 - a_TexCoord.y);
    v_Output.WorldNormals = mat3(a_Tangent, a_Binormal, a_Normal);
    v_Output.WorldTransform = mat3(1.0f);
    v_Output.Binormal = a_Binormal;

    v_Output.CameraView = mat3(u_ViewMatrix);
    v_Output.CameraPosition = u_CameraPosition;
    v_Output.ViewPosition = vec3(u_ViewMatrix * vec4(v_Output.WorldPosition, 1.0));

    vec4 shadowCoords[4];
    shadowCoords[0] = (u_DirShadowCascades.DirLightViewMat[0] * vec4(worldPosition.xyz, 1.0));
    shadowCoords[1] = (u_DirShadowCascades.DirLightViewMat[1] * vec4(worldPosition.xyz, 1.0));
    shadowCoords[2] = (u_DirShadowCascades.DirLightViewMat[2] * vec4(worldPosition.xyz, 1.0));
    shadowCoords[3] = (u_DirShadowCascades.DirLightViewMat[3] * vec4(worldPosition.xyz, 1.0));
    v_Output.ShadowMapCoords[0] = vec3(shadowCoords[0].xyz / (shadowCoords[0].w));
    v_Output.ShadowMapCoords[1] = vec3(shadowCoords[1].xyz / (shadowCoords[1].w));
    v_Output.ShadowMapCoords[2] = vec3(shadowCoords[2].xyz / (shadowCoords[2].w));
    v_Output.ShadowMapCoords[3] = vec3(shadowCoords[3].xyz / (shadowCoords[3].w));

    gl_Position = u_ViewProjectionMatrix * worldPosition;
}

#type fragment
#version 450 core

// Constant normal incidence Fresnel factor for all dielectrics.
const vec3 Fdielectric = vec3(0.04);

const float PI = 3.141592;
const float TwoPI = 2 * PI;
const float Epsilon = 0.00001;

struct VertexOutput
{
    vec3 WorldPosition;
    vec3 Normal;
    vec2 TexCoord;
    mat3 WorldNormals;
    mat3 WorldTransform;
    vec3 Binormal;

    mat3 CameraView;
    vec3 CameraPosition;

    vec3 ViewPosition;
    vec3 ShadowMapCoords[4];
};

struct PBRParameters
{
	vec3 Albedo;
	float Roughness;
	float Metalness;

	vec3 Normal;
	vec3 View;
	float NdotV;
} m_Params;

layout(location = 0) in VertexOutput v_Input;

layout(location = 0) out vec4 o_Color;

// PBR texture inputs
layout(set = 0, binding = 0) uniform sampler2D u_AlbedoTexture;
layout(set = 0, binding = 1) uniform sampler2D u_NormalTexture;
layout(set = 0, binding = 2) uniform sampler2D u_MetalnessTexture;
layout(set = 0, binding = 3) uniform sampler2D u_RoughnessTexture;
layout(set = 0, binding = 4) uniform sampler2DArray u_ShadowMapTexture;

const int MAX_POINT_LIGHT_COUNT = 128;

struct PointLight
{
    vec3 Position;
    float Multiplier;
    // Color
    vec3 Radiance;
    float Radius;
    float MinRadius;
    float Falloff;

    // TODO: angles

    vec2 Padding;
};

layout(std140, set = 1, binding = 1) uniform PointLightsData
{
    uint Count;
    PointLight Lights[MAX_POINT_LIGHT_COUNT];
} u_PointLights;

layout(push_constant) uniform MaterialPBR
{
	vec3 AlbedoColor;
	float Metalness;
	float Roughness;
	float Emission;
	float AmbientOcclusion
	
	bool UseNormalMap;
} u_MaterialUniforms;



void main()
{
	vec3 N = normalize(v_Input.Normal);
	vec3 V = normalize(v_Input.CameraPosition - v_Input.WorldPosition);

	// Calculate direct point light irradiance.
	// To satisfy the reflectance equation, we sum each lights individual radiance scaled by BRDF and the light's incident angle.
	vec3 Lo = vec3(0.0);
	for (int i = 0; i < u_PointLights.Count; ++i) {
		vec3 lightPos = u_PointLights[i].Position;
		vec3 L = normalize(u_PointLights[i].Position - v_Input.WorldPosition);
		vec3 H = normalize(V + L);

		float distance = length(lightPos - v_Input.WorldPosition);
		float attenuation = 1.0 / (distance * distance);
		vec3 radiance = u_PointLights[i].Radiance * attenuation;
		// TODO:
	}


}