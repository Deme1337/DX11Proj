

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

struct PixelOutputType
{
	float4 color : SV_Target0;
	float4 normal : SV_Target1;
	float4 specular : SV_Target2;
	float4 position : SV_Target3;
	float4 roughness : SV_Target4;
};

//TODO:: Add sun for this shader for sunlike effect to the sky
PixelOutputType SkyDomePixelShader(PixelInputType input) : SV_TARGET
{
	PixelOutputType output;

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

		//Draw sun 
	/*
	float3 dir = normalize(input.domePosition);
	float cosSunAngle = dot(dir, normalize(input.sunPosition));
	if (cosSunAngle >= 0.892f)
	{
		float lum = (1 + 2 * cosSunAngle) / 3;
		float4 sunColor = float4(lum,lum,lum, 1.0f);
		outputColor 
	}
	*/
	output.color = outputColor*1.5;
	output.normal = float4(1.0f, 1.0f, 1.0f, 0.5f);
	output.specular = float4(0.0f, 0.0f, 0.0f, 1.0f);
	output.position = input.domePosition;
	output.roughness = float4(1.0f, 1.0f, 1.0f, 1.0f);

	return output;
}