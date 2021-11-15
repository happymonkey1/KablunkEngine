#type vertex
#version 450 core

layout(location = 0) in vec3 a_WorldPosition;
layout(location = 1) in vec3 a_LocalPosition;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in float a_Radius;
layout(location = 4) in float a_Thickness;
layout(location = 5) in float a_Fade;
layout(location = 6) in int a_EntityID; // TODO remove when ray cast mouse picking added to editor

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

struct VertexOutput
{
	vec3 LocalPosition;
	float Radius;
	vec4 Color;
	float Thickness;
	float Fade;
};

layout (location = 0) out VertexOutput v_Output;
layout (location = 5) flat out int v_EntityID; // TODO remove when ray cast mouse picking added to editor

void main()
{
	v_Output.LocalPosition = a_LocalPosition;
	v_Output.Color = a_Color;
	v_Output.Radius = a_Radius;
	v_Output.Thickness = a_Thickness;
	v_Output.Fade = a_Fade;
	v_EntityID = a_EntityID; // TODO remove when ray cast mouse picking added to editor
	
	gl_Position = u_ViewProjection * vec4(a_WorldPosition, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID; // TODO remove when ray cast mouse picking added to editor

struct VertexOutput
{
	vec3 LocalPosition;
	float Radius;
	vec4 Color;
	float Thickness;
	float Fade;
};

layout(location = 0) in VertexOutput v_Output;
layout(location = 5) flat in int v_EntityID; // TODO remove when ray cast mouse picking added to editor

void main()
{
	float distance = 1.0 - length(v_Output.LocalPosition);
	float alpha = smoothstep(0.0, v_Output.Fade, distance);
	alpha *= smoothstep(v_Output.Thickness + v_Output.Fade, v_Output.Thickness, distance);

	if (alpha == 0)
		discard;

	o_Color = v_Output.Color;
	o_Color.a *= alpha;

	o_EntityID = v_EntityID; // TODO remove when ray cast mouse picking added to editor
}
