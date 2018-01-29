
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
    float texOffSetX;
    float texOffSetY;
	int HasAlpha;
};



struct VertexInputType
{
    float4 position : POSITION;
    float2 tex      : TEXCOORD0;
	float3 normal   : NORMAL;
	float3 tangent  : TANGENT;
	float3 binormal : BINORMAL;
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


PixelInputType DeferredVertexShader(VertexInputType input)
{
    PixelInputType output;
    
    
	
    input.position.w = 1.0f;

    output.position = mul(input.position, worldMatrix);
    output.worldPosition = output.position;
    output.position = mul(output.position, viewMatrix);
	
	float2 newTex = input.tex;
    output.tex.x = newTex.x * texOffSetX;
    output.tex.y = newTex.y * texOffSetY;



    output.normal = mul(input.normal, (float3x3)worldMatrix);
    output.normal = normalize(output.normal);


	output.tangent = mul(input.tangent, (float3x3)worldMatrix);
	output.tangent = normalize(output.tangent);

	
	output.binormal = mul(input.binormal, (float3x3)worldMatrix);
	output.binormal = normalize(output.binormal);

	output.HasAlpha = HasAlpha;

	return output;
}