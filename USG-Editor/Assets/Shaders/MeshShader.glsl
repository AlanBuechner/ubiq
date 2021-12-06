#shader vertex
#version 450 core

uniform mat4 u_Transform;
uniform mat4 u_View;
uniform mat4 u_Projection;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;


out vec2 v_TexCoord;
out vec3 v_CameraNormal;

void main() 
{
	v_TexCoord = a_TexCoord;
	v_TexCoord.y = 1-v_TexCoord.y;
	v_CameraNormal = normalize(vec3(u_View * u_Transform * vec4(a_Normal, 1.0)));
	gl_Position = u_Projection * u_View * u_Transform * vec4(a_Position, 1.0);
}








#shader fragment
#version 450 core

layout(location = 0) out vec4 color;
layout(location = 1) out int id;

in vec2 v_TexCoord;
in vec3 v_CameraNormal;

uniform sampler2D u_Diffuse;
uniform sampler2D u_Specular;

const int MAX_LIGHTS = 12;
struct PointLight
{
	vec3 position;
	vec3 color;
	float diffuseIntensity;
	float attConst;
	float attLin;
	float attQuad;
};
uniform int numLights = 0;
uniform PointLight pointLights[MAX_LIGHTS];
uniform vec3 ambientLight;

uniform vec3 u_CameraPosition;


void main() 
{
	vec4 dcolor = texture(u_Diffuse, v_TexCoord);
	vec3 scolor = vec3(texture(u_Specular, v_TexCoord));

	vec3 totalDiffuse = vec3(0.0, 0.0, 0.0);
	vec3 totalSpec = vec3(0.0, 0.0, 0.0);

	for(int i = 0; i < numLights; i++)
	{
		const vec3 vToL = pointLights[i].position - u_CameraPosition;
		const float distToL = length(vToL);
		const vec3 dirToL = vToL / distToL;
	
		const float att = 1.0 / (pointLights[i].attConst + (pointLights[i].attLin * distToL) + (pointLights[i].attQuad * distToL * distToL));
	
		const vec3 diffuse = pointLights[i].color * pointLights[i].diffuseIntensity * att * max(0.0, dot(dirToL, v_CameraNormal));

		const vec3 w = v_CameraNormal * dot(vToL, v_CameraNormal);
		const vec3 r = (w * 2.0) - vToL;

		const vec3 specular = att * (pointLights[i].color * pointLights[i].diffuseIntensity) * scolor * pow(max(0.0, dot(normalize(-r), normalize(v_CameraNormal))), 30.0);
		
		totalDiffuse += diffuse;
		totalSpec += specular;
	}

	//color = vec4(totalDiffuse * dcolor.rgb, dcolor.a);
	color = vec4(clamp((totalDiffuse + ambientLight + totalSpec) * dcolor.rgb, 0.0, 1.0), dcolor.a);

	id = 0;
}
