////////////////////////////////////////////////////////////////////////////////
// Filename: terrain.ps
////////////////////////////////////////////////////////////////////////////////


//////////////
// TEXTURES //
//////////////
Texture2D shaderTexture : register(t0);
Texture2D normalTexture : register(t1);


//////////////
// SAMPLERS //
//////////////
SamplerState SampleType : register(s0);


//////////////////////
// CONSTANT BUFFERS //
//////////////////////
cbuffer LightBuffer
{
	float4 diffuseColor;
	float3 lightDirection;
	float padding;
};


//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float4 color : COLOR;
    float4 worldPosition : TEXCOORD1;
};

struct PixelOutputType
{
    float3 color : SV_Target0;
    float4 normal : SV_Target1;
    float2 specular : SV_Target2;
    float4 position : SV_Target3;
    float roughness : SV_Target4;
    float4 tangent : SV_Target5;
    float4 binormal : SV_Target6;
};


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 TerrainPixelShader(PixelInputType input) : SV_TARGET
{
	float4 textureColor;
	float3 lightDir;
	float4 bumpMap;
	float3 bumpNormal;
	float lightIntensity;
	float4 color;


	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
	textureColor = shaderTexture.Sample(SampleType, input.tex);

	// Combine the color map value into the texture color.
	textureColor = saturate(textureColor * input.color * 2.0f);

	// Invert the light direction for calculations.
	lightDir = -lightDirection;

	// Calculate the amount of light on this pixel using the normal map.
	bumpMap = normalTexture.Sample(SampleType, input.tex);
	bumpMap = (bumpMap * 2.0f) - 1.0f;
	bumpNormal = (bumpMap.x * input.tangent) + (bumpMap.y * input.binormal) + (bumpMap.z * input.normal);
	bumpNormal = normalize(bumpNormal);
	lightIntensity = saturate(dot(bumpNormal, lightDir));

	// Determine the final amount of diffuse color based on the diffuse color combined with the light intensity.
	color = saturate(diffuseColor * lightIntensity);

	// Multiply the texture pixel and the final diffuse color to get the final pixel color result.
	color = color * textureColor;

	return color;
}


PixelOutputType TerrainPixelShader1(PixelInputType input) : SV_TARGET
{
    PixelOutputType output;
    float4 bumpMap;
    float3 bumpNormal;
    float4 textureColor;

    textureColor = shaderTexture.Sample(SampleType, input.tex);
    textureColor = saturate(textureColor * input.color * 2.0f);
   
    output.color = textureColor;
    output.position = input.worldPosition;
    output.roughness = 1.0f;
    output.specular = float2(0.01f, 0.18f);
    

    bumpMap = normalTexture.Sample(SampleType, input.tex);
    bumpMap = (bumpMap * 2.0f) - 1.0f;
    bumpNormal = (bumpMap.x * input.tangent) + (bumpMap.y * input.binormal) + (bumpMap.z * input.normal);
    bumpNormal = normalize(bumpNormal);

    output.normal = float4(bumpNormal.xyz, 0.2f);
    output.tangent = float4(input.tangent, 1.0f);
    output.binormal = float4(input.binormal, 1.0f);

    return output;
}