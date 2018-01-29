
Texture2D colorTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D specularTexture : register(t2);
Texture2D positionTexture : register(t3);
Texture2D roughnessTexture : register(t4);
Texture2D tangentTexture: register(t5);
Texture2D binormalTexture : register(t6);

Texture2D shadowMapTexture : register(t7);

TextureCube envmapTexture : register(t8);
Texture2D irradianceTexture : register(t9);

Texture2D ssaoTexture : register(t10);

SamplerState SampleTypePoint : register(s0);
SamplerComparisonState SampleTypeShadow : register(s1);

//test anistotropic
SamplerState SamplerAnisotropic 
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 16;
	AddressU = Wrap;
	AddressV = Wrap;
};

cbuffer LightBuffer: register(b0)
{
    float4 lightDirection;
	float4 lightColor;
	matrix lightViewMatrix;
	matrix lightProjectionMatrix;
    matrix viewMatrix;
	float globalAmbient; 
    float shadowBias;
    float attenuation;
};

#define POINT_LIGHT_COUNT 40

cbuffer PointLightBuffer: register(b1)
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

struct LightPixelShaderOutput
{
    float4 color : SV_Target0;
    float4 specular : SV_Target1;
};

#include "Commons.hlsl"

// ===============================================================================================
// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
// ===============================================================================================
float2 Hammersley(uint i, uint N)
{
	float ri = reversebits(i) * 2.3283064365386963e-10f;
	return float2(float(i) / float(N), ri);
}

//http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
float3 ImportanceSampleGGX(float2 Xi, float roughness, float3 N)
{
	float a = roughness*roughness;

	float Phi = 2 * PI * Xi.x;
	float CosTheta = sqrt((1 - Xi.y) / (1 + (a*a - 1) * Xi.y));
	float SinTheta = sqrt(1 - CosTheta * CosTheta);

	float3 H;
	H.x = SinTheta * cos(Phi);
	H.y = SinTheta * sin(Phi);
	H.z = CosTheta;
	float3 UpVector = abs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
	float3 TangentX = normalize(cross(UpVector, N));
	float3 TangentY = cross(N, TangentX);

	// Tangent to world space
	return TangentX * H.x + TangentY * H.y + N * H.z;
}

float2 IntegrateBRDF(float Roughness, float NoV, float3 N)
{
	float3 V;
	V.x = sqrt(1.0f - NoV * NoV); // sin
	V.y = 0;
	V.z = NoV; // cos
	float A = 0;
	float B = 0;
	const uint NumSamples = 10;
	for (uint i = 0; i < NumSamples; i++)
	{
		float2 Xi = Hammersley(i, NumSamples);
		float3 H = ImportanceSampleGGX(Xi, Roughness, N);
		float3 L = 2 * dot(V, H) * H - V;
		float NoL = saturate(L.z);
		float NoH = saturate(H.z);
		float VoH = saturate(dot(V, H));
		if (NoL > 0)
		{
			//float G = G_Smith(Roughness, NoV, NoL);
			float G = Geometric_Smith_GGX(Roughness, NoV, NoL);
			float G_Vis = G * VoH / (NoH * NoV);
			float Fc = pow(1 - VoH, 5);
			A += (1 - Fc) * G_Vis;
			B += Fc * G_Vis;
		}
	}
	return float2(A, B) / NumSamples;
}


float3 PrefilterEnvMap(float Roughness, float3 R)
{
	float3 N = R;
	float3 V = R;
	float3 PrefilteredColor = 0;
	const uint NumSamples = 10;
	float TotalWeight = 0;
	for (uint i = 0; i < NumSamples; i++)
	{
		float2 Xi = Hammersley(i, NumSamples);
		float3 H = ImportanceSampleGGX(Xi, Roughness, N);
		float3 L = 2 * dot(V, H) * H - V;
		float NoL = saturate(dot(N, L));
		if (NoL > 0)
		{
			float4 envMap = envmapTexture.SampleLevel(SamplerAnisotropic, L, 0);
			PrefilteredColor +=  envMap.xyz * NoL;
			TotalWeight += NoL;
		}
	}
	return PrefilteredColor / TotalWeight;
}

