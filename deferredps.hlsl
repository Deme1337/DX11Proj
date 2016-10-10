
Texture2D shaderTexture : register(t0);
Texture2D shaderBumpMap : register(t1);
Texture2D shaderSpecular : register(t2);
Texture2D shaderRoughness : register(t3);

SamplerState SampleTypeWrap : register(s0);



struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
    float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float4 worldPosition : TEXCOORD1;
};

struct PixelOutputType
{
    float4 color : SV_Target0;
    float4 normal : SV_Target1;
	float4 specular : SV_Target2;
	float4 position : SV_Target3;
	float4 roughness : SV_Target4;
};



PixelOutputType DeferredPixelShader(PixelInputType input) : SV_TARGET
{
	PixelOutputType output;


	// Sample the color from the texture and store it for output to the render target.
	output.color = shaderTexture.Sample(SampleTypeWrap, input.tex);
	output.specular = shaderSpecular.Sample(SampleTypeWrap, input.tex);
	

	float3 bumpMap = shaderBumpMap.Sample(SampleTypeWrap, input.tex);

	bumpMap = (bumpMap * 2.0f) - 1.0f;

	// Calculate the normal from the data in the bump map.
	float3 bumpNormal = (bumpMap.x * input.tangent) + (bumpMap.y * input.binormal) + (bumpMap.z * input.normal);
	output.normal = float4(bumpNormal,1.0f);

	output.position = input.worldPosition;

	output.roughness = shaderRoughness.Sample(SampleTypeWrap, input.tex);
	/*
	if (output.color.a < 0.9)
	{
		discard;
	}
	*/
    return output;
}