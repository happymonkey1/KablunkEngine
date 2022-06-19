#type vertex
#version 450 core

layout(std140, binding = 1) uniform Camera
{
    mat4 u_ViewProjectionMatrix;
    mat4 u_InverseViewProjectionMatrix;
    mat4 u_ProjectionMatrix;
    mat4 u_ViewMatrix;
};

layout(location = 1) in vec3 a_Position;
layout(location = 2) in vec3 a_Normal;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Binormal;
layout(location = 5) in vec2 a_TexCoord;

layout(push_constant) uniform Transform
{
    mat4 Transform;
} u_Renderer;

struct VertexOutput
{
    vec3 WorldPosition;
    vec3 Normal;
    vec2 TexCoord;
    mat3 WorldNormals;
    mat3 WorldTransform;
    vec3 Binormal;

    mat3 CameraView;

    vec3 ViewPosition;
};

layout(location = 0) out VertexOutput Output;

void main()
{
    vec4 worldPosition = u_Renderer.Transform * vec4(a_Position, 1.0);
    Output.WorldPosition = worldPosition.xyz;
    Output.Normal = mat3(u_Renderer.Transform) * a_Normal;
    Output.TexCoord = vec2(a_TexCoord.x, 1.0 - a_TexCoord.y);
    Output.WorldNormals = mat3(u_Renderer.Transform) * mat3(a_Tangent, a_Binormal, a_Normal);
    Output.WorldTransform = mat3(u_Renderer.Transform);
    Output.Binormal = a_Binormal;

    Output.CameraView = mat3(u_ViewMatrix);
    Output.ViewPosition = vec3(u_ViewMatrix * vec4(Output.WorldPosition, 1.0));

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

    vec3 ViewPosition;
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

layout(location = 0) in VertexOutput Input;

layout(location = 0) out vec4 o_Color;
layout(location = 1) out vec4 o_ViewNormals;
//layout(location = 2) out vec4 o_ViewPosition;

// PBR texture inputs
layout(set = 0, binding = 5) uniform sampler2D u_AlbedoTexture;
layout(set = 0, binding = 6) uniform sampler2D u_NormalTexture;
layout(set = 0, binding = 7) uniform sampler2D u_MetalnessTexture;
layout(set = 0, binding = 8) uniform sampler2D u_RoughnessTexture;

// BRDF LUT
layout(set = 1, binding = 9) uniform sampler2D u_BRDFLUTTexture;

layout(push_constant) uniform Material
{
	vec3 AlbedoColor;
	float Metalness;
	float Roughness;
	float Emission;

	float EnvMapRotation;
	
	bool UseNormalMap;
} u_MaterialUniforms;

// Shlick's approximation of the Fresnel factor.
vec3 FresnelSchlick(vec3 F0, float cosTheta)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 FresnelSchlickRoughness(vec3 F0, float cosTheta, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

// wtf is this?
vec3 IBL(vec3 F0, vec3 Lr)
{
	//vec3 irradiance = texture(u_EnvIrradianceTex, m_Params.Normal).rgb;
    vec3 irradiance = vec3(1.0);
	vec3 F = FresnelSchlickRoughness(F0, m_Params.NdotV, m_Params.Roughness);
	vec3 kd = (1.0 - F) * (1.0 - m_Params.Metalness);
	vec3 diffuseIBL = m_Params.Albedo * irradiance;

	//int envRadianceTexLevels = textureQueryLevels(u_EnvRadianceTex);
    int envRadianceTexLevels = 1;
	float NoV = clamp(m_Params.NdotV, 0.0, 1.0);
	vec3 R = 2.0 * dot(m_Params.View, m_Params.Normal) * m_Params.Normal - m_Params.View;
	vec3 specularIrradiance = textureLod(u_EnvRadianceTex, RotateVectorAboutY(u_MaterialUniforms.EnvMapRotation, Lr), (m_Params.Roughness) * envRadianceTexLevels).rgb;
	//specularIrradiance = vec3(Convert_sRGB_FromLinear(specularIrradiance.r), Convert_sRGB_FromLinear(specularIrradiance.g), Convert_sRGB_FromLinear(specularIrradiance.b));

	// Sample BRDF Lut, 1.0 - roughness for y-coord because texture was generated (in Sparky) for gloss model
	vec2 specularBRDF = texture(u_BRDFLUTTexture, vec2(m_Params.NdotV, 1.0 - m_Params.Roughness)).rg;
	vec3 specularIBL = specularIrradiance * (F0 * specularBRDF.x + specularBRDF.y);

	return kd * diffuseIBL + specularIBL;
}

vec3 RotateVectorAboutY(float angle, vec3 vec)
{
	angle = radians(angle);
	mat3x3 rotationMatrix = { vec3(cos(angle),0.0,sin(angle)),
							vec3(0.0,1.0,0.0),
							vec3(-sin(angle),0.0,cos(angle)) };
	return rotationMatrix * vec;
}

float Convert_sRGB_FromLinear(float theLinearValue)
{
	return theLinearValue <= 0.0031308f
		? theLinearValue * 12.92f
		: pow(theLinearValue, 1.0f / 2.4f) * 1.055f - 0.055f;
}



void main()
{
    // Standard PBR inputs
	vec4 albedoTexColor = texture(u_AlbedoTexture, Input.TexCoord);
	m_Params.Albedo = albedoTexColor.rgb * u_MaterialUniforms.AlbedoColor;
	float alpha = albedoTexColor.a;
	m_Params.Metalness = texture(u_MetalnessTexture, Input.TexCoord).r * u_MaterialUniforms.Metalness;
	m_Params.Roughness = texture(u_RoughnessTexture, Input.TexCoord).r * u_MaterialUniforms.Roughness;
	o_MetalnessRoughness = vec4(m_Params.Metalness, m_Params.Roughness, 0.f, 1.f);
	m_Params.Roughness = max(m_Params.Roughness, 0.05); // Minimum roughness of 0.05 to keep specular highlight
    
    // Normals (either from vertex or map)
	m_Params.Normal = normalize(Input.Normal);
	if (u_MaterialUniforms.UseNormalMap)
	{
		m_Params.Normal = normalize(texture(u_NormalTexture, Input.TexCoord).rgb * 2.0f - 1.0f);
		m_Params.Normal = normalize(Input.WorldNormals * m_Params.Normal);
	}
	// View normals
	o_ViewNormals.xyz = Input.CameraView * m_Params.Normal;

	m_Params.View = normalize(u_Scene.CameraPosition - Input.WorldPosition);
	m_Params.NdotV = max(dot(m_Params.Normal, m_Params.View), 0.0);

    // Specular reflection vector
	vec3 Lr = 2.0 * m_Params.NdotV * m_Params.Normal - m_Params.View;

    // Fresnel reflectance, metals use albedo
	vec3 F0 = mix(Fdielectric, m_Params.Albedo, m_Params.Metalness);

    // #TODO lights
    vec3 lightContribution = vec3(0.3);
    lightContribution += m_Params.Albedo * u_MaterialUniforms.Emission;

    o_Color = vec4(lightContribution, 1.0);
}