#shader fragment
#version 330 core
layout(location = 0) out vec4 color;

in vec4 v_Color;

void main() {
	color = vec4(0.8, 0.2, 0.3, 1.0);
	color = v_Color;
}

#shader vertex
#version 330 core

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;

out vec4 v_Color;

void main() {
	v_Color = a_Color;
	gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
}