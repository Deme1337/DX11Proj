////////////////////////////////////////////////////////////////////////////////
// Filename: texture.vs
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
	matrix projectionV;
};


//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	matrix projection : TEXCOORD1;
};


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
    float2 texcoord: TEXCOORD0;
    float4 offset : TEXCOORD1;
};



#define SMAA_RT_METRICS float4(1.0 / 1920.0, 1.0 / 1080.0, 1920.0, 1080.0)
#define SMAA_HLSL_4
#define SMAA_PRESET_ULTRA
#include "SMAA.hlsl"

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType TextureVertexShader(VertexInputType input)
{
	PixelInputType output;


	// Change the position vector to be 4 units for proper matrix calculations.
	input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);
	output.projection = projectionV;
	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;

	return output;
}


/*==========SMAA VERTEX SHADERS============*/

SmaaPSInputType1 SmaaVSStage1(VertexInputType input)
{
    SmaaPSInputType1 output;
 
    	// Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    output.texcoord = input.tex;
    float4 offsets[3];
    SMAAEdgeDetectionVS(input.tex, offsets);
    output.offset = offsets;

    return output;
}

SmaaPSInputType2 SmaaVSStage2(VertexInputType input)
{
    SmaaPSInputType2 output;

    	// Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);


    output.texcoord = input.tex;
    float2 pixcoords;
    float4 offsets[3];

    SMAABlendingWeightCalculationVS(input.tex, pixcoords, offsets);

    output.pixcoord = pixcoords;
    output.offset = offsets;

    //Not used?
    output.subsampleIndices = float4(1.0f, 1.0f, 1.0f, 1.0f);


    return output;
}

SmaaPSInputType3 SmaaVSStage3(VertexInputType input)
{
    
    SmaaPSInputType3 output;
    	// Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);



    output.texcoord = input.tex;
    float4 offsets;
    SMAANeighborhoodBlendingVS(input.tex, offsets);

    output.offset = offsets;

    return output;
}