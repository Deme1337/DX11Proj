
Texture2D shaderTexture : register(t0);
Texture2D specHighTex   : register(t1);

Texture2D areaTexture   : register(t2);
Texture2D positionTex   : register(t3);
Texture2D normalTex     : register(t4);

SamplerState SampleType;

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	matrix projection : TEXCOORD1;
};

cbuffer PostProcessData : register(b0)
{
	float2 screenWH;
	float expa;
	float4 ssaoKernel[64];
};

// Calculates the gaussian blur weight for a given distance and sigmas
float CalcGaussianWeight(int sampleDist, float sigma)
{
	float g = 1.0f / sqrt(2.0f * 3.14159 * sigma * sigma);
	return (g * exp(-(sampleDist * sampleDist) / (2 * sigma * sigma)));
}



#include "FXAA_TEST.hlsl"
#define SMAA_RT_METRICS float4(1.0 / 1920.0, 1.0 / 1080.0, 1920.0, 1080.0)
#define SMAA_HLSL_4
#define SMAA_PRESET_ULTRA
#include "SMAA.hlsl"

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


// Approximates luminance from an RGB value
float CalcLuminance(float3 color)
{
	return max(dot(color, float3(0.299f, 0.587f, 0.114f)), 0.0001f);
}

// Retrieves the log-average luminance from the texture
float GetAvgLuminance(Texture2D lumTex)
{
	return lumTex.Load(uint3(0, 0, 0)).x;
}

// Creates the luminance map for the scene
float4 LuminanceMap(float2 texcoord)
{
	// Sample the input
	float3 color = shaderTexture.Sample(SampleType, texcoord).rgb;

	// calculate the luminance using a weighted average
	float luminance = log(max(CalcLuminance(color), 0.00001f));

	return float4(luminance, 1.0f, 1.0f, 1.0f);
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


// Determines the color based on exposure settings
float3 CalcExposedColor(float3 color, float avgLuminance, float threshold, out float exposure)
{
	// Use geometric mean
	avgLuminance = max(avgLuminance, 0.001f);
	float keyValue = 2;
	float linearExposure = (keyValue / avgLuminance);
	exposure = log2(max(linearExposure, 0.0001f));
	exposure -= threshold;
	return exposure*exposure * color;
}

// Uses a lower exposure to produce a value suitable for a bloom pass
float4 Threshold(float2 texcoord)
{
	float3 color = 0;

	color = specHighTex.Sample(SampleType, texcoord).rgb;

	// Tone map it to threshold
	float avgLuminance = GetAvgLuminance(specHighTex);
	float exposure = 2;
	float pixelLuminance = CalcLuminance(color);
	color = CalcExposedColor(color, avgLuminance, 2, exposure);

	if (dot(color, 0.333f) <= 0.001f)
		color = 0.0f;

	return float4(color, 1.0f);
}

// Initial pass for bloom
float4 Bloom(PixelInputType input)
{
	
	float4 reds   = specHighTex.GatherRed(  SampleType, input.tex,int2(1,1));
	float4 greens = specHighTex.GatherGreen(SampleType, input.tex, int2(1, 1));
	float4 blues = specHighTex.GatherBlue(SampleType, input.tex, int2(1, 1));

	float3 result = 0.0f;

	
	for (uint i = 0; i < 4; ++i)
	{
		float3 color = float3(reds[i], greens[i], blues[i]);

		result += color;
	}

	result /= 4.0f;

	return float4(result, 1.0f);
}




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

		textureColor *= ToneMap(textureColor);
		textureColor += (Bloom(input) + (Blur(input, specHighTex, float2(0, 1), sigma, false) + Blur(input, specHighTex, float2(1, 0), sigma, false)));
	}
	
	

	return textureColor;
}


float4 BlurVertical(PixelInputType input) : SV_TARGET
{
	float sigma = expa;
	return Blur(input, specHighTex, float2(0, 1), sigma, false);
}

float4 BlurHorizontal(PixelInputType input) : SV_TARGET
{
	float sigma = expa;
	return Blur(input, specHighTex, float2(1, 0), sigma, false);
}


float4 BloomColors(PixelInputType input) : SV_TARGET
{
	
	return Bloom(input);
}

float4 ReturnTexture(PixelInputType input) : SV_TARGET
{
	return shaderTexture.Sample(SampleType, input.tex);
}

float4 Combine(PixelInputType input) : SV_TARGET
{
	float2 TexPos = input.tex;
	float4 textureColor = 0.0f;
	float4 textureColor1 = 0.0f;
	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
	FxaaTex fColors;
	fColors.smpl = SampleType;
	fColors.tex = shaderTexture;
	textureColor = float4(FxaaPixelShader(TexPos, fColors, float2(1.0 / screenWH.x, 1.0 / screenWH.y)), 1.0);

	//
	fColors.tex = specHighTex;
	textureColor1 = float4(FxaaPixelShader(TexPos, fColors, float2(1.0 / screenWH.x, 1.0 / screenWH.y)), 1.0);


	return textureColor + ToneMap(textureColor1);
}


//http://www.learnopengl.com/#!Advanced-Lighting/SSAO changed to hlsl
float4 SSAO(PixelInputType input) : SV_TARGET
{
	float3 position = positionTex.Sample(SampleType, input.tex).xyz;
	float3 normal = normalTex.Sample(SampleType, input.tex).xyz;
	float3 randomF = areaTexture.Sample(SampleType, input.tex).xyz;

	float3 tangent = normalize(randomF - normal * dot(randomF, normal));
	float3 bitangent = cross(normal, tangent);

	float occlusion = 0.0f;

	float radius = 100.0f;
	
	float2 noiseScale = float2(screenWH.x / 4.0f, screenWH.y / 4.0f);

	for (int i = 0; i < 64; i++)
	{
		float3 sample = tangent * bitangent * normal *  ssaoKernel[i].xyz;
		sample = position + sample * radius;

		float4 offset = float4(sample, 1.0f);

		offset = mul(input.projection, offset);
		offset.xyz /= offset.w; // perspective divide
		offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
		
		float sampleDepth = -positionTex.Sample(SampleType, offset.xy).w;


		float rangeCheck = smoothstep(0.0f, 1.0f, radius / abs(position.z  - sampleDepth + 0.000005));

		occlusion += (sampleDepth >= sample.z ? 1.0f : 0.0f) * rangeCheck;
	}

	occlusion = 1.0f - (occlusion / 64);

	return occlusion;
}					  
