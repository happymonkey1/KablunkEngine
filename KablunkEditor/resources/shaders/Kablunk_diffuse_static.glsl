#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Binormal;
layout(location = 4) in vec2 a_TexCoord;
layout(location = 5) in int a_EntityID; // TODO remove when ray cast mouse picking added to editor

layout(std140, binding = 0) uniform Camera
{
    mat4 u_ViewProjectionMatrix;
    mat4 u_ProjectionMatrix;
    mat4 u_ViewMatrix;
    vec3 u_CameraPosition;
};

// This is prob really bad performance wise
layout(std140, binding = 1) uniform Renderer
{
    uniform mat4 u_Transform;
};


// Material
//layout(set = 0, binding = 2) uniform sampler2D DiffuseMap;
//layout(set = 0, binding = 3) uniform sampler2D SpecularMap;
//layout(std140, binding = 4) uniform MaterialShininess
//{
//    float Shininess;
//};

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

layout(location = 0) out VertexOutput v_Input;
layout(location = 15) out vec3 v_Color;
layout(location = 16) flat out int v_EntityID;

void main()
{
    v_Input.WorldPosition = vec3(u_Transform * vec4(a_Position, 1.0));
    v_Input.Normal = mat3(u_Transform) * a_Normal;
    v_Input.TexCoord = vec2(a_TexCoord.x, 1.0 - a_TexCoord.y);
    v_Input.WorldNormals = mat3(u_Transform) * mat3(a_Tangent, a_Binormal, a_Normal);
    v_Input.WorldTransform = mat3(u_Transform);
    v_Input.Binormal = a_Binormal;

    v_Input.CameraView = mat3(u_ViewMatrix);
    v_Input.CameraPosition = u_CameraPosition;
    v_Input.ViewPosition = vec3(u_ViewMatrix * vec4(v_Input.WorldPosition, 1.0));

    v_Color = vec3(1.0, 1.0, 1.0);
    v_EntityID = a_EntityID;

    gl_Position = u_ViewProjectionMatrix * u_Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

struct PointLight
{
    vec3 Position;
    float Multiplier;
    vec3 Radiance;
    float Radius;
    float MinRadius;
    float Falloff;
};

layout(std140, binding = 5) uniform PointLightsData
{
    uint u_PointLightsCount;
    PointLight u_PointLights[16];
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

layout(location = 0) in VertexOutput v_Input;
layout(location = 15) in vec3 v_Color;
layout(location = 16) flat in int v_EntityID;

layout(location = 0) out vec4 o_Color;
//layout(location = 1) out int o_EntityID;

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
    float ambientStrength = 0.3;
    float diffuseStrength = 1.0;
    float specularStrength = 0.5;
    
    vec3 result = vec3(0.33);
    for (int i = 0; i < u_PointLightsCount; i++)
    {
        PointLight light = u_PointLights[i];
        float distance = length(light.Position - v_Input.WorldPosition);
        vec3 attenuationValues = GetPointLightAttenuationValues(distance);
        float constant = attenuationValues.x;
        float linear = attenuationValues.y;
        float quadratic = attenuationValues.z;

        //float attenuation = clamp(1.0 / (constant + linear * distance + quadratic * distance * distance), 0.0, 1.0);
        float attenuation = clamp(1.0 / (1 + (2.0 / light.Radius) * distance + (1.0 / (light.Radius * light.Radius)) * (distance * distance)), 0.0, 1.0);
        vec3 radiance = light.Radiance * light.Multiplier;

        // Ambient
        vec3 ambient = ambientStrength * radiance;

        // Diffuse
        vec3 lightDir = normalize(light.Position - v_Input.WorldPosition);
        float diffuseImpact = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diffuseImpact * diffuseStrength * radiance;

        // Specular
        float shininess = 32;
        vec3 reflectDir = reflect(lightDir, normal);
        float specularImpact = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        vec3 specular = specularStrength * specularImpact * radiance;

        result += (ambient + diffuse + specular) * attenuation;
        //result += vec3(light.MinRadius);
    }

    return result;
}

void main()
{
    vec3 normal = normalize(v_Input.Normal);
    vec3 viewDir = normalize(v_Input.CameraPosition - v_Input.WorldPosition);
    vec4 pLightsColor = vec4(CalculatePointLights(normal, viewDir), 1.0);

    o_Color = vec4(v_Color, 1.0) * (pLightsColor);
    //o_EntityID = v_EntityID;
}