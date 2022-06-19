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
    v_Output.Normal = mat3(u_Transform) * a_Normal;
    v_Output.TexCoord = vec2(a_TexCoord.x, 1.0 - a_TexCoord.y);
    v_Output.WorldNormals = mat3(u_Transform) * mat3(a_Tangent, a_Binormal, a_Normal);
    v_Output.WorldTransform = mat3(u_Transform);
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
    vec3 Radiance;
    float Radius;
    float MinRadius;
    float Falloff;

    vec2 Padding;
};

layout(std140, binding = 2) uniform PointLightsData
{
    uint Count;
    PointLight Lights[1024];
} u_PointLights;

layout(push_constant) uniform Material
{
	float AmbientStrength;
    float DiffuseStrength;
    float SpecularStrength;
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
    vec3 result = vec3(0.33);
    for (int i = 0; i < u_PointLights.Count; i++)
    {
        PointLight light = u_PointLights.Lights[i];
        float distance = length(light.Position - v_Input.WorldPosition);
        vec3 attenuationValues = GetPointLightAttenuationValues(distance);
        float constant = attenuationValues.x;
        float linear = attenuationValues.y;
        float quadratic = attenuationValues.z;

        //float attenuation = clamp(1.0 / (constant + linear * distance + quadratic * distance * distance), 0.0, 1.0);
        float attenuation = clamp(1.0 / (1 + (2.0 / light.Radius) * distance + (1.0 / (light.Radius * light.Radius)) * (distance * distance)), 0.0, 1.0);
        vec3 radiance = light.Radiance * light.Multiplier;

        // Ambient
        vec3 ambient = u_MaterialUniforms.AmbientStrength * radiance;

        // Diffuse
        vec3 lightDir = normalize(light.Position - v_Input.WorldPosition);
        float diffuseImpact = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diffuseImpact * u_MaterialUniforms.DiffuseStrength * radiance;

        // Specular
        float shininess = 32;
        // Blinn-Phong 
        //vec3 reflectDir = reflect(-lightDir, normal);
        vec3 halfDir = normalize(lightDir + viewDir);
        float specularImpact = pow(max(dot(normal, halfDir), 0.0), shininess);
        vec3 specular = u_MaterialUniforms.SpecularStrength * specularImpact * radiance;

        result += (ambient + diffuse + specular) * attenuation;
        //result += vec3(light.MinRadius);
    }

    return result;
}

int MAX_BOUNCES = 3
float PI = 3.141592

vec3 getRandomDirectionHemisphere(in vec3 normal)
{
    float rotMat[2][2] = { {normal.y, -normal.x}, {normal.x, normal.y} };
    float theta = rand() * PI;
    float cosTheta = cos(theta);
    float sinTheta = sin(theta);

}

vec3 rayCast(in vec3 origin, in vec3 dir, in int bounces)
{
    if (bounces >= MAX_BOUNCES)
        return vec3(0.0)
    
    vec3 indirectDiffuse = vec3(0.0);

    int samples = 16;
    for (int i = 0; i < samples; ++i)
    {
        vec3 sampleVec = ...;
        vec3 sampleLocalVec = ...;

        indirectDiffuse += dir.dot(sampleLocalVec) * rayCast(origin, sampleLocalVec, bounces + 1);
    }

    vec3 contribution = (indirectDiffuse / samples) / PI;
    return contribution;
}

void main()
{
    vec3 color = vec3(1.0);
    vec3 normal = normalize(v_Input.Normal);
    vec3 viewDir = normalize(v_Input.CameraPosition - v_Input.WorldPosition);
    vec4 directLightContribution = vec4(CalculatePointLights(normal, viewDir), 1.0);


    vec3 indirectLightApprox = rayCast(v_Input.WorldPosition, , 0)

    vec4 indirectLightContribution = vec4(indirectLightApprox, 1.0);
    o_Color = vec4(color, 1.0) * (directLightContribution + indirectLightContribution);
}