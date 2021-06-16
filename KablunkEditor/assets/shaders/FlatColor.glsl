#type vertex
#version 450 core
			
layout(location = 0) in vec3 a_Position;
			
uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec3 v_Position;

void main()
{
	gl_Position = u_ViewProjection * u_Transform  * vec4(a_Position, 1.0);
	v_Position = a_Position;
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
in vec3 v_Position;

uniform vec4 u_Color;

void main()
{
	o_Color = u_Color;
}