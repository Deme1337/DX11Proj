
Texture2D colorTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D specularTexture : register(t2);
Texture2D positionTexture : register(t3);
Texture2D roughnessTexture : register(t4);

Texture2D shadowMapTexture : register(t5);


SamplerState SampleTypePoint : register(s0);
SamplerComparisonState SampleTypeShadow : register(s1);


cbuffer LightBuffer
{
    float4 lightDirection;
	float4 lightColor;
	matrix lightViewMatrix;
	matrix lightProjectionMatrix;
};

#define POINT_LIGHT_COUNT 40

cbuffer PointLightBuffer
{
	float4 PointLightPosition[POINT_LIGHT_COUNT];
	float4 PointLightColor[POINT_LIGHT_COUNT];
};


struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 viewDir : TEXCOORD1;

};

#include "Commons.hlsl"

float G1V(float dotNV, float k)
{
	return 1.0f / (dotNV*(1.0f - k) + k);
}

float LightingFuncGGX_REF(float3 N, float3 V, float3 L, float roughness, float F0)
{
	float alpha = roughness*roughness;

	float3 H = normalize(V + L);

	float dotNL = saturate(dot(N, L));
	float dotNV = saturate(dot(N, V));
	float dotNH = saturate(dot(N, H));
	float dotLH = saturate(dot(L, H));

	float F, D, vis;

	// D
	float alphaSqr = alpha*alpha;
	float pi = 3.14159f;
	float denom = dotNH * dotNH *(alphaSqr - 1.0) + 1.0f;
	D = alphaSqr / (pi * denom * denom);

	// F
	float dotLH5 = pow(1.0f - dotLH, 5);
	F = F0 + (1.0 - F0)*(dotLH5);


	// V
	float k = alpha / 2.0f;
	vis = G1V(dotNL, k)*G1V(dotNV, k);

	float specular = dotNL * D * F * vis;
	return specular;
}

struct LightPixelShaderOutput
{
	float4 color : SV_Target0;
	float4 specular : SV_Target1;
};


float4 PointLightCalculation(float4 Ppos, float4 Pcolor, float4 ColorTex, float3 Normals, 
	float Spec, float Roughness, float4 FragPos, float3 View)
{
	float4 PointLightColor = 0.0f;
	float4 DiffuseColor = 0.0f;
	float4 SpecularColor = 0.0f;


	
	//Diffuse color
	float3 lightDir = normalize(Ppos - FragPos);
	float NdotL = saturate(dot(Normals, lightDir));

	if (NdotL < 0.0f)
	{
		return float4(0.0f,0.0f,0.0f,1.0f);
	}

	DiffuseColor = Pcolor * NdotL * ColorTex;

	//Specular color
	SpecularColor = LightingFuncGGX_REF(Normals, View, lightDir, Roughness, 0.1f) * Spec * Pcolor;


	float dist = length(Ppos - FragPos);
	float atten = 10.0f / (1.0f + 0.09 * dist + 0.032 * (dist * dist));


	SpecularColor *= atten;
	DiffuseColor *= atten;

	PointLightColor = DiffuseColor + SpecularColor;

	return PointLightColor;
}

LightPixelShaderOutput LightPixelShader(PixelInputType input) : SV_TARGET
{
	LightPixelShaderOutput output;

	float4 colors;
	float4 normals;
	float3 lightDir;
	float lightIntensity;
	float4 outputColor;
	float4 ambientLight = 0.2;
	float4 specularColor;


	
	// Sample the colors from the color render texture using the point sampler at this texture coordinate location.
	colors = colorTexture.Sample(SampleTypePoint, input.tex);

	colors = pow(colors, 1.2);

	// Sample the normals from the normal render texture using the point sampler at this texture coordinate location.
	normals = normalTexture.Sample(SampleTypePoint, input.tex);

	specularColor = specularTexture.Sample(SampleTypePoint, input.tex);
	
	

	
	
	// Invert the light direction for calculations.
    lightDir = -lightDirection.xyz;

	float4 positionTex = positionTexture.Sample(SampleTypePoint, input.tex);
	positionTex /= positionTex.w;

	

	
	float4 lightMatrix = mul(positionTex, lightViewMatrix);
	lightMatrix = mul(lightMatrix, lightProjectionMatrix);
	
	float shadow = shadowAA(shadowMapTexture, SampleTypeShadow, lightMatrix);


	//input.viewDir is actually camera position
	float3 viewDirection = normalize(input.viewDir - positionTex.xyz);
	
	float3 H = normalize(lightDir + viewDirection);

    // Calculate the amount of light on this pixel.
    lightIntensity = saturate(dot(normals.xyz, lightDir));
	
	
	ambientLight *= colors;


	float a = roughnessTexture.Sample(SampleTypePoint, input.tex).x;
	

	float3 DiffuseLight;
	float4 specularLight;
	
	DiffuseLight = lightIntensity * colors * lightColor;


	specularLight = LightingFuncGGX_REF(normals.xyz, viewDirection, lightDir, a, 0.1f) * specularColor.r;

	float4 PointLightVal = 0.0f;

	for (int i = 0; i < POINT_LIGHT_COUNT; i++)
	{
		PointLightVal += PointLightCalculation(PointLightPosition[i], PointLightColor[i], colors, normals, specularColor.r, a, positionTex, viewDirection);
	}


	if (length(lightColor.xyz) > 0.01f)
	{
		output.color = saturate((float4(DiffuseLight, 1.0)* shadow + specularLight));
	}



	
	output.color += PointLightVal;

	output.color += ambientLight;

	if (shadow > 0.9 && length(lightColor.xyz) > 0.01f)
	{
		output.specular = saturate(ambientLight + (float4(DiffuseLight, 1.0) + specularLight)) * 0.4;
	}
	
	else
	{
		output.specular = float4(0.0,0.0,0.0,1.0);
	}
	
	if (positionTex.x > 3900.0f || positionTex.y > 3900.0f || positionTex.y > 3900.0f
		|| positionTex.x < -3900.0f || positionTex.y < -3900.0f || positionTex.y < -3900.0f)
	{
		output.color = colors;
		output.specular = float4(0.0, 0.0, 0.0, 1.0);
	}
		

	
	

	return output;
}
