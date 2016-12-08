
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
    float texOffSet;
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
    
    
	// Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
	output.worldPosition = output.position;
	//Viewmatrix is mul(proj,view)
    output.position = mul(output.position, viewMatrix);

    
	// Store the texture coordinates for the pixel shader.
	float2 newTex = input.tex;
    output.tex = newTex * texOffSet;
    
	// Calculate the normal vector against the world matrix only.
    output.normal = mul(input.normal, (float3x3)worldMatrix);
	
    // Normalize the normal vector.
    output.normal = normalize(output.normal);

	// Calculate the tangent vector against the world matrix only and then normalize the final value.
	output.tangent = mul(input.tangent, (float3x3)worldMatrix);
	output.tangent = normalize(output.tangent);

	// Calculate the binormal vector against the world matrix only and then normalize the final value.
	output.binormal = mul(input.binormal, (float3x3)worldMatrix);
	output.binormal = normalize(output.binormal);

	output.HasAlpha = HasAlpha;

	return output;
}