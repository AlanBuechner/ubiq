#shader vertex
#version 330 core

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

uniform int u_AtlasRows = 1;
uniform vec2 u_AtlasPos;

out vec2 v_TexCoord;

void main() {
	v_TexCoord = (a_TexCoord/u_AtlasRows) + u_AtlasPos;
	gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
}








#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;

uniform sampler2D u_Texture;

void main() {
	color = texture(u_Texture, v_TexCoord);
}
