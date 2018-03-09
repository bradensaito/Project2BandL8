#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
uniform mat4 P;
uniform mat4 MV;
uniform vec3 light;
out vec3 fragNor;
out vec3 realLight;
out vec3 inColor;

void main()
{
	inColor = vec3(1.0, 0.0, 0.0);
	vec3 lightPos = light;

	vec3 vertPosMine = vertPos.xyz;

	realLight = lightPos;

	gl_Position = P * MV * vertPos;
	fragNor = (MV * vec4(vertNor, 0.0)).xyz;
	fragNor = normalize(fragNor);

}