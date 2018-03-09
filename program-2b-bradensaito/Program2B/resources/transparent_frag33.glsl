#version 330 core 
in vec3 fragNor;
out vec4 color;

void main()
{
	vec3 normal = normalize(fragNor);
	// Map normal in the range [-1, 1] to color in range [0, 1];
	if(0.5 * normal.z < 0)
	{
	vec3 Ncolor = 0.5*normal + 0.5;
	color = vec4(1 + normal.z, 1 + normal.z, 1 + normal.z, 1.0);
	}
	else
	{
	color = vec4(1.0, 1.0, 1.0, 0.3);
	//color.a = 0.3;
	}
}