
Texture2D shaderTexture : register(t0);
Texture2D specHighTex   : register(t1);
SamplerState SampleType;

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

cbuffer PostProcessData
{
	float2 screenWH;
	float expa;
};

// Calculates the gaussian blur weight for a given distance and sigmas
float CalcGaussianWeight(int sampleDist, float sigma)
{
	float g = 1.0f / sqrt(2.0f * 3.14159 * sigma * sigma);
	return (g * exp(-(sampleDist * sampleDist) / (2 * sigma * sigma)));
}
#include "FXAA_TEST.hlsl"
// Performs a gaussian blur in one direction
float4 Blur(PixelInputType input, Texture2D shaderTexture, float2 texScale, float sigma, bool nrmlize)
{
	float2 inputSize = 0.0f;
	shaderTexture.GetDimensions(inputSize.x, inputSize.y);

	float4 color = 0;
	float weightSum = 0.0f;
	nrmlize = true;
	for (int i = -11; i < 11; i++)
	{

		float weight = CalcGaussianWeight(i, sigma);
		weightSum += weight;
		float2 texCoord = input.tex;
		texCoord += (i / inputSize) * texScale;
		float4 samples = shaderTexture.Sample(SampleType, texCoord);
		color += samples * weight;
	}

	if (nrmlize)
		color /= weightSum;

	return color;
}

float2 texOffset(int u, int v)
{
	return float2(u * 1.0f / screenWH.x, v * 1.0f / screenWH.y);
}




//Jim Heijl & Richard Burgess-Dawson
float4 ToneMap(float4 TexColor)
{
	float3 TexCol = TexColor.rgb;
	TexCol *= 16;

	float3 x = max(0, TexColor - 0.004);
	
	float3 retColor = (x*(6.2*x + .5)) / (x* (6.2*x + 1.7) + 0.06);

	return float4(retColor, 1.0f);
}

// Initial pass for bloom
float4 Bloom(PixelInputType input)
{
	float4 reds   = specHighTex.GatherRed(  SampleType, input.tex);
	float4 greens = specHighTex.GatherGreen(SampleType, input.tex);
	float4 blues  = specHighTex.GatherBlue( SampleType, input.tex);

	float3 result = 0.0f;

	[unroll]
	for (uint i = 0; i < 4; ++i)
	{
		float3 color = float3(reds[i], greens[i], blues[i]);

		result += color;
	}

	result /= 4.0f;

	return float4(result, 1.0f);
}

/*
// Horizontal gaussian blur
float4 BlurH(in PSInput input) : SV_Target
{
	return Blur(input, float2(1, 0), AppSettings.BloomBlurSigma, false);
}

// Vertical gaussian blur
float4 BlurV(in PSInput input) : SV_Target
{
	return Blur(input, float2(0, 1), AppSettings.BloomBlurSigma, false);
}

*/



float4 TexturePixelShader(PixelInputType input) : SV_TARGET
{
	float4 textureColor = 0.0f;
	float4 textureBlurred;
	
	float2 TexPos = input.tex;

	// Sample the pixel color from the texture using the sampler at this texture coordinate location.

	float sigma = expa;

	FxaaTex fColors;
	fColors.smpl = SampleType;
	fColors.tex = shaderTexture;
	textureColor = float4(FxaaPixelShader(TexPos, fColors, float2(1.0 / screenWH.x, 1.0 / screenWH.y)), 1.0);

	float4 specBloomTex = specHighTex.Sample(SampleType, input.tex);

	
	
	if (sigma > 0.1)
	{
		textureColor += saturate(Bloom(input) + (Blur(input, specHighTex, float2(0, 1), sigma, false) + Blur(input, specHighTex, float2(1, 0), sigma, false)));


		textureColor *= ToneMap(textureColor);
		textureColor = pow(textureColor, 1.0f / 1.4);
	}
	
	

	return textureColor;
}