//=================================================================================================
// Helper Functions
//=================================================================================================



//-------------------------------------------------------------------------------------------------
// Calculates the Fresnel factor using Schlick's approximation
//-------------------------------------------------------------------------------------------------
float3 Fresnel(float3 specAlbedo, float3 h, float3 l)
{
	float3 fresnel = specAlbedo + (1.0f - specAlbedo) * pow((1.0f - saturate(dot(l, h))), 5.0f);

		// Fade out spec entirely when lower than 0.1% albedo
		fresnel *= saturate(dot(specAlbedo, 333.0f));

	return fresnel;
}

//-------------------------------------------------------------------------------------------------
// Helper for computing the Beckmann geometry term
//-------------------------------------------------------------------------------------------------
float Beckmann_G1(float m, float nDotX)
{
	float nDotX2 = nDotX * nDotX;
	float tanTheta = sqrt((1 - nDotX2) / nDotX2);
	float a = 1.0f / (m * tanTheta);
	float a2 = a * a;

	float g = 1.0f;
	if (a < 1.6f)
		g *= (3.535f * a + 2.181f * a2) / (1.0f + 2.276f * a + 2.577f * a2);

	return g;
}

//-------------------------------------------------------------------------------------------------
// Computes the specular term using a Beckmann microfacet distribution, with a matching
// geometry factor and visibility term. Based on "Microfacet Models for Refraction Through
// Rough Surfaces" [Walter 07]. m is roughness, n is the surface normal, h is the half vector,
// l is the direction to the light source, and specAlbedo is the RGB specular albedo
//-------------------------------------------------------------------------------------------------
float Beckmann_Specular(float m, float3 n, float3 h, float3 v, float3 l)
{
	float Pi = 3.14159265359;
	float nDotH = max(dot(n, h), 0.0001f);
	float nDotL = saturate(dot(n, l));
	float nDotV = max(dot(n, v), 0.0001f);

	float nDotH2 = nDotH * nDotH;
	float nDotH4 = nDotH2 * nDotH2;
	float m2 = m * m;

	// Calculate the distribution term
	float tanTheta2 = (1 - nDotH2) / nDotH2;
	float expTerm = exp(-tanTheta2 / m2);
	float d = expTerm / (Pi * m2 * nDotH4);

	// Calculate the matching geometric term
	float g1i = Beckmann_G1(m, nDotL);
	float g1o = Beckmann_G1(m, nDotV);
	float g = g1i * g1o;

	return  d * g * (1.0f / (4.0f * nDotL * nDotV));
}

float2 poissonDisk[16] = {
	
	float2(-0.94201624, -0.39906216),
	float2(0.94558609, -0.76890725),
	float2(-0.094184101, -0.92938870),
	float2(0.34495938, 0.29387760),
	float2(-0.91588581, 0.45771432),
	float2(-0.81544232, -0.87912464),
	float2(-0.38277543, 0.27676845),
	float2(0.97484398, 0.75648379),
	float2(0.44323325, -0.97511554),
	float2(0.53742981, -0.47373420),
	float2(-0.26496911, -0.41893023),
	float2(0.79197514, 0.19090188),
	float2(-0.24188840, 0.99706507),
	float2(-0.81409955, 0.91437590),
	float2(0.19984126, 0.78641367),
	float2(0.14383161, -0.14100790)

	

};

float2 rand_2_10(float2 uv) {
	float noiseX = (frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453));
	float noiseY = sqrt(1 - noiseX * noiseX);
	return float2(noiseX, noiseY);
}

float random(float3 seed0,int seed1)
{
	float4 seed01 = float4(seed0, seed1);
	float dot_product = dot(seed01, float4(12.9898, 78.233, 45.164, 94.673));
	return frac(sin(dot_product) * 43758.5453);
}

float2 texOffset(int u, int v )
{
	return float2(u * 2.0f / 2048.0f, v * 2.0f / 2048.0f);
}



float shadowAA(Texture2D shaderShadow, SamplerComparisonState SampleTypeShadow, float4 lightMatrix)
{
	float visibility = 1.0f;
	lightMatrix.xyz = lightMatrix.xyz / lightMatrix.w;
	
	if (lightMatrix.x < -1.0f || lightMatrix.x > 1.0f ||
		lightMatrix.y < -1.0f || lightMatrix.y > 1.0f ||
		lightMatrix.z < 0.0f  || lightMatrix.z > 1.0f 
		)
	{
		return 0.4f;
	}
	
	lightMatrix.x = lightMatrix.x / 2.0f + 0.5f;
	lightMatrix.y = lightMatrix.y / -2.0f + 0.5f;
	
	float shadowBias = 0.00001f;

	float x, y;
	float LOS = 2;

	
	for (y = -LOS; y <= LOS; y += 1.0f)
	{	
		for (x = -LOS; x <= LOS; x += 1.0f)
		{
			float shaderTex = shaderShadow.SampleCmpLevelZero(SampleTypeShadow, lightMatrix.xy + texOffset(x, y) * 0.15, lightMatrix.z - shadowBias).r;
			if (shaderTex > lightMatrix.z - shadowBias)
			{
				visibility += 0.3;
			}

		}
	}
	
	

	if (visibility < 0.4f)
	{
		visibility = 0.4f;
	}

	visibility /= 4.0f;

	
	return visibility;
}
/*-----------------------------------DISNEY STUFF------------------------------------------*/
float PI = 3.14159265359;
#define NDF_GGX 1
#define FRESNEL_SCHLICK 1
#define GEOMETRIC_SMITH_SCHLICK_GGX 1
#define DISNEY_BRDF 1

#define g_Roughness              0.0f
#define g_Metallic               0.0f
#define g_OverrideAlbedo         0.0f
#define g_OverrideNormal         0.0f
#define g_OverrideMetallic       0.0f
#define g_OverrideRoughness      0.0f
#define g_OverrideSpecular       0.0f
#define g_LightIntensity         0.0f
#define g_AmbientLightIntensity  0.0f
#define g_ReflectionIntensity    0.0f


#define g_Subsurface            0.0f
#define g_Specular              0.0f
#define g_SpecularTint          0.0f
#define g_Anisotropic           0.0f
#define g_Sheen                 0.0f
#define g_SheenTint             0.0f
#define g_Clearcoat             0.0f
#define g_ClearcoatGloss        0.0f

float3 Diffuse(float3 pAlbedo)
{
	return pAlbedo / PI;
}
//-------------------------- Disney BRDF helpers functions --------------------------------------------
float sqr(float x)
{
	return x*x;
}

float GTR2_aniso(float NdH, float HdX, float HdY, float ax, float ay)
{
	return 1.0f / (PI * ax*ay * sqr(sqr(HdX / ax) + sqr(HdY / ay) + NdH*NdH));
}

float smithG_GGX(float NdV, float alphaG)
{
	float a = alphaG*alphaG;
	float b = NdV*NdV;
	return 1.0f / (NdV + sqrt(a + b - a*b));
}

float GTR1(float NdH, float a)
{
	if (a >= 1.0f)
	{
		return 1.0f / PI;
	}

	float a2 = a*a;
	float t = 1.0f + (a2 - 1.0f) * NdH * NdH;
	return (a2 - 1.0f) / (PI*log(a2)*t);
}

//-------------------------- Normal distribution functions --------------------------------------------
float NormalDistribution_GGX(float a, float NdH)
{
	// Isotropic ggx.
	float a2 = a*a;
	float NdH2 = NdH * NdH;

	float denominator = NdH2 * (a2 - 1.0f) + 1.0f;
	denominator *= denominator;
	denominator *= PI;

	return a2 / denominator;
}

float NormalDistribution_BlinnPhong(float a, float NdH)
{
	return (1 / (PI * a * a)) * pow(NdH, 2 / (a * a) - 2);
}

float NormalDistribution_Beckmann(float a, float NdH)
{
	float a2 = a * a;
	float NdH2 = NdH * NdH;

	return (1.0f / (PI * a2 * NdH2 * NdH2 + 0.001)) * exp((NdH2 - 1.0f) / (a2 * NdH2));
}

//-------------------------- Geometric shadowing -------------------------------------------
float Geometric_Implicit(float a, float NdV, float NdL)
{
	return NdL * NdV;
}

float Geometric_Neumann(float a, float NdV, float NdL)
{
	return (NdL * NdV) / max(NdL, NdV);
}

float Geometric_CookTorrance(float a, float NdV, float NdL, float NdH, float VdH)
{
	return min(1.0f, min((2.0f * NdH * NdV) / VdH, (2.0f * NdH * NdL) / VdH));
}

float Geometric_Kelemen(float a, float NdV, float NdL, float LdV)
{
	return (2 * NdL * NdV) / (1 + LdV);
}

float Geometric_Beckman(float a, float dotValue)
{
	float c = dotValue / (a * sqrt(1.0f - dotValue * dotValue));

	if (c >= 1.6f)
	{
		return 1.0f;
	}
	else
	{
		float c2 = c * c;
		return (3.535f * c + 2.181f * c2) / (1 + 2.276f * c + 2.577f * c2);
	}
}

float Geometric_Smith_Beckmann(float a, float NdV, float NdL)
{
	return Geometric_Beckman(a, NdV) * Geometric_Beckman(a, NdL);
}

float Geometric_GGX(float a, float dotValue)
{
	float a2 = a * a;
	return (2.0f * dotValue) / (dotValue + sqrt(a2 + ((1.0f - a2) * (dotValue * dotValue))));
}

float Geometric_Smith_GGX(float a, float NdV, float NdL)
{
	return Geometric_GGX(a, NdV) * Geometric_GGX(a, NdL);
}

float Geometric_Smith_Schlick_GGX(float a, float NdV, float NdL)
{
	// Smith schlick-GGX.
	float k = a * 0.5f;
	float GV = NdV / (NdV * (1 - k) + k);
	float GL = NdL / (NdL * (1 - k) + k);

	return GV * GL;
}

//-------------------------- Fresnel -------------------------------------
float3 Fresnel_None(float3 specularColor)
{
	return specularColor;
}

// Used by Disney BRDF.
float Fresnel_Schlick(float u)
{
	float m = saturate(1.0f - u);
	float m2 = m*m;
	return m2*m2*m;
}

float3 Fresnel_Schlick(float3 specularColor, float3 h, float3 v)
{
	return (specularColor + (1.0f - specularColor) * pow((1.0f - saturate(dot(v, h))), 5));
}

float3 Fresnel_CookTorrance(float3 specularColor, float3 h, float3 v)
{
	float3 n = (1.0f + sqrt(specularColor)) / (1.0f - sqrt(specularColor));
	float c = saturate(dot(v, h));
	float3 g = sqrt(n * n + c * c - 1.0f);

	float3 part1 = (g - c) / (g + c);
	float3 part2 = ((g + c) * c - 1.0f) / ((g - c) * c + 1.0f);

	return max(0.0f.xxx, 0.5f * part1 * part1 * (1 + part2 * part2));
}



float Specular_D(float a, float NdH)
{
#ifdef NDF_BLINNPHONG
	return NormalDistribution_BlinnPhong(a, NdH);
#else
#ifdef NDF_BECKMANN
	return NormalDistribution_Beckmann(a, NdH);
#else
#ifdef NDF_GGX
	return NormalDistribution_GGX(a, NdH);
#endif
#endif
#endif
}



float3 Specular_F(float3 specularColor, float3 h, float3 v)
{
#ifdef FRESNEL_NONE
	return Fresnel_None(specularColor);
#else
#ifdef FRESNEL_SCHLICK
	return Fresnel_Schlick(specularColor, h, v);
#else
#ifdef FRESNEL_COOKTORRANCE
	return Fresnel_CookTorrance(specularColor, h, v);
#endif
#endif
#endif
}

float3 Specular_F_Roughness(float3 specularColor, float a, float3 h, float3 v)
{
#ifdef FRESNEL_SCHLICK
	// Sclick using roughness to attenuate fresnel.
	return (specularColor + (max(1.0f - a, specularColor) - specularColor) * pow((1 - saturate(dot(v, h))), 5));
#else
#ifdef FRESNEL_NONE
	return Fresnel_None(specularColor);
#else
#ifdef FRESNEL_COOKTORRANCE
	return Fresnel_CookTorrance(specularColor, h, v);
#endif
#endif
#endif
}



float Specular_G(float a, float NdV, float NdL, float NdH, float VdH, float LdV)
{
#ifdef GEOMETRIC_IMPLICIT
	return Geometric_Implicit(a, NdV, NdL);
#else

#ifdef GEOMETRIC_NEUMANN
	return Geometric_Neumann(a, NdV, NdL);
#else

#ifdef GEOMETRIC_COOKTORRANCE
	return Geometric_CookTorrance(a, NdV, NdL, NdH, VdH);
#else

#ifdef GEOMETRIC_KELEMEN
	return Geometric_Kelemen(a, NdV, NdL, LdV);
#else

#ifdef GEOMETRIC_SMITH_BECKMANN
	return Geometric_Smith_Beckmann(a, NdV, NdL);
#else

#ifdef GEOMETRIC_SMITH_GGX
	return Geometric_Smith_GGX(a, NdV, NdL);
#else

#ifdef GEOMETRIC_SMITH_SCHLICK_GGX
	return Geometric_Smith_Schlick_GGX(a, NdV, NdL);
#endif
#endif
#endif
#endif
#endif
#endif
#endif
}

float3 Specular(float3 specularColor, float3 h, float3 v, float3 l, float a, float NdL, float NdV, float NdH, float VdH, float LdV)
{
	return ((Specular_D(a, NdH) * Specular_G(a, NdV, NdL, NdH, VdH, LdV)) * Specular_F(specularColor, v, h)) / (4.0f * NdL * NdV + 0.0001f);
}

float3 ComputeLight(float3 albedoColor, float3 specularColor, float3 normal, float roughness, float3 lightPosition, float3 lightColor, float3 lightDir, float3 viewDir)
{
	// Compute some useful values.
	float NdL = saturate(dot(normal, lightDir));
	float NdV = saturate(dot(normal, viewDir));
	float3 h = normalize(lightDir + viewDir);
	float NdH = saturate(dot(normal, h));
	float VdH = saturate(dot(viewDir, h));
	float LdV = saturate(dot(lightDir, viewDir));
	float a = max(0.001f, roughness * roughness);

	float3 cDiff = Diffuse(albedoColor);
	float3 cSpec = Specular(specularColor, h, viewDir, lightDir, a, NdL, NdV, NdH, VdH, LdV);

	return lightColor * NdL * (cDiff * (1.0f - cSpec) + cSpec);
}


// From Disney's BRDF explorer: https://github.com/wdas/brdf
float3 DisneyBRDF(float3 baseColor, out float3 specularColor, float3 normal, float roughness, float3 lightDir, float3 viewDir, float3 X, float3 Y, out float3 diffuse)
{
	// Compute some useful values.
	float NdL = saturate(dot(normal, lightDir));
	float NdV = saturate(dot(normal, viewDir));

	float3 h = normalize(lightDir + viewDir);
	float NdH = saturate(dot(normal, h));
	float VdH = saturate(dot(viewDir, h));
	float LdV = saturate(dot(lightDir, viewDir));
	float LdH = saturate(dot(lightDir, h));
	float a = max(0.001f, roughness * roughness);

	float luminance = 0.3f * baseColor.x + 0.6f * baseColor.y + 0.1f * baseColor.z;

	float3 tint = luminance > 0.0f ? baseColor / luminance : 1.0f.xxx; // Normalize luminance to isolate hue+sat.
	specularColor = lerp(g_Specular * 0.08f * lerp(1.0f.xxx, tint, g_SpecularTint), baseColor, g_Metallic);
	float3 CSheen = lerp(1.0f.xxx, tint, g_SheenTint);

	// Diffuse fresnel - go from 1 at normal incidence to .5 at grazing
	// and mix in diffuse retro-reflection based on roughness
	float FL = Fresnel_Schlick(NdL);
	float FV = Fresnel_Schlick(NdV);
	float Fd90 = 0.5f + 2.0f * LdH * LdH * a;
	float Fd = lerp(1.0f, Fd90, FL) * lerp(1.0f, Fd90, FV);

	// Based on Hanrahan-Krueger brdf approximation of isotropic bssrdf
	// 1.25 scale is used to (roughly) preserve albedo
	// Fss90 used to "flatten" retroreflection based on roughness
	float Fss90 = LdH * LdH * a;
	float Fss = lerp(1.0f, Fss90, FL) * lerp(1.0f, Fss90, FV);
	float ss = 1.25f * (Fss * (1.0f / (NdL + NdV + 0.0001f) - 0.5f) + 0.5f);

	// Specular
	float aspect = sqrt(1.0f - g_Anisotropic*0.9f);
	float ax = max(0.001f, sqr(a) / aspect);
	float ay = max(0.001f, sqr(a)*aspect);
	float Ds = GTR2_aniso(NdH, dot(h, X), dot(h, Y), ax, ay);
	float FH = Fresnel_Schlick(LdH);
	float3 Fs = lerp(specularColor, 1.0f.xxx, FH);
	float roughg = sqr(a*0.5f + 0.5f);
	float Gs = smithG_GGX(NdL, roughg) * smithG_GGX(NdV, roughg);

	// Sheen
	float3 Fsheen = FH * g_Sheen * CSheen;

	// Clearcoat (ior = 1.5 -> F0 = 0.04)
	float Dr = GTR1(NdH, lerp(0.1f, 0.001f, g_ClearcoatGloss));
	float Fr = lerp(0.04f, 1.0f, FH);
	float Gr = smithG_GGX(NdL, 0.25f) * smithG_GGX(NdV, 0.25f);
	diffuse = ((1.0f / PI) * lerp(Fd, ss, g_Subsurface) * baseColor + Fsheen) * (1.0f - g_Metallic);
	return (diffuse  + Gs*Fs*Ds + 0.25f*g_Clearcoat*Gr*Fr*Dr)*NdL;
}

