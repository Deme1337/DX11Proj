
Texture2D shaderTexture : register(t0);
Texture2D shaderBumpMap : register(t1);
Texture2D shaderSpecular : register(t2);
Texture2D shaderRoughness : register(t3);

SamplerState SampleTypeWrap : register(s0);


cbuffer	ObjectData
{
    float4 objColor;
	int UseTextures;
	float roughnessOffset;
	float metallic;
    int IsPaper;
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
    float3 vsNormal : TEXCOORD3;
    float3 vsPosition : TEXCOORD4;
};

struct PixelOutputType
{
    float3 color : SV_Target0;
    float4 normal : SV_Target1;
	float2 specular : SV_Target2;
	float4 position : SV_Target3;
	float roughness : SV_Target4;
	float4 tangent : SV_Target5;
    float4 vsPosition : SV_Target6;
};

//http://www.learnopengl.com/#!Advanced-Lighting/SSAO
float LinearizeDepth(float depth)
{
	float z = depth * 2.0 - 1.0;
	return (2.0f * 0.1f * 1.0) / (1.0 + 0.1 - z * (1.0 - 0.1f));
}

PixelOutputType DeferredPixelShader(PixelInputType input) : SV_TARGET
{
	PixelOutputType output;

    //float linDepth = LinearizeDepth(input.vsPosition.z);

    if (shaderTexture.Sample(SampleTypeWrap, input.tex).w < 0.4 && input.HasAlpha == 1)
    {
        discard;
    }

    if (IsPaper == 1)
    {
        output.color = shaderTexture.Sample(SampleTypeWrap, input.tex).xyz * 2;
        output.normal = float4(1.0f, 1.0f, 1.0f, 0.8f);
        output.specular = float4(0.0f, 0.0f, 0.0f, 1.0f);
        output.position = input.worldPosition;
        output.roughness = float4(1.0f, 1.0f, 1.0f, 1.0f);
        output.tangent = float4(input.tangent, 1.0f);
        return output;
    }
    else
    {
        if (UseTextures > 0.9)
        {
	        // Sample the color from the texture and store it for output to the render target.
            output.color = shaderTexture.Sample(SampleTypeWrap, input.tex).xyz;
            output.specular.x = shaderSpecular.Sample(SampleTypeWrap, input.tex).x;
            output.specular.y = metallic;

            float3 bumpMap = shaderBumpMap.Sample(SampleTypeWrap, input.tex);

            bumpMap = (bumpMap * 2.0f) - 1.0f;

	        // Calculate the normal from the data in the bump map.
            float3 bumpNormal = (bumpMap.x * input.tangent) + (bumpMap.y * input.binormal) + (bumpMap.z * input.normal);
	
            output.normal = float4(bumpNormal, 1.0f);

            output.position = input.worldPosition;

            output.roughness = shaderRoughness.Sample(SampleTypeWrap, input.tex).x + roughnessOffset;

            output.vsPosition = float4(input.vsPosition.xyz, 1.0f);
            //output.vsPosition = input.worldPosition;
            output.tangent = float4(input.vsNormal, 1.0f);

        }
        else
        {
            output.color = objColor;
            output.specular.x = 1.0f;
            output.specular.y = metallic;
            output.position = input.worldPosition;
            output.normal = float4(input.normal, 1.0f);
            output.roughness = 1.0f + roughnessOffset;
            output.vsPosition = float4(input.vsPosition.xyz, 1.0f);
            //output.vsPosition = input.worldPosition;
            output.tangent = float4(input.vsNormal, 1.0f);
        }

    }



	
    return output;
}