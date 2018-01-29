
cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer CameraBuffer :register(b1)
{
	float4 CameraPosition;
};

struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 viewDir : TEXCOORD1;
};



PixelInputType LightVertexShader(VertexInputType input)
{
    PixelInputType output;
	float4 worldPosition;
    
    
	// Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
	worldPosition = output.position;
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);


 
	// Store the texture coordinates for the pixel shader.
    output.tex = input.tex;
    
	output.viewDir = CameraPosition;

	return output;
}