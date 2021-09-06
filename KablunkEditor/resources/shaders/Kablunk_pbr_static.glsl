#type vertex
#version 450 core

layout(location = 1) in vec3 a_Position;
layout(location = 2) in vec3 a_Normal;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Binormal;
layout(location = 5) in vec2 a_TexCoord;

layout(std140, binding = 0) uniform Camera
{
    mat4 u_ViewProjectionMatrix;
    mat4 u_InverseViewProjectionMatrix;
    mat4 u_ProjectionMatrix;
    mat4 u_ViewMatrix;
};

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

uniform sampler2D u_Texture;

out VertexOutput v_Input;
out vec3 v_Albedo;

void main()
{
    v_Input.WorldPosition = vec3(u_Renderer.Transform * vec4(a_Position, 1.0));
    v_Input.Normal = mat3(u_Renderer.Transform) * a_Normal;
    v_Input.TexCoord = vec2(a_TexCoord.x, 1.0 - a_TexCoord.y);
    v_Input.WorldNormals = mat3(u_Renderer.Transform) * mat3(a_Tangent, a_Binormal, a_Normal);
    v_Input.WorldTransform = mat3(u_Renderer.Transform);
    v_Input.Binormal = a_Binormal;

    v_Input.CameraView = mat3(u_ViewMatrix);
    v_Input.ViewPosition = vec3(u_ViewMatrix * vec4(v_Input.WorldPosition, 1.0));

    //v_Albedo = vec3(texture(u_Texture, a_TexCoord));
    v_Albedo = vec3(1.0, 1.0, 1.0);

    gl_Position = u_ViewProjectionMatrix * u_Renderer.Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out vec4 o_ViewNormals;
layout(location = 2) out vec4 o_ViewPosition;

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

in VertexOutput v_Input;
in vec3 v_Albedo;

void main()
{
    vec3 normal = v_Input.CameraView * normalize(v_Input.Normal); 
    o_ViewNormals = vec4(normal, 1.0);

    o_ViewPosition = vec4(v_Input.ViewPosition, 1.0);

    float ambientStrength = 0.3;
	vec3 lightColor = vec3(1.0, 1.0, 1.0);
    vec4 ambient = vec4(ambientStrength * lightColor, 1.0);

    o_Color = vec4(v_Albedo, 1.0) * ambient;
}