#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in float a_TilingFactor;
layout(location = 5) in int a_EntityID; // TODO remove when ray cast mouse picking added to editor

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

layout(location = 0) out vec4 v_Color;
layout(location = 1) out vec2 v_TexCoord;
layout(location = 2) out flat float v_TexIndex;
layout(location = 3) out float v_TilingFactor;
layout(location = 4) flat out int v_EntityID; // TODO remove when ray cast mouse picking added to editor

void main()
{
	v_Color = a_Color;
	v_TexCoord = a_TexCoord;
	v_TexIndex = a_TexIndex;
	v_TilingFactor = a_TilingFactor;
	v_EntityID = a_EntityID; // TODO remove when ray cast mouse picking added to editor
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID; // TODO remove when ray cast mouse picking added to editor

layout(location = 0) in vec4 v_Color;
layout(location = 1) in vec2 v_TexCoord;
layout(location = 2) in flat float v_TexIndex;
layout(location = 3) in float v_TilingFactor;
layout(location = 4) flat in int v_EntityID; // TODO remove when ray cast mouse picking added to editor

layout(binding = 1) uniform sampler2D u_Textures[32];

void main()
{
	o_Color  = texture(u_Textures[int(v_TexIndex)], v_TexCoord * v_TilingFactor) * v_Color;
	o_EntityID = v_EntityID; // TODO remove when ray cast mouse picking added to editor
}
