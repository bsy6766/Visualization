in vec2 v_texCoord;   
in vec4 v_color;

uniform int lightSize;					// The actualy light used
uniform vec2 lightPositions[16];
uniform vec4 lightColors[16];
uniform float lightIntensities[16];
uniform sampler2D lightMaps[16];

void main()         
{
	int size = lightSize;
	if(size > 16)
	{
		size = 16;
	}

    vec4 finalColor = vec4(0.0, 0.0, 0.0, 0.0);

    int count = 0;

	for (int i = 0; i < size; i++)
	{
		// Read pixel on light map
		vec4 lightMapPixel = texture2D(lightMaps[i], vec2(v_texCoord.x, 1.0 - v_texCoord.y));
		vec4 lightColor = lightColors[i];

		if (lightMapPixel.r == 1.0 && lightMapPixel.g == 1.0 && lightMapPixel.b == 1.0)
		{
			// This pixel is visible from light
			// Get distance
			vec2 lightPos = lightPositions[i];
		    float dist = distance(gl_FragCoord.xy, lightPos);
		    float finalIntensity = dist / lightIntensities[i];
		    if (finalIntensity > 1.0)
		    {
		    	// Limit to 1.0
		    	finalIntensity = 1.0;
		    }
		    // Flip
		    finalIntensity = 1.0 - finalIntensity;
		    // Multiply light color
		   	lightColor *= finalIntensity;
		   	//lightColor.a = 0.2;

		   	//lightColor = lightColors[i];
		   	count++;
			finalColor += lightColor;
		}

	}

	if(size > 1)
	{
		finalColor /= (float)count;
	}

	//finalColor.a = 1.0;

    gl_FragColor = finalColor;
}