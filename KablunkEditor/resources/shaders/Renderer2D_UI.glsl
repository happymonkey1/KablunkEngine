#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in float a_TilingFactor;


layout(location = 0) out vec4 v_Color;
layout(location = 1) out vec2 v_TexCoord;
layout(location = 2) out flat float v_TexIndex;
layout(location = 3) out float v_TilingFactor;

layout (push_constant) uniform Transform
{
	mat4 Transform;
} u_Renderer;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
	mat4 u_Projection;
	mat4 u_View;
	vec3 u_Position;
};

void main()
{
	v_Color = a_Color;
	v_TexCoord = a_TexCoord;
	v_TexIndex = a_TexIndex;
	v_TilingFactor = a_TilingFactor;
	vec4 p = vec4(mat3(u_Projection) * a_Position, 1.0);
	p.z = 0.0f;
	gl_Position = p; 
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec4 v_Color;
layout(location = 1) in vec2 v_TexCoord;
layout(location = 2) in flat float v_TexIndex;
layout(location = 3) in float v_TilingFactor;

layout(binding = 1) uniform sampler2D u_Textures[32];

void main()
{
	vec4 color = texture(u_Textures[int(v_TexIndex)], v_TexCoord * v_TilingFactor) * v_Color;
	// remove when transparency sorting implemented
	if (color.a < 0.5)
		discard;
	o_Color = color;
}
