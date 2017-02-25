in vec2 v_texCoord;   
in vec4 v_color;

#define MAX_LIGHT_COUNT 17;				// 16 lights + cursor light

uniform int lightSize;					// The actualy light used
uniform vec2 lightPositions[MAX_LIGHT_COUNT];
uniform vec3 lightColors[MAX_LIGHT_COUNT];
uniform float lightIntensities[MAX_LIGHT_COUNT];
uniform sampler2D lightMap;

void main()         
{
	int size = lightSize;
	if(size > MAX_LIGHT_COUNT)
	{
		size = MAX_LIGHT_COUNT;
	}
	
	vec4 lightMapColor = texture2D(lightMap, v_texCoord);

	int count = 0;

	vec3 colorSum = vec3(0.0, 0.0, 0.0);

	for (int i = 0; i < MAX_LIGHT_COUNT; i++)
	{
		vec3 lightColor = lightColors[i];
		vec2 lightPos = lightPositions[i];
		float lightIntensity = lightIntensities[i];

	    float dist = distance(gl_FragCoord.xy, lightPos);
	    float finalIntensity = dist / lightIntensity;

	    if (finalIntensity > 1.0) finalIntensity = 1.0;
	    else if(finalIntensity < 0.0) finalIntensity = 0.0;
	    finalIntensity = 1.0 - finalIntensity;
	    lightColor *= finalIntensity;

		colorSum += lightColor;

	   	count++;
	}

	float c = float(size);
	colorSum.r /= c;
	colorSum.g /= c;
	colorSum.b /= c;

    gl_FragColor = vec4(colorSum, lightMapColor.a);
}