

cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
	float4 sunPosition;
};



struct VertexInputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float4 normal : NORMAL;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float4 domePosition : TEXCOORD0;
	float4 sunPosition : TEXCOORD1;
	float2 texCoord : TEXCOORD2;
	float4 normals : TEXCOORD3;
};


PixelInputType SkyDomeVertexShader(VertexInputType input)
{
	PixelInputType output;
	float PI = 3.14159265359;

	// Change the position vector to be 4 units for proper matrix calculations.
	input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(input.position, worldMatrix);
	output.normals = input.position;

	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	// Send the unmodified position through to the pixel shader.
	output.domePosition = input.position;
	output.sunPosition = sunPosition;
	output.texCoord.x = asin(input.normal.x) / PI + 0.5;
	output.texCoord.y = asin(input.normal.y) / PI + 0.5;

	

	return output;
}