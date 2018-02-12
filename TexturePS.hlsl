
Texture2D shaderTexture : register(t0);
Texture2D specHighTex   : register(t1);

Texture2D areaTexture : register(t2);
Texture2D positionTex   : register(t3);
Texture2D normalTex     : register(t4);



Texture2D searchTex : register(t5);
Texture2D edgesTex : register(t6);
Texture2D blendTex : register(t7);

Texture2D smaaReadyTex : register(t8);

Texture2D tangentTex : register(t9);
Texture2D bitangentTex : register(t10);

SamplerState SampleType;

SamplerState SamplerWrap
{
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MaxAnisotropy = 1;
    AddressU = Wrap;
    AddressV = Wrap;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
    matrix projectionMatrix : TEXCOORD1;
    matrix viewMatrix : TEXCOORD10;
};

cbuffer PostProcessData : register(b0)
{
	float2 screenWH;
	float expa;
    float ssaoBias;
    float ssaoRadius;
	float4 ssaoKernel[64]; 
    matrix projectionMatrix;
    matrix lightViewMat;
    matrix lightProjectionMat;
    float4 cameraPosition;
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


/*===SMAA PIXEL INPUTS===*/
struct SmaaPSInputType1
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float4 offset[3] : TEXCOORD1;
};

struct SmaaPSInputType2
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float2 pixcoord : TEXCOORD1;
    float4 subsampleIndices : TEXCOORD2;
    float4 offset[3] : TEXCOORD3;

};

struct SmaaPSInputType3
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float4 offset : TEXCOORD1;
};

#include "ToneMapping.hlsl"

float2 SmaaPSStage1(SmaaPSInputType1 input) : SV_TARGET
{
    return SMAALumaEdgeDetectionPS(input.texcoord, input.offset, shaderTexture);
}

float4 SmaaPSStage2(SmaaPSInputType2 input) : SV_TARGET
{
    
    return SMAABlendingWeightCalculationPS(input.texcoord, input.pixcoord, input.offset, edgesTex, areaTexture, searchTex, float4(1.0f,1.0f,1.0f,1.0f));
}

float4 SmaaPSStage3(SmaaPSInputType3 input) : SV_TARGET
{
    return SMAANeighborhoodBlendingPS(input.texcoord, input.offset, shaderTexture, blendTex);
}



float4 ssaoBlur(PixelInputType input) : SV_TARGET
{
    float4 blurredColor = 1.0f;



    return blurredColor;
}


// Performs a gaussian blur in one direction
float4 Blur1(PixelInputType input, float2 texScale, float sigma)
{
    float2 inputSize = 0.0f;
    specHighTex.GetDimensions(inputSize.x, inputSize.y);

    float4 color = 0;
    for (int i = -6; i < 6; i++)
    {
        float weight = CalcGaussianWeight(i, sigma);
        float2 texCoord = input.tex;
        texCoord += (i / inputSize) * texScale;
        float4 sample = specHighTex.Sample(PointSampler, texCoord);
        color += sample * weight;
    }

    return color;
}

// Performs a gaussian blur in one direction
float4 Blur(PixelInputType input, Texture2D shaderTexture1, float2 texScale, float sigma, bool nrmlize)
{
	float2 inputSize = 0.0f;
    shaderTexture1.GetDimensions(inputSize.x, inputSize.y);

	float4 color = 0;
	float weightSum = 0.0f;
	
	for (int i = -7; i < 7; i++)
	{

		float weight = CalcGaussianWeight(i, sigma);
		weightSum += weight;
		float2 texCoord = input.tex;
		texCoord += (i / inputSize) * texScale;
        float4 samples = shaderTexture1.Sample(SampleType, texCoord);
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
	
	float4 reds   = specHighTex.GatherRed  (SampleType, input.tex, int2(1, 1));
	float4 greens = specHighTex.GatherGreen(SampleType, input.tex, int2(1, 1));
	float4 blues  = specHighTex.GatherBlue (SampleType, input.tex, int2(1, 1));

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

		//textureColor *= ToneMap(textureColor);
		textureColor += (Bloom(input) + (Blur(input, specHighTex, float2(0, 1), sigma, false) + Blur(input, specHighTex, float2(1, 0), sigma, false)));
	}
	
	

	return textureColor;
}


float4 BlurVertical(PixelInputType input) : SV_TARGET
{
	float sigma = expa;
	return Blur1(input, float2(0, 1), sigma);
}

float4 BlurHorizontal(PixelInputType input) : SV_TARGET
{
	float sigma = expa;
    return Blur1(input, float2(1, 0), sigma);
}


float4 BloomColors(PixelInputType input) : SV_TARGET
{
	
	return Bloom(input);
}

float4 ReturnTexture(PixelInputType input) : SV_TARGET
{
	return shaderTexture.Sample(SampleType, input.tex);
}

//Combines the blurred and not blurred images

float4 Combine(PixelInputType input) : SV_TARGET
{
	float2 TexPos = input.tex;
	float4 textureColor = 0.0f;
	float4 textureColor1 = 0.0f;
	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
	FxaaTex fColors;
	fColors.smpl = SampleType;
	fColors.tex =  shaderTexture;
	textureColor = float4(FxaaPixelShader(TexPos, fColors, float2(1.0 / screenWH.x, 1.0 / screenWH.y)), 1.0);

	//
	fColors.tex = specHighTex;
	textureColor1 = float4(FxaaPixelShader(TexPos, fColors, float2(1.0 / screenWH.x, 1.0 / screenWH.y)), 1.0);

    if (expa <= 0.1)
        return float4(textureColor.xyz, 1.0f);
    else
        return textureColor + textureColor1;
      
	
}

float2 texOffset1(int u, int v, float2 texsize)
{
    return float2(u * 2.0f / texsize.x, v * 2.0f / texsize.y);
}

//This is used as blur for ssao now
float4 ToneMapPass(PixelInputType input) : SV_Target
{
    const int uBlurSize = 2;
    float2 inputSize = 0.0f;
    specHighTex.GetDimensions(inputSize.x, inputSize.y);

    float2 TexPos = input.tex;

    float4 texColor = 0.0f;


    //float2 hlim = float2(float(-uBlurSize) * 0.5 + 0.5, float(-uBlurSize) * 0.5 + 0.5);
    for (int i = -2; i < uBlurSize; ++i)
    {
        for (int j = -2; j < uBlurSize; ++j)
        {
            //float2 offset = (hlim + float2(float(i), float(j))) * inputSize;
            texColor += specHighTex.Sample(SampleType, TexPos + texOffset1(i, j, inputSize));
        }
    }
 
    texColor = texColor / 16;

    return texColor;
}

float getShadow(Texture2D shaderShadow, SamplerState SampleTypeShadow, float4 lightMatrix, float shBias)
{
    float vis = 0.1f;
    lightMatrix.xyz = lightMatrix.xyz / lightMatrix.w;
	
    if (lightMatrix.x < -1.0f || lightMatrix.x > 1.0f ||
		lightMatrix.y < -1.0f || lightMatrix.y > 1.0f ||
		lightMatrix.z < 0.0f || lightMatrix.z > 1.0f
		)
    {
        return 1.0f;
    }
	
    lightMatrix.x = lightMatrix.x / 2.0f + 0.5f;
    lightMatrix.y = lightMatrix.y / -2.0f + 0.5f;
	
    float bias = shBias;

    float shaderTex = shaderShadow.Sample(SampleTypeShadow, lightMatrix.xy).r;
			
    if (shaderTex > lightMatrix.z - bias)
    {
        vis = 0.1f;
    }
    else
    {
        vis = 1.0f;
    }


    return vis;
}


float4 SSAO(PixelInputType input) : SV_TARGET
{
    float4 position = positionTex.Sample(SampleType, input.tex);
    //float3 vsNormal = tangentTex.Sample(SampleType, input.tex); //shadow map

    //float4 lightMatrix = mul(input.position, lightViewMat);
    //lightMatrix = mul(lightMatrix, lightProjectionMat);
    //
    //
    //float3 shadow = getShadow(tangentTex, PointSampler, lightMatrix, 0.05);

    float4 fogColor = float4(0.5f, 0.5f, 0.5f, 1.0f);
    //float fogPower = expa;


    //float dist = position.z + expa;


  
    return fogColor;
    
}

/*
//http://www.learnopengl.com/#!Advanced-Lighting/SSAO changed to hlsl
float4 SSAO(PixelInputType input) : SV_TARGET
{
    float2 noiseScale = float2(screenWH.x / 4.0f, screenWH.y / 4.0f);

    float4 position = positionTex.Sample(SampleType, input.tex);
    //float3 normal = normalTex.Sample(SampleType, input.tex).xyz;
    //normal = normalize(normal);
    float3 randomF = areaTexture.Sample(SamplerWrap, input.tex * noiseScale).xyz;
    randomF = normalize(randomF);



    float3 vsNormal = tangentTex.Sample(SampleType, input.tex);
    vsNormal = normalize(vsNormal);


    float3 tangent = normalize(randomF - vsNormal.xyz * dot(randomF, vsNormal.xyz));
    float3 binormal = normalize(cross(vsNormal.xyz, tangent));
 
    //float3 tangent = tangentTex.Sample(SampleType, input.tex) * 2.0f - 1.0f;
    //float3 bitangent = bitangentTex.Sample(SampleType, input.tex) * 2.0f - 1.0f;

    matrix TBN = float4x4(float4(tangent, 1.0), float4(binormal, 1.0f), float4(vsNormal, 1.0f), float4(1.0f, 1.0f, 1.0f, 1.0f));

	float occlusion = 0.0f;

    float radius = ssaoRadius;
    float bias = ssaoBias;

	for (int i = 0; i < 64; i++)
	{
        float3 sample = mul(ssaoKernel[i], TBN);
        sample = position.xyz + sample * radius;

		float4 offset = float4(sample, 1.0f);

        offset = mul(projectionMatrix, offset);
		offset.xy /= offset.w; // perspective divide
	    offset.xy = offset.xy * 0.5 + 0.5; // transform to range 0.0 - 1.0
		
        float sampleDepth = positionTex.Sample(SampleType, offset.xy).z;

        float rangeCheck = smoothstep(0.0f, 1.0, radius / abs(position.z - sampleDepth));

        occlusion += (sampleDepth <= sample.z - bias ? 1.0f : 0.0f) * rangeCheck;
    }

	occlusion = 1 - (occlusion / 64);
    occlusion = pow(occlusion, expa);
    return float4(occlusion, occlusion, occlusion, 1.0f);
    //return float4(vsNormal, 1.0f);
}					  
*/