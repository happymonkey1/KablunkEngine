#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Binormal;
layout(location = 4) in vec2 a_TexCoord;

layout(std140, binding = 0) uniform Camera
{
    mat4 u_ViewProjectionMatrix;
    mat4 u_InverseViewProjectionMatrix;
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

out VertexOutput v_Input;
out vec3 v_Color;

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

    v_Color = vec3(1.0, 0.5, 0.31);

    gl_Position = u_ViewProjectionMatrix * u_Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

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

in VertexOutput v_Input;
in vec3 v_Color;

void main()
{
    // Ambient
    float ambientStrength = 0.3;
	vec3 lightColor = vec3(1.0, 1.0, 1.0);
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    float diffuseStrength = 1.0;
    vec3 lightPos = vec3(1.2, 1.0, 2.0);
    //vec3 lightPos = vec3(0.0, 1.0, 0.0);
    vec3 normal = normalize(v_Input.Normal);
    vec3 lightDir = normalize(lightPos - v_Input.WorldPosition);
    float diffuseImpact = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diffuseImpact * lightColor * diffuseStrength;


    // Specular
    float shininess = 32;
    float specularStrength = 0.5;
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 viewDir = normalize(v_Input.WorldPosition - v_Input.CameraPosition);
    float specularImpact = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * specularImpact * lightColor;

    o_Color = vec4(v_Color, 1.0) * vec4(ambient + diffuse + specular, 1.0);
}