in vec2 v_texCoord;   
in vec4 v_color;

const int MAX_LIGHT_COUNT = 16;				// 16 lights + cursor light
const float GAP1 = 5.0;
const float GAP2 = 15.0;

uniform int lightSize;					// The actualy light used
// uniform vec2 lightPositions[MAX_LIGHT_COUNT];
// uniform vec3 lightColors[MAX_LIGHT_COUNT];
// uniform float lightIntensities[MAX_LIGHT_COUNT];
uniform float lightSources[MAX_LIGHT_COUNT * 6];
uniform sampler2D lightMap;

// vec4 blur(vec2 p)
// {
// 	float sampleNum = 2.0;
// 	float blurRadius = 5.0;
// 	float resolution = 650.0;
//     if (blurRadius > 0.0 && sampleNum > 1.0)
//     {
//         vec4 col = vec4(0);
//         vec2 unit = vec2(1.0 / resolution, 1.0 / resolution);
        
//         float r = blurRadius;
//         float sampleStep = r / sampleNum;
        
//         float count = 0.0;
        
//         for(float x = -r; x < r; x += sampleStep)
//         {
//             for(float y = -r; y < r; y += sampleStep)
//             {
//                 float weight = (r - abs(x)) * (r - abs(y));
//                 col += texture2D(lightMap, p + vec2(x * unit.x, y * unit.y)) * weight;
//                 count += weight;
//             }
//         }
        
//         return col / count;
//     }
    
//     return texture2D(lightMap, p);
// }

vec2 getLightPosition(const int index)
{
	return vec2(lightSources[index * 6], lightSources[index * 6 + 1]);
}

vec3 getLightColor(const int index)
{
	return vec3(lightSources[index * 6 + 2], lightSources[index * 6 + 3], lightSources[index * 6 + 4]);
}

float getLightIntensity(const int index)
{
	return lightSources[index * 6 + 5];
}

void main()         
{
	int size = lightSize;
	if(size > MAX_LIGHT_COUNT)
	{
		size = MAX_LIGHT_COUNT;
	}
	
	vec4 lightMapColor = texture2D(lightMap, v_texCoord);
	//vec4 lightMapColor = blur(v_texCoord);

	if(size > 0)
	{
		vec3 colorSum = vec3(0.0, 0.0, 0.0);
		vec4 finalColor = vec4(0, 0, 0, 0);

		float sizeF = float(size);
		float mixRatio = 1.0 / sizeF;

		bool whiteRange = false;
		bool mixRange = false;
		float mixRangeRatio = 0.0;

		float finalAlpha = lightMapColor.r * 1.3;

		for (int i = 0; i < size; i++)
		{
			//vec2 lightPos = lightPositions[i];
			vec2 lightPos = getLightPosition(i);

			if(lightPos.x == 0.0 && lightPos.y == 0.0)
			{
				continue;
			}
			// vec3 lightColor = lightColors[i];
			vec3 lightColor = getLightColor(i);

			// float lightIntensity = lightIntensities[i];
			float lightIntensity = getLightIntensity(i);

		    float dist = abs(distance(gl_FragCoord.xy, lightPos));

		    if(0.0 <= dist && dist <= GAP1)
		    {
		    	lightColor = vec3(1, 1, 1);
		    	finalColor = vec4(lightColor, 1.0);
		    	whiteRange = true;
		    	break;
		    }
		    else if(GAP1 < dist && dist <= GAP2)
		    {
		    	float ratio = 1.0 - ((dist - GAP1) / (GAP2 - GAP1));
		    	colorSum += (lightColor * mixRatio);
		    	mixRange = true;
		    	mixRangeRatio = ratio;
		    }
		    else if(GAP2 < dist && dist <= lightIntensity)
		    {
		    	float ratio = ((dist - GAP2) / (lightIntensity - GAP2));
		    	lightColor = mix(lightColor, vec3(0, 0, 0), ratio);
		    	colorSum += (lightColor * mixRatio);
		    }
		    else
		    {
		    	continue;
		    }
		}

		if(!whiteRange)
		{
			if(mixRange)
			{
			    colorSum = mix(colorSum, vec3(1, 1, 1), mixRangeRatio);
			 	finalColor = vec4(colorSum * 1.5, mix(finalAlpha, 1.0, mixRangeRatio));
			}
			else
			{
				finalColor = vec4(colorSum * 1.5, finalAlpha);
			}
		}

		gl_FragColor = finalColor;
	}
	else
	{
   		gl_FragColor = vec4(0, 0, 0, 1);
	}
}