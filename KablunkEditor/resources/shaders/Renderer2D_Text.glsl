#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;

layout(std140, binding = 0) uniform Camera
{
    mat4 u_ViewProjection;
    mat4 u_Projection;
    mat4 u_View;
    vec3 u_Position;
};

layout (push_constant) uniform Transform
{
    mat4 Transform;
} u_Renderer;

struct VertexOutput
{
    vec4 Color;
    vec2 TexCoord;
};

layout(location = 0) out VertexOutput v_Output;
layout(location = 5) out flat float v_TexIndex;

void main()
{
    v_Output.Color = a_Color;
    v_Output.TexCoord = a_TexCoord;
    v_TexIndex = a_TexIndex;
    gl_Position = u_ViewProjection * u_Renderer.Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

struct VertexOutput
{
    vec4 Color;
    vec2 TexCoord;
};

layout(location = 0) out vec4 o_Color;

layout(location = 0) in VertexOutput v_Input;
layout(location = 5) in flat float v_TexIndex;

layout(binding = 1) uniform sampler2D u_FontAtlases[32];

float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

float screen_pixel_range()
{
    float pixel_range = 2.0f;
    vec2 unit_range = vec2(pixel_range) / vec2(textureSize(u_FontAtlases[int(v_TexIndex)], 0));
    vec2 screen_texture_size = vec2(1.0) / fwidth(v_Input.TexCoord);
    return max(0.5*dot(unit_range, screen_texture_size), 1.0);
}

void main()
{
    //vec4 bg_color = vec4(v_Input.Color.rgb, 0.0);
    //vec4 fg_color = v_Input.Color;

    //vec3 msd = texture(u_FontAtlases[int(v_TexIndex)], v_Input.TexCoord).rgb;
    //float sd = median(msd.r, msd.g, msd.b);
    //float screen_pixel_distance = screen_pixel_range() * (sd - 0.5f);
    //float opacity = clamp(screen_pixel_distance + 0.5, 0.0, 1.0);
    //o_Color = mix(bg_color, fg_color, opacity);

    o_Color = texture(u_FontAtlases[int(v_TexIndex)], v_Input.TexCoord) * v_Input.Color;
}