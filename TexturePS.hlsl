
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

// Performs a gaussian blur in one direction
float4 Blur(PixelInputType input, Texture2D shaderTexture, float2 texScale, float sigma, bool nrmlize)
{
	float2 inputSize = 0.0f;
	shaderTexture.GetDimensions(inputSize.x, inputSize.y);

	float4 color = 0;
	float weightSum = 0.0f;
	for (int i = -7; i < 7; i++)
	{
		float weight = CalcGaussianWeight(i, sigma);
		weightSum += weight;
		float2 texCoord = input.tex;
		texCoord += (i / inputSize) * texScale;
		float4 sample = shaderTexture.Sample(SampleType, texCoord);
		color += sample * weight;
	}

	if (nrmlize)
		color /= weightSum;

	return color;
}

float2 texOffset(int u, int v)
{
	return float2(u * 1.0f / screenWH.x, v * 1.0f / screenWH.y);
}
#include "FXAA_TEST.hlsl"
float4 BlurTest(PixelInputType input)
{
	float4 BlurredColor = float4(0.0, 0.0, 0.0, 0.0);
	// Initialize the color to black.

	float x, y;
	float LOS = 1.0f;


	[unroll]
			for (y = -LOS; y <= LOS; y += 1.0f)
			{
				for (x = -LOS; x <= LOS; x += 1.0f)
				{

					FxaaTex fColors;
					fColors.smpl = SampleType;
					fColors.tex = specHighTex;
					BlurredColor += float4(FxaaPixelShader(input.tex + texOffset(x, y), fColors, float2(1.0 / screenWH.x, 1.0 / screenWH.y)), 1.0);
				}
			}
		
	


	return BlurredColor / 4.0;
}

// Applies the approximated version of HP Duiker's film stock curve
float3 ToneMapFilmicALU(in float3 color)
{
	color = max(0, color - 0.004f);
	color = (color * (6.2f * color + 0.5f)) / (color * (6.2f * color + 1.7f) + 0.06f);
	return color;
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
	float4 reds   = shaderTexture.GatherRed(  SampleType, input.tex);
	float4 greens = shaderTexture.GatherGreen(SampleType, input.tex);
	float4 blues  = shaderTexture.GatherBlue( SampleType, input.tex);

	float3 result = 0.0f;

	[unroll]
	for (uint i = 0; i < 4; ++i)
	{
		float3 color = float3(reds[i], greens[i], blues[i]);

		result += color;
	}

	result /= 16.0f;

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


	FxaaTex fColors;
	fColors.smpl = SampleType;
	fColors.tex = shaderTexture;
	textureColor = float4(FxaaPixelShader(TexPos, fColors, float2(1.0 / screenWH.x, 1.0 / screenWH.y)), 1.0);

	float4 specBloomTex = specHighTex.Sample(SampleType, input.tex);


	textureColor += Bloom(input) * Blur(input, shaderTexture, float2(0, 1), 0.5, false) * Blur(input, shaderTexture, float2(1, 0), 0.5, false);

	textureColor *= ToneMap(textureColor);

	return textureColor;
}