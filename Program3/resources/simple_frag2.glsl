#version 330 core 
in vec3 inColor;
in vec3 fragNor;
in vec3 realLight; 
out vec4 color;

void main()
{

	vec3 normal = normalize(fragNor);
	vec3 light = normalize(realLight);
	// Map normal in the range [-1, 1] to color in range [0, 1];
/*	
	vec3 Ncolor = 0.5*normal + 0.5;
	color = vec4(Ncolor, 1.0);*/

	float diffuseIntensity;
	diffuseIntensity = 1.0 * clamp(dot(normal, light), 0.0, 1.0) * 1.0;

	float ambientIntensity = 0.3;

	vec3 colorthree = inColor * (diffuseIntensity + ambientIntensity);

	color = vec4(colorthree, 1.0);
	
}

