#version 420

#define MAX_POINT_LIGHTS 16
#define MAX_DIRECTIONAL_LIGHTS 4

struct PointLight {
	vec3 Position;
	float Radius;
};

struct DirectionalLight {
	vec3 InvDirection;
};

layout(std140) uniform Lights{
	int lights_pad1, lights_pad2;

	int pointLightCount;
	int directionalLightCount;
	PointLight       pointLights      [MAX_POINT_LIGHTS];
	DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
};

in vec3 WorldPos;
in vec4 LightPos;
in vec3 CameraPos;
in vec2 TexCoords;
in vec3 Normal;
in mat3 TBN;
in vec4 DirectionalLightPos[MAX_DIRECTIONAL_LIGHTS];

out vec4 FragColor;

// global props

uniform float ambiance;

// material parameters

uniform vec4  mat_albedo;
uniform float mat_metallic;
uniform float mat_roughness;
uniform float mat_reflectance;
uniform float mat_ao;

uniform float mat_hasAlbedoMap;
uniform float mat_hasNormalMap;
uniform float mat_hasMetallicMap;
uniform float mat_hasRoughnessMap;
uniform float mat_hasReflectanceMap;
uniform float mat_hasAoMap;

uniform sampler2D mat_albedoMap;
uniform sampler2D mat_normalMap;
uniform sampler2D mat_metallicMap;
uniform sampler2D mat_roughnessMap;
uniform sampler2D mat_reflectanceMap;
uniform sampler2D mat_aoMap;

uniform sampler2D mat_alphaMaskMap;
uniform float mat_hasAlphaMaskMap;

uniform float     mat_hasShadowMap;       // take out of material at some point
uniform sampler2D mat_shadowMap;          // take out of material at some point

uniform float       mat_hasShadowMap2;    // take out of material at some point
uniform samplerCube mat_shadowMap2;       // take out of material at some point

const float PI = 3.14159265359f;

// Math functions

float lerp(
	float a, 
	float b, 
	float w)
{
	return a + w * (b - a);
}

float linstep(
	float l, 
	float h, 
	float v)
{
	return clamp((v - l) / (h - l), 0.0, 1.0);
}

// Gamma correction

vec3 sRGBToLinear(
	vec3 sRGBCol)
{
	vec3 linearRGBLo = sRGBCol / 12.92f;
	vec3 linearRGBHi = pow((sRGBCol + 0.055f) / 1.055f, vec3(2.4f));
	vec3 linearRGB   = (length(sRGBCol) <= 0.04045f) ? linearRGBLo : linearRGBHi;
	return linearRGB;
}

vec3 linearToSRGB(
	vec3 linearCol)
{
	vec3 sRGBLo = linearCol * 12.92f;
	vec3 sRGBHi = (pow(abs(linearCol), vec3(1.0f / 2.4f)) * 1.055f) - 0.055f;
	vec3 sRGB   = (length(linearCol) <= 0.0031308f) ? sRGBLo : sRGBHi;
	return sRGB;
}

// Shadows

float DirectionalLightShadow(
	vec4 coords4)
{
	if (mat_hasShadowMap == 0) {
		return 1.0f;
	}

	vec3 coords = (coords4.xyz / coords4.w) * 0.5 + 0.5;
	vec2 moments = texture(mat_shadowMap, coords.xy).rg;
	float compare = coords.z;

	if (compare > 1.0) {
		return 1.0;
	}

	float p = step(compare, moments.x);
	float v = max(moments.y - moments.x * moments.x, 0.00002);

	float d = compare - moments.x;
	float pMax = linstep(0.2, 1.0, v / (v + d * d));

	return min(max(p, pMax), 1.0);
}

float PointLightShadow(
	vec3 NegL,
	float R)
{
	if (mat_hasShadowMap2 == 0) {
		return 1.0f;
	}

	float closestDepth = texture(mat_shadowMap2, NegL).r;
	float currentDepth = length(NegL);

	float bias = 0.05;
	float shadow = R * closestDepth > currentDepth - bias ? 1.0 : 0.0;

	return shadow;
}

// PBR BRDF

vec3 F_Schlick(
	vec3 f0, 
	float f90, 
	float u)
{ 
	return f0 + (f90 - f0) * pow(1.f - u, 5.f);
} 

float V_SmithGGXCorrelated(
	float NdotL, 
	float NdotV, 
	float roughness)
{ 
	float alphaG2 = roughness * roughness;
	float Lambda_GGXV = NdotL * sqrt((-NdotV * alphaG2 + NdotV) * NdotV + alphaG2);
	float Lambda_GGXL = NdotV * sqrt((-NdotL * alphaG2 + NdotL) * NdotL + alphaG2);
	
	return 0.5f / (Lambda_GGXV + Lambda_GGXL);
} 