float3 ApproximateSpecularIBL(float3 SpecularColor, float Roughness, float3 N, float3 V)
{
	float NoV = saturate(dot(N, V));
	float3 R = 2 * dot(V, N) * N - V;
	float3 PrefilteredColor = PrefilterEnvMap(Roughness, R);
	float2 EnvBRDF = IntegrateBRDF(Roughness, NoV, R);
	return PrefilteredColor * (SpecularColor * EnvBRDF.x + EnvBRDF.y);
}

float4 CalculateEnvironmentMap(float3 normals, float3 viewDir, float roughness)
{
	float a = roughness;
	float3 reflectVector = reflect(-viewDir, normals.xyz);
	float mipIndex = a* a * 8.0f;

	float4 envMap = envmapTexture.SampleLevel(SamplerAnisotropic, reflectVector, mipIndex);
	envMap = pow(envMap, 2.2);
	return envMap;
}

//Cook torrance lighting N = normal, V = View, L = light, a = roughness, albedo = color, metallic = specular texture
float3 CookTorrance(float3 N, float3 V, float3 L, float a, float3 albedo, float metallic, float3 radiance)
{
    float3 H = normalize(V + L);
    float3 F0 = 0.03f.xxx;
    float3 Cook = 0.0f;

    float3 metallic1 = float3(metallic, metallic, metallic);

    F0 = lerp(F0, albedo, metallic1);
    

    float NoH = dot(N, H);
    float NoV = dot(N, V);
    float NoL = dot(N, L);
    float VoH = dot(V, H);
    float HoV = dot(H, V);


    float NDF = DistributionGGX(N, H, a);
    float G = GeometrySmith(N, V, L, a);
    float3 F = FresnelSchlick1(max(HoV, 0.0), F0);

    float3 kS = F;
    float kD = float3(1.0f, 1.0f, 1.0f) - kS;
    kD *= 1.0 - metallic1;

    float3 nominator = NDF * G * F;
    float denom = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
    float3 specular = nominator / denom;

    NoL = saturate(NoL);
    float3 diffuseC = Diffuse_OrenNayar(albedo, a, NoV, NoL, VoH); //DiffuseLambertian(albedo);
    float3 specularC = specular;

    Cook = (kD * diffuseC + specularC) * radiance * NoL;

    Cook = Cook / (Cook + 1.0f);
    Cook = pow(Cook, 1.0f / 2.2f);

    return Cook;

}




float4 PointLightCalculation(PixelInputType input, float4 N, float r, float3 t, float3 bt, float4 pos, float metallic, float spec, float4 ambient)
{
    float3 Diffuse = 0.0f.xxx;
    float3 Specular = 0.0f.xxx;
    float4 color = 0.0f;
    float3 viewDirection = normalize(input.viewDir - pos.xyz);

    float3 rlAlbedo;
    float3 realSpec;

   

    for(int i = 0; i < POINT_LIGHT_COUNT; i++)
    {
        float3 albedo = colorTexture.Sample(SampleTypePoint, input.tex).rgb ;
        float3 lightDir = normalize(PointLightPosition[i].xyz - pos.xyz);

        if (metallic > 0.03f)
        {
            rlAlbedo = albedo;
            realSpec = lerp(0.03f.xxx, albedo, metallic);
        }
        else
        {
            rlAlbedo = albedo;
            realSpec = lerp(0.03f.xxx, albedo, spec);
        }
	

	



        float3 brdf_Disney = max(DisneyBRDF(albedo, Specular.xyz, N.xyz, r, lightDir, viewDirection, t.xyz, bt.xyz, Diffuse.xyz, input.tex, 1.0), 0.0);
	
        float3 envFresnel = Specular_F_Roughness(realSpec, r * r, N.xyz, viewDirection);
	

        float3 reflectVector = reflect(-viewDirection, N.xyz);
        float mipIndex = r * r * 8.0f;

        float3 envColor = envmapTexture.SampleLevel(SampleTypePoint, -reflectVector, mipIndex);
	    envColor = pow(envColor, 2.2);

        float3 realalbedoColor = saturate(Diffuse);

        float3 fragColor;
        float lightDistance = length(pos - PointLightPosition[i]);

       
        float attenuation = 1.0 / (lightDistance * lightDistance);

        fragColor = 1 * brdf_Disney + envFresnel * envColor * 1.0 + realalbedoColor * PointLightColor[i].xyz * (float3(1.0f, 1.0f, 1.0f) * 0.5) * attenuation;





        color += float4(fragColor,1.0);
    }

    return color;

}

const float G_SCATTERING = 0.2f;
const float NB_STEPS = 100.0f;

const matrix biasMat = matrix(
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0);

float ComputeScattering(float lightDotView)
{
    float result = 1.0f - G_SCATTERING * G_SCATTERING;
    result /= (4.0f * PI * pow(1.0f + G_SCATTERING * G_SCATTERING - (2.0f * G_SCATTERING) * lightDotView, 1.5f));
    return result;
}

/*

LightPixelShaderOutput LightPixelShader(PixelInputType input) : SV_TARGET
{
    LightPixelShaderOutput output;
    float3 Diffuse = 0.0f.xxx;
    float3 Specular = 0.0f.xxx;
    float Ambient = globalAmbient;
    float4 color = 0.0f;

    float3 Albedo = colorTexture.Sample(SampleTypePoint, input.tex).rgb * lightColor.xyz;
    color = float4(Albedo, 2.2f);

    float4 normals = normalTexture.Sample(SampleTypePoint, input.tex);


    float specularColor = specularTexture.Sample(SampleTypePoint, input.tex).r;
    float roughness = roughnessTexture.Sample(SampleTypePoint, input.tex).x;
	//float3 tangent = normalize(cross(float3(0,1,0), normals.xyz));
	//float3 binormal = normalize(cross(normals.xyz, tangent));

    float3 lightDir = mul(normalize(lightDirection), viewMatrix).xyz;
    float4 positionTex = positionTexture.Sample(SampleTypePoint, input.tex);

    //float3 viewDirection = normalize(positionTex.xyz - mul(float4(input.viewDir, 1.0f), viewMatrix).xyz);
    float3 viewDirection = normalize(positionTex.xyz - input.viewDir);


    output.color = positionTex;
    output.specular = positionTex;

    return output;
}

*/

//Cook torrance lighting N = normal, V = View, L = light, a = roughness, albedo = color, metallic = specular texture
float3 CookTorrance(float3 N, float3 V, float3 L, float a, float3 albedo, float3 metallic, float3 radiance, float shadow, float3 envMap)
{
    float3 H = normalize(V + L);
    float3 F0 = 0.03f.xxx;
    float3 Cook = 0.0f;

    float3 metallic1 = metallic;


    float NoH = saturate(dot(N, H));
    float NoV = saturate(dot(N, V));
    float NoL = saturate(dot(N, L));
    float VoH = saturate(dot(V, H));
    float HoV = saturate(dot(H, V));


    float NDF = DistributionGGX(N, H, a);
    float G = GeometrySmith(N, V, L, a);
    float3 F = FresnelSchlick(max(HoV, 0.0), F0);

    float3 kS = F;
    float kD = float3(1.0f, 1.0f, 1.0f) - kS;
    kD *= 1.0 - metallic1;

    float3 nominator = NDF * G * F;
    float denom = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
    float3 specular = nominator / denom;

    float3 diffuseC = DiffuseLambertian(albedo);
    float3 specularC = pow(specular, (1.0f / 2.2f));

    Cook = kD * diffuseC * shadow + specularC * envMap;
    Cook *= radiance * NoL;
    Cook = Cook / (Cook + 1.0f);
    Cook = pow(Cook, 1.0f / 2.2f);

    return Cook;

}

LightPixelShaderOutput LightPixelShader(PixelInputType input) : SV_TARGET
{
    LightPixelShaderOutput output;
    float3 Diffuse = 0.0f.xxx;
    float3 Specular = 0.0f.xxx;
    float Ambient = globalAmbient;
    float3 color = 0.0f;

    float3 Albedo = colorTexture.Sample(SampleTypePoint, input.tex).rgb * lightColor.xyz;
    color = pow(Albedo, (1.0f / 2.2f));
    float4 normals = normalTexture.Sample(SampleTypePoint, input.tex);
    float specularColor = specularTexture.Sample(SampleTypePoint, input.tex).r;
    float roughness = roughnessTexture.Sample(SampleTypePoint, input.tex).x;
	//float3 tangent = normalize(cross(float3(0,1,0), normals.xyz));
	//float3 binormal = normalize(cross(normals.xyz, tangent));
    float3 tangent = tangentTexture.Sample(SampleTypePoint, input.tex).xyz;
    float3 binormal = binormalTexture.Sample(SampleTypePoint, input.tex).xyz;

    float4 positionTex = positionTexture.Sample(SampleTypePoint, input.tex);
    float3 lightDir = normalize(lightDirection);
    float Metallic = specularTexture.Sample(SampleTypePoint, input.tex).g;

    float3 viewDirection = normalize(input.viewDir - positionTex.xyz);
    float3 pixelColor = 0.0f;

    float3 rlAlbedo;
    float3 realSpec;

    //check if it is 2d or skydome so use only color
    if (normals.w < 1.0f && normals.w > 0.2)
    {
        output.color = float4(color, 1.0);
        output.specular = float4(0.0, 0.0, 0.0, 0.5);
        return output;
    }

    if (Metallic >= 0.03f)
    {
        rlAlbedo = color;
        realSpec = lerp(0.03f.xxx, color, Metallic);
    }
    else
    {
        rlAlbedo = color;
        realSpec = lerp(0.03f.xxx, color, specularColor);
    }

    float3 radiance = lightColor.xyz * attenuation;



    float4 lightMatrix = mul(positionTex, lightViewMatrix);
    lightMatrix = mul(lightMatrix, lightProjectionMatrix);

    float shadow = shadowAA(shadowMapTexture, SampleTypeShadow, lightMatrix, shadowBias, globalAmbient);


    //Environment map
    float3 reflectVector = reflect(-viewDirection, normals.xyz);
    float mipIndex = roughness * roughness * 8.0f;

    float3 envColor = envmapTexture.SampleLevel(SampleTypePoint, -reflectVector, mipIndex);
    envColor = pow(envColor, (1.0f / 2.2f));


    pixelColor = CookTorrance(normals.xyz, viewDirection, lightDir, roughness, rlAlbedo, realSpec, radiance, shadow, envColor);

    pixelColor = pixelColor + globalAmbient * Albedo;

    output.color = pow(float4(pixelColor, 1.0f), 2.2f);

    if(shadow > 0.90f)
        output.specular = float4(pixelColor, 1.0f);
    else
        output.specular = float4(0.0f, 0.0f, 0.0f, 1.0f);

    return output;

}


/**********************
     DISNEY BRDF
**********************/
/*
LightPixelShaderOutput LightPixelShader(PixelInputType input) : SV_TARGET
{
	LightPixelShaderOutput output;
	float3 Diffuse = 0.0f.xxx;
	float3 Specular = 0.0f.xxx;
	float Ambient = globalAmbient;
	float4 color = 0.0f;

	float3 Albedo = colorTexture.Sample(SampleTypePoint, input.tex).rgb * lightColor.xyz;
	color = float4(Albedo, 2.2f);
	float4 normals =  normalTexture.Sample(SampleTypePoint, input.tex);
	float specularColor = specularTexture.Sample(SampleTypePoint, input.tex).r;
	float roughness = roughnessTexture.Sample(SampleTypePoint, input.tex).x;
	//float3 tangent = normalize(cross(float3(0,1,0), normals.xyz));
	//float3 binormal = normalize(cross(normals.xyz, tangent));
	float3 tangent = tangentTexture.Sample(SampleTypePoint, input.tex).xyz;
	float3 binormal	= binormalTexture.Sample(SampleTypePoint, input.tex).xyz;

	float4 positionTex = positionTexture.Sample(SampleTypePoint, input.tex);
    float3 lightDir = -normalize(lightDirection);
	float Metallic = specularTexture.Sample(SampleTypePoint, input.tex).g;

	float3 viewDirection = normalize(positionTex.xyz - input.viewDir);

	float3 rlAlbedo;
	float3 realSpec;

    //check if it is 2d or skydome so use only color
    if (normals.w < 1.0f && normals.w > 0.2)
    {
        output.color = float4(Albedo,1.0);
        output.specular = float4(0.0, 0.0, 0.0, 0.5);
        return output;
    }

	if (Metallic >= 0.03f)
	{
		 rlAlbedo = Albedo;
		 realSpec = lerp(0.03f.xxx, Albedo, Metallic);
	}
	else
	{
		rlAlbedo = Albedo;
		realSpec = lerp(0.03f.xxx, Albedo, specularColor);
	}
	
    
    float4 lightMatrix = mul(positionTex, lightViewMatrix);
    lightMatrix = mul(lightMatrix, lightProjectionMatrix);

    float shadow = shadowAA(shadowMapTexture, SampleTypeShadow, lightMatrix, shadowBias, globalAmbient);
	
    //float shadow = softShadow(shadowMapTexture, SampleTypePoint, lightMatrix, shadowBias);

    float3 brdf_Disney = DisneyBRDF(Albedo, Specular.xyz, normals.xyz, roughness, -lightDir, viewDirection, tangent.xyz, binormal.xyz, Diffuse.xyz, input.tex, shadow);
	
	float3 envFresnel = Specular_F_Roughness(realSpec, roughness * roughness, normals.xyz, viewDirection);
	

	float3 reflectVector = reflect(-viewDirection, normals.xyz);
	float mipIndex = roughness* roughness * 8.0f;

	float3 envColor = envmapTexture.SampleLevel(SampleTypePoint, -reflectVector, mipIndex);
	envColor = pow(envColor, 2.2);

	float3 realalbedoColor = saturate(Diffuse);

    


   
    float3 fragColor; 

    //float AmbientOcclusion = ssaoTexture.Sample(SampleTypePoint, input.tex).r;
    
    float3 irradianceColor = 0.5f.xxx;
    float3 reflectionIntensity = 0.2f.xxx;

    //terrain for now without shadow
    if(normals.w < 0.3f)
    {
        fragColor = attenuation * 1.0 * brdf_Disney + envFresnel * envColor * reflectionIntensity + realalbedoColor * irradianceColor * globalAmbient;
    }
    else
    {
        fragColor = attenuation * shadow * brdf_Disney + envFresnel * envColor * reflectionIntensity + realalbedoColor * irradianceColor * globalAmbient;
    }
	
    output.color = float4(fragColor, 1.0f); //PointLightCalculation(input, normals, roughness, tangent, binormal, positionTex, Metallic, specularColor, Ambient);

	if (shadow > 0.98)
	{
        output.specular = float4(fragColor, 1.0);
    }
	else
	{
        output.specular = 0.0f.xxxx;
    }

	

	return output;
}
*/