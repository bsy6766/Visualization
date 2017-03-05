in vec2 v_texCoord;   
in vec4 v_color;

uniform vec2 lightPosition;
uniform float lightIntensity;
uniform vec3 lightColor;

vec4 blur(vec2 p)
{
	float sampleNum = 2.0;
	float blurRadius = 5.0;
	float resolution = 650.0;
    if (blurRadius > 0.0 && sampleNum > 1.0)
    {
        vec4 col = vec4(0);
        vec2 unit = vec2(1.0 / resolution, 1.0 / resolution);
        
        float r = blurRadius;
        float sampleStep = r / sampleNum;
        
        float count = 0.0;
        
        for(float x = -r; x < r; x += sampleStep)
        {
            for(float y = -r; y < r; y += sampleStep)
            {
                float weight = (r - abs(x)) * (r - abs(y));
                col += texture2D(CC_Texture0, p + vec2(x * unit.x, y * unit.y)) * weight;
                count += weight;
            }
        }
        
        return col / count;
    }
    
    return texture2D(CC_Texture0, p);
}

void main()         
{
	// teture color must be white
	vec4 textureColor = texture2D(CC_Texture0, v_texCoord);
	//vec4 textureColor = blur(v_texCoord);

	if(textureColor.r == 0.0 && textureColor.g == 0.0 && textureColor.b == 0.0)
	{
		gl_FragColor = vec4(0, 0, 0, 0);
	}
	else
	{
		float dist = abs(distance(gl_FragCoord.xy, lightPosition));
		float ratio = 1.0 - (dist / lightIntensity);

		if(ratio > 1.0) ratio = 1.0;

		if(ratio <= 0.0)
		{
			gl_FragColor = vec4(0, 0, 0, 0);
		}
		else
		{
			// vec3 color = vec3(v_color.r, v_color.g, v_color.b);
			//vec4 color = textureColor * v_color;
		   	gl_FragColor = vec4(lightColor, ratio * 0.3);
		}
   }
}