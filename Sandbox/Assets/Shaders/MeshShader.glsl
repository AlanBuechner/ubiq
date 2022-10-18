#shader vertex
#version 450 core

uniform mat4 u_Transform;
uniform mat4 u_View;
uniform mat4 u_Projection;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;
layout(location = 3) in vec3 a_Tangent;


out vec2 v_TexCoord;
out mat3 m_TBN;

void main() 
{
	v_TexCoord = a_TexCoord;
	v_TexCoord.y = 1-v_TexCoord.y;
	mat4 model_view = u_View * u_Transform;
	vec3 N = normalize(vec3(model_view * vec4(a_Normal, 1)));
	vec3 T = normalize(vec3(model_view * vec4(a_Tangent, 1)));
	T = normalize(T - dot(T, N) * N);
	vec3 B = normalize(cross(N, T));
	m_TBN = mat3(T, B, N);
	gl_Position = u_Projection * u_View * u_Transform * vec4(a_Position, 1.0);
}








#shader fragment
#version 450 core

layout(location = 0) out vec4 color;
layout(location = 1) out int id;

in vec2 v_TexCoord;
in mat3 m_TBN;

uniform sampler2D u_Diffuse;
uniform sampler2D u_Specular;
uniform sampler2D u_NormalMap;

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
	vec3 scolor = texture(u_Specular, v_TexCoord).rgb;
	vec3 normal = texture(u_NormalMap, v_TexCoord).rgb;
	// convert rgb (0 <-> 1) to xyx (-1 <-> 1)
	normal = normalize(normal * 2.0 - 1.0);
	// transform normals to model view space
	normal = normalize(m_TBN * normal);

	vec3 totalDiffuse = vec3(0.0, 0.0, 0.0);
	vec3 totalSpec = vec3(0.0, 0.0, 0.0);

	for(int i = 0; i < numLights; i++)
	{
		const vec3 vToL = pointLights[i].position - u_CameraPosition;
		const float distToL = length(vToL);
		const vec3 dirToL = vToL / distToL;
	
		const float att = 1.0 / (pointLights[i].attConst + (pointLights[i].attLin * distToL) + (pointLights[i].attQuad * distToL * distToL));
	
		const vec3 diffuse = pointLights[i].color * pointLights[i].diffuseIntensity * att * max(0.0, dot(dirToL, normal));

		const vec3 w = normal * dot(vToL, normal);
		const vec3 r = (w * 2.0) - vToL;

		const vec3 specular = att * (pointLights[i].color * pointLights[i].diffuseIntensity) * scolor * pow(max(0.0, dot(normalize(-r), normalize(normal))), 30.0);
		
		totalDiffuse += diffuse;
		totalSpec += specular;
	}

	color = vec4(clamp((totalDiffuse + ambientLight + totalSpec) * dcolor.rgb, 0.0, 1.0), dcolor.a);
	//color = vec4(totalDiffuse, 1);

	id = 0;
}
