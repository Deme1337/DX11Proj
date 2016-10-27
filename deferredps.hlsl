
Texture2D shaderTexture : register(t0);
Texture2D shaderBumpMap : register(t1);
Texture2D shaderSpecular : register(t2);
Texture2D shaderRoughness : register(t3);

SamplerState SampleTypeWrap : register(s0);


cbuffer	ObjectData
{
	int UseTextures;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
    float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float4 worldPosition : TEXCOORD1;
	int HasAlpha : TEXCOORD2;
};

struct PixelOutputType
{
    float3 color : SV_Target0;
    float4 normal : SV_Target1;
	float specular : SV_Target2;
	float4 position : SV_Target3;
	float roughness : SV_Target4;
};



PixelOutputType DeferredPixelShader(PixelInputType input) : SV_TARGET
{
	PixelOutputType output;


	if (UseTextures > 0.9)
	{
		// Sample the color from the texture and store it for output to the render target.
		output.color = shaderTexture.Sample(SampleTypeWrap, input.tex).xyz;
		output.specular = shaderSpecular.Sample(SampleTypeWrap, input.tex).x;


		float3 bumpMap = shaderBumpMap.Sample(SampleTypeWrap, input.tex);

		bumpMap = (bumpMap * 2.0f) - 1.0f;

		// Calculate the normal from the data in the bump map.
		float3 bumpNormal = (bumpMap.x * input.tangent) + (bumpMap.y * input.binormal) + (bumpMap.z * input.normal);
		output.normal = float4(bumpNormal, 1.0f);

		output.position = input.worldPosition;

		output.roughness = shaderRoughness.Sample(SampleTypeWrap, input.tex).x;

	}
	
	else
	{
		output.color = float3(1.0f, 1.0f, 1.0f);
		output.specular = 1.0f;
		output.position = input.worldPosition;
		output.normal = float4(input.normal,1.0f);
		output.roughness = 1.0f;
	}

	if (shaderTexture.Sample(SampleTypeWrap, input.tex).w < 0.4 && input.HasAlpha == 1)
	{
		discard;
	}
	
    return output;
}