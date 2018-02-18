Texture2D skyDomeTexture : register(t0);
TextureCube skyDomeTextureCube : register(t1);
TextureCube skyDomeRadianceTexture : register(t8);

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
    float radiance;
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



//-------------------------------------------------------------------------------------------------
// Calculates the angle between two vectors
//-------------------------------------------------------------------------------------------------
float AngleBetween(in float3 dir0, in float3 dir1)
{
    return acos(dot(dir0, dir1));
}


float3 skyModelCIA(PixelInputType input)
{
 
    float3 dir = normalize(input.normals.xyz);
    float3 sunDir = normalize(sunPosition);

    float gamma = AngleBetween(dir, sunDir);
    float S = AngleBetween(sunDir, float3(0, 1, 0));
    float theta = AngleBetween(dir, float3(0, 1, 0));

    float cosTheta = cos(theta);
    float cosS = cos(S);
    float cosGamma = cos(gamma);

    float num = (0.91f + 10 * exp(-3 * gamma) + 0.45 * cosGamma * cosGamma) * (1 - exp(-0.32f / cosTheta));
    float denom = (0.91f + 10 * exp(-3 * S) + 0.45 * cosS * cosS) * (1 - exp(-0.32f));

    float lum = num / denom;

	// Clear Sky model only calculates luminance, so we'll pick a strong blue color for the sky
    const float3 SkyColor = float3(0.2f, 0.5f, 1.0f);
    const float3 SunColor = float3(1.0f, 0.8f, 0.3f) * 150;
    const float SunWidth = 0.05f;

    float3 color = SkyColor;
    color = lerp(SunColor, SkyColor, saturate(abs(gamma) / SunWidth));

    return max(color * lum, 0);
}

PixelOutputType SkyDomePixelShader(PixelInputType input) : SV_TARGET
{
    PixelOutputType output;
    float height, width;
    // Determine the position on the sky dome where this pixel is located.
    height = input.domePosition.y;

    width = input.domePosition.x;
	// The value ranges from -1.0f to +1.0f so change it to only positive values.

	
    float4 SkyTexture = skyDomeTextureCube.Sample(SamplerLinear, -input.normals.xyz);

    if (height < 0.0)
    {
        height = 0.0f;
    }
    if (width < 0.0)
    {
        width = 0.0f;
    }
    
	// Determine the gradient color by interpolating between the apex and center based on the height of the pixel in the sky dome.
    float3 skyColor = lerp(centerColor, apexColor, height);

    output.color = float4(skyModelCIA(input) + skyColor + SkyTexture, 1.0f);

    output.normal = float4(1.0f, 1.0f, 1.0f, 0.8f);
    output.specular = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.position = input.domePosition;
    output.roughness = float4(1.0f, 1.0f, 1.0f, 1.0f);
    return output;
}


//TODO:: Add sun for this shader for sunlike effect to the sky
/*
PixelOutputType SkyDomePixelShader(PixelInputType input) : SV_TARGET
{
	PixelOutputType output;
    float FP16Max = 65000.0f;
	float height,width;
	float4 outputColor;
	float sunDiameter = 1.0f;
	float pixDist;
	float4 sunPosition1 = sunPosition;
	float4 SkyTexture = skyDomeTextureCube.Sample(SamplerLinear, -input.normals.xyz);
    //float4 skyRadiance = skyDomeRadianceTexture.Sample(SamplerLinear, -normalize(input.normals.xyz));

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
	// Determine the gradient color by interpolating between the apex and center based on the height of the pixel in the sky dome.
    outputColor = lerp(centerColor, apexColor, height);

    float3 dir = normalize(input.normals.xyz);

    float cosSunAngle = dot(dir, normalize(sunPosition.xyz));
    float4 sunColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float cosSunAngularRad = cos(sunPower);



    if (cosSunAngle >= cosSunAngularRad)
    {
        output.color = sunColor * 5.0;
    }
    else
    {
        output.color = lerp(outputColor + SkyTexture, sunColor, cosSunAngle * scale1);
    }

    output.color = clamp(output.color, 0.0f, FP16Max);
 


	output.normal = float4(1.0f, 1.0f, 1.0f, 0.8f);
	output.specular = float4(0.0f, 0.0f, 0.0f, 1.0f);
	output.position = input.domePosition;
	output.roughness = float4(1.0f, 1.0f, 1.0f, 1.0f);

	return output;
}*/