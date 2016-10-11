

cbuffer ColorBuffer
{
	float4 apexColor;
	float4 centerColor;
	float4 sunPosition;
};


struct PixelInputType
{
	float4 position : SV_POSITION;
	float4 domePosition : TEXCOORD0;
	float4 sunPosition : TEXCOORD1;
};


//TODO:: Add sun for this shader for sunlike effect to the sky
float4 SkyDomePixelShader(PixelInputType input) : SV_TARGET
{
	float height,width;
	float4 outputColor;
	float sunDiameter = 1.0f;
	float pixDist;
	float4 sunPosition1 = sunPosition;

	sunPosition1 = normalize(sunPosition1);
	// Determine the position on the sky dome where this pixel is located.
	height = input.domePosition.y;

	width  = input.domePosition.x;
	// The value ranges from -1.0f to +1.0f so change it to only positive values.

	//Draw sun 
	/*
	float3 dir = normalize(input.domePosition);
	float cosSunAngle = dot(dir, normalize(input.sunPosition));
	if (cosSunAngle >= 0.892f)
	{
		float lum = (1 + 2 * cosSunAngle) / 3;
		float4 sunColor = float4(lum,lum,lum, 1.0f);
		return sunColor;
	}
	*/
	if (height < 0.0)
	{
		height = 0.0f;
	}
	if (width < 0.0)
	{
		width = 0.0f;
	}

	// Determine the gradient color by interpolating between the apex and center based on the height of the pixel in the sky dome.
	outputColor = lerp(centerColor, apexColor, height);

	

	return outputColor;
}