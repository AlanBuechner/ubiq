#shader vertex
#version 450 core

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;


out vec2 v_TexCoord;
out vec4 v_Color;

void main() 
{
	v_TexCoord = a_TexCoord;
	v_TexCoord.y = 1-v_TexCoord.y;
	gl_Position = u_Transform * u_ViewProjection * vec4(a_Position, 1.0);
}








#shader fragment
#version 450 core

layout(location = 0) out vec4 color;
layout(location = 1) out int id;

in vec2 v_TexCoord;

uniform sampler2D u_Texture;

void main() 
{
	color = texture(u_Texture, v_TexCoord);

	id = 0;
}