float D_GGX(
	float NdotH, 
	float roughness) 
{ 
	float m2 = roughness * roughness;
	float f = (NdotH * m2 - NdotH) * NdotH + 1;
	return m2 / (f * f);
} 

float Fr_DisneyDiffuse(
	float NdotV, 
	float NdotL, 
	float LdotH, 
	float roughness) 
{ 
	float energyBias   = lerp(0,   0.5,        roughness);
	float energyFactor = lerp(1.0, 1.0 / 1.51, roughness);
	vec3 f0            = vec3(1.0f, 1.0f, 1.0f);
	float fd90         = energyBias + 2.0 * LdotH * LdotH * roughness;
	float lightScatter = F_Schlick(f0, fd90, NdotL).r;
	float viewScatter  = F_Schlick(f0, fd90, NdotV).r;
	
	return lightScatter * viewScatter * energyFactor;
}

// attenuation

float getDistanceAtt(
	vec3 unormalizedLightVector,
	float invSqrRadius)
{
	float dist2 = dot(unormalizedLightVector, unormalizedLightVector);
	return clamp(1.0 - dist2 * invSqrRadius, 0.0, 1.0);
}

vec3 BRDF(
	vec3 N, 
	vec3 V, 
	vec3 L, 
	vec3 albedo, 
	vec3 f0, 
	float f90,
	float roughness,
	float metallic)
{
	vec3 nL = normalize(L);
	vec3  H = normalize(V + nL);

	float NdotV = abs  (dot(N,  V));
	float LdotH = clamp(dot(nL, H),  0.0f, 1.0f);
	float NdotH = clamp(dot(N,  H),  0.0f, 1.0f);
	float NdotL = clamp(dot(N,  nL), 0.0f, 1.0f);

	vec3  F   = F_Schlick(f0, f90, LdotH);
	float Vis = V_SmithGGXCorrelated(NdotV, NdotL, roughness);
	float D   = D_GGX(NdotH, roughness);

	// Specular BRDF
	vec3 Fr = D * F * Vis / PI;

	// Diffuse BRDF
	float Fd = Fr_DisneyDiffuse(NdotV, NdotL, LdotH, roughness) / PI * (1.0 - metallic);

	return (albedo * Fd + Fr) * NdotL;
}

void main() {
	// Discard is in alpha mask

	if (mat_hasAlphaMaskMap == 1) {
		if (0.5 > texture(mat_alphaMaskMap, TexCoords).r) {
			discard;
		}
	}

	// Color

	vec4 albedo = mat_albedo;
	if (mat_hasAlbedoMap == 1) {
		albedo = texture(mat_albedoMap, TexCoords);
	}

	albedo.rgb = sRGBToLinear(albedo.rgb);

	// Normal

	vec3 normal = Normal;
	if (mat_hasNormalMap == 1) {
		normal = TBN * (texture(mat_normalMap, TexCoords).xyz * 2 - 1);
	}

	// Metallic

	float metallic = mat_metallic;
	if (mat_hasMetallicMap == 1) {
		metallic = texture(mat_metallicMap, TexCoords).r;
	}

	// Roughness

	float roughness = mat_roughness;
	if (mat_hasRoughnessMap == 1) {
		roughness = texture(mat_roughnessMap, TexCoords).r;
	}

	roughness = roughness*roughness;

	// Reflectance

	float reflectance = mat_reflectance;
	if (mat_hasReflectanceMap == 1) {
		reflectance = texture(mat_reflectanceMap, TexCoords).r;
	}

	// Ambient occlusion

	float ao = mat_ao;
	if (mat_hasAoMap == 1) {
		ao = texture(mat_aoMap, TexCoords).r;
	}

	vec3  f0  = mix(vec3(0.16f * reflectance * reflectance), albedo.xyz, metallic);
	float f90 = clamp(50.0f * dot(f0, vec3(0.33f)), 0.0f, 1.0f);

	vec3 N = normalize(Normal);
	vec3 V = normalize(CameraPos - WorldPos);

	vec3 color = vec3(0);

	for (int i = 0; i < pointLightCount; i++) {
		vec3 P = pointLights[i].Position;
		vec3 L = P - WorldPos;

		float R = pointLights[i].Radius;

		color += BRDF(N, V, L, albedo.xyz, f0, f90, roughness, metallic) 
		       * getDistanceAtt(L, 1 / pow(R, 2))
			   * PointLightShadow(-L, R);
	}
	
	for (int i = 0; i < directionalLightCount; i++) {
		vec3 L = directionalLights[i].InvDirection;

		color += BRDF(N, V, L, albedo.xyz, f0, f90, roughness, metallic)
		       * DirectionalLightShadow(DirectionalLightPos[i]);
	}

	vec3 ambient = max(ambiance - ao, 0) * albedo.rgb;
	color += ambient;

	FragColor = vec4(linearToSRGB(color), albedo.a);
}
