Texture2D skyDomeTexture : register(t0);
TextureCube skyDomeTextureCube : register(t1);

SamplerState SamplerLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;

};

cbuffer ColorBuffer
{
	float4 apexColor;
	float4 centerColor;
	float4 sunPosition;
	float4 cameraPosition;
	float sunSize;
    float sunPower;
    float scale1;
    float scale2;

};


struct PixelInputType
{
	float4 position : SV_POSITION;
	float4 domePosition : TEXCOORD0;
	float4 sunPosition : TEXCOORD1;
	float2 texCoord : TEXCOORD2;
	float4 normals : TEXCOORD3;
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
	float4 SkyTexture = skyDomeTextureCube.Sample(SamplerLinear, -input.normals.xyz);

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
    

    float SunSize = sunSize;
	
    float sunTheta = max(0.0, dot(normalize(input.domePosition), sunPosition1));


	// Determine the gradient color by interpolating between the apex and center based on the height of the pixel in the sky dome.
    outputColor = lerp(centerColor, apexColor, height);

    float3 dir = normalize(input.normals.xyz);

    float cosSunAngle = dot(dir, normalize(sunPosition.xyz));
    float4 sunColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float cosSunAngularRad = cos(sunPower);

    if (cosSunAngle >= cosSunAngularRad)
    {
        output.color = sunColor * 5;
    }
    else
    {
        output.color = SkyTexture + outputColor;
    }




	output.normal = float4(1.0f, 1.0f, 1.0f, 0.8f);
	output.specular = float4(0.0f, 0.0f, 0.0f, 1.0f);
	output.position = input.domePosition;
	output.roughness = float4(1.0f, 1.0f, 1.0f, 1.0f);

	return output;
}