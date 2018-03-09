#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
uniform mat4 P;
uniform mat4 MV;
uniform vec3 light;
//out vec3 fragNor;

out vec3 inColor;

void main()
{
	vec3 tmpcolor = vec3(1.0, 0.0, 0.0);
	vec3 lightPos = light;

	vec3 realLight = lightPos;
	realLight = normalize(realLight);

	gl_Position = P * MV * vertPos;
	vec3 fragNor = (MV * vec4(vertNor, 0.0)).xyz;
	fragNor = normalize(fragNor);

	float diffuseIntensity;
	diffuseIntensity = 1.0 * clamp(dot(fragNor, realLight), 0.0, 1.0) * 1.0;

	float ambientIntensity = 0.3;



	inColor = tmpcolor * (diffuseIntensity + ambientIntensity);

}
