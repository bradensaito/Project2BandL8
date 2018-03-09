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
	vec3 white = vec3(1.0, 1.0, 1.0);
	vec3 black = vec3(0, 0, 0);
	vec3 lightPos = light;

	vec3 realLight = lightPos;
	realLight = normalize(realLight);

	gl_Position = P * MV * vertPos;
	vec3 fragNor = (MV * vec4(vertNor, 0.0)).xyz;
	fragNor = normalize(fragNor);

	vec3 curPos = vec3(gl_Position.xyz);
	vec4 camera = vec4(0, 0, -6, 0);
	vec4 cPos = P * MV * camera;
	camera = cPos - gl_Position;
	vec3 three = vec3(camera.xyz);

	//vec3 normalized = normalize(vertNor);
/*
	if(abs(acos(dot(three, fragNor))) > 3.14 / 2)
	{
		inColor = black;
	}
	else
	{
		inColor = white;
	}

*/
/*
	if(acos(dot(camera, fragNor)) > 1)
	{
		inColor = black;
	}
	else
	{
		inColor = white;
	}*/

	/*
	if(fragNor.x > 0.8 || fragNor.y > 0.8 || fragNor.x < -0.8 || fragNor.y < -0.8)
	{
		inColor = black;
	}
	else
	{
		inColor = white;
	}
	*/

	if(fragNor.z > 0.4)
	{
		inColor = white;
	}
	else
	{
		inColor = black;
	}
	
}
