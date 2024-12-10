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
    mat4 u_ProjectionMatrix;
    mat4 u_ViewMatrix;
    vec3 u_CameraPosition;
};

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

    gl_Position = u_ViewProjectionMatrix * worldPosition;
}

#type fragment
#version 450 core

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
};

layout(location = 0) in VertexOutput v_Input;

layout(location = 0) out vec4 o_Color;

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

layout(set = 0, binding = 5) uniform sampler2D u_AlbedoTexture;
layout(set = 0, binding = 6) uniform sampler2D u_NormalTexture;

layout(std140, set = 1, binding = 1) uniform PointLightsData
{
    uint Count;
    PointLight Lights[128];
} u_PointLights;

// Directional light represents an "infintely" far away sun
layout(std140, set = 1, binding = 2) uniform DirectionalLightData
{
    // Light direction
    vec3 Direction;
    // Multiplier for the radiance
    float Multiplier;
    // Color
    vec3 Radiance;
    // Whether the directional light is enabled
    bool Enabled;
} u_DirectionalLight;

layout(std140, push_constant) uniform Material
{
	float AmbientStrength;
    float DiffuseStrength;
    float SpecularStrength;
    vec3 AlbedoColor;
    float Metalness;
	float Roughness;
	float Emission;
    bool UseNormalMap;
} u_MaterialUniforms;

vec3 GetPointLightAttenuationValues(in float distance)
{
    // Values courtesy of Orge3D's wiki: https://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation
    if (distance <= 7)
    {
        return vec3(1.0, 0.7, 1.8);
    }
    else if (distance <= 13)
    {
        return vec3(1.0, 0.35, 0.44);
    }
    else if (distance <= 20)
    {
        return vec3(1.0, 0.22, 0.2);
    }
    else if (distance <= 32)
    {
        return vec3(1.0, 0.14, 0.07);
    }
    else if (distance <= 50)
    {
        return vec3(1.0, 0.09, 0.032);
    }
    else if (distance <= 65)
    {
        return vec3(1.0, 0.07, 0.017);
    }
    else if (distance <= 100)
    {
        return vec3(1.0, 0.045, 0.0075);
    }
    else if (distance <= 160)
    {
        return vec3(1.0, 0.027, 0.0028);
    }
    else if (distance <= 200)
    {
        return vec3(1.0, 0.022, 0.0019);
    }
    else if (distance <= 325)
    {
        return vec3(1.0, 0.014, 0.0007);
    }
    else if (distance <= 600)
    {
        return vec3(1.0, 0.007, 0.0002);
    }
    else 
    {
        return vec3(1.0, 0.0014, 0.000007);
    }
}


vec3 CalculatePointLights(in vec3 normal, in vec3 viewDir)
{
    vec3 albedoColor = texture(u_AlbedoTexture, v_Input.TexCoord).rgb * u_MaterialUniforms.AlbedoColor;

    vec3 result = vec3(0.0);
    for (int i = 0; i < u_PointLights.Count; i++)
    {
        PointLight light = u_PointLights.Lights[i];
        float distance = length(light.Position - v_Input.WorldPosition);
        vec3 attenuationValues = GetPointLightAttenuationValues(distance);
        float constant = attenuationValues.x;
        float linear = attenuationValues.y;
        float quadratic = attenuationValues.z;

        float attenuation = clamp(1.0 / (1 + (2.0 / light.Radius) * distance + (1.0 / (light.Radius * light.Radius)) * (distance * distance)), 0.0, 1.0);

        vec3 radiance = light.Radiance * light.Multiplier * albedoColor;

        // Diffuse
        vec3 lightDir = normalize(light.Position - v_Input.WorldPosition);
        float diffuseImpact = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diffuseImpact * u_MaterialUniforms.DiffuseStrength * radiance;

        // Specular
        float shininess = 32;
        // Blinn-Phong 
        vec3 halfDir = normalize(lightDir + viewDir);
        float specularImpact = pow(max(dot(normal, halfDir), 0.0), shininess);
        vec3 specular = u_MaterialUniforms.SpecularStrength * specularImpact * radiance;

        result += (diffuse + specular) * attenuation;
        //result += vec3(light.MinRadius);
    }

    return result;
}

mat3 cotangent(vec3 N, vec3 p, vec2 uv)
{
  // get edge vectors of the pixel triangle
  vec3 dp1 = dFdx(p);
  vec3 dp2 = dFdy(p);
  vec2 duv1 = dFdx(uv);
  vec2 duv2 = dFdy(uv);

  // solve the linear system
  vec3 dp2perp = cross(dp2, N);
  vec3 dp1perp = cross(N, dp1);
  vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
  vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

  // construct a scale-invariant frame 
  float invmax = 1.0 / sqrt(max(dot(T,T), dot(B,B)));
  return mat3(normalize(T * invmax), normalize(B * invmax), N);
}

vec3 perturb(vec3 normalMap, vec3 normal, vec3 view, vec2 texCoord)
{
    mat3 TBN = cotangent(normal, -view, texCoord);
    return normalize(TBN * normalMap);
}

void main()
{
    vec3 color = texture(u_AlbedoTexture, v_Input.TexCoord).rgb;
    vec3 normalMap = texture(u_NormalTexture, v_Input.TexCoord).rgb * 2.0 - 1.0;

    // Ambient
    vec3 albedoColor = texture(u_AlbedoTexture, v_Input.TexCoord).rgb * u_MaterialUniforms.AlbedoColor;
    // vec3 ambient = u_MaterialUniforms.AmbientStrength * albedoColor;
    vec3 ambient = u_MaterialUniforms.AmbientStrength * albedoColor;

    vec3 viewDir = normalize(v_Input.CameraPosition - v_Input.WorldPosition);
    vec3 normal = normalize(v_Input.Normal);
    // TODO: https://learnopengl.com/Advanced-Lighting/Normal-Mapping
    // vec3 normal = perturb(normalMap, normalize(v_Input.Normal), viewDir, v_Input.TexCoord);

    // ===========================
    // Calculate directional light
    // ===========================
    vec3 dirLightDirection = normalize(-u_DirectionalLight.Direction);
    vec3 dirLightRadiance = u_DirectionalLight.Multiplier * u_DirectionalLight.Radiance * albedoColor;

    // Diffuse
    float diffuseImpact = max(dot(normal, dirLightDirection), 0.0);
    vec3 diffuse = diffuseImpact * u_MaterialUniforms.DiffuseStrength * dirLightRadiance;

    // Specular
    float shininess = 32;
    // Blinn-Phong 
    vec3 halfDir = normalize(dirLightDirection + viewDir);
    float specularImpact = pow(max(dot(normal, halfDir), 0.0), shininess);
    vec3 specular = u_MaterialUniforms.SpecularStrength * specularImpact * dirLightRadiance;
    vec3 directionalLightColor = vec3(0.0);
    if (u_DirectionalLight.Enabled) {
        directionalLightColor = diffuse + specular;
    }
    // ===========================

    // Calculate point lighting
    vec3 pLightsColor = CalculatePointLights(normal, viewDir);

    o_Color = vec4(ambient + directionalLightColor + pLightsColor, 1.0);
}