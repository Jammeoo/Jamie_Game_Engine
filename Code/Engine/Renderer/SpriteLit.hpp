#pragma once

const char* g_spriteLitShaderSource = R"(
cbuffer CameraConstants : register(b2)
{
	float4x4 ProjectionMatrix;
	float4x4 ViewMatrix;
};

cbuffer ModelConstants : register(b3)
{
	float4x4 ModelMatrix;
	float4 ModelColor;
};

cbuffer LightingConstants : register(b1)
{
	float3 SunDirection;
	float SunIntensity;
	float AmbientIntensity;
	float3 SunPadding;
};

Texture2D diffuseTexture : register(t0);

SamplerState diffuseSampler : register(s0);

float Interpolate(float start, float end, float fractionTowardEnd)
{
	return start + (end - start) * fractionTowardEnd;
}

float GetFractionWithinRange(float value, float rangeStart, float rangeEnd)
{
	return (value - rangeStart) / (rangeEnd - rangeStart);
}

float RangeMap(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	return Interpolate(outStart, outEnd, GetFractionWithinRange(inValue, inStart, inEnd));
}

struct vs_input_t
{
	float3 localPosition : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
	float3 localNormal : NORMAL;
};

struct v2p_t
{
	float4 position : SV_Position;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
	float4 normal : NORMAL;
};

v2p_t VertexMain(vs_input_t input)
{
	v2p_t v2p;
	v2p.color = input.color;
	v2p.uv = input.uv;
	float4 localPosition = float4(input.localPosition, 1);
	float4 worldPosition = mul(ModelMatrix, localPosition);
	float4 viewPosition = mul(ViewMatrix, worldPosition);
	float4 clipPosition = mul(ProjectionMatrix, viewPosition);
	v2p.position = clipPosition;
	float4 localNormalFour = float4(input.localNormal, 0);
	float4 locNormal = mul(ModelMatrix, localNormalFour);
	v2p.normal = locNormal;
	return v2p;
}
float4 PixelMain(v2p_t input) :SV_Target0
{
	float4 textureColor = diffuseTexture.Sample(diffuseSampler, input.uv);
	float4 vertexColor = input.color;
	float4 modelColor = ModelColor;

	float4 normalizedNormal = normalize(input.normal);
    float4 sunDir = float4(-SunDirection, 0);
	float dotResult = dot(normalizedNormal, sunDir);
	float resultSa = saturate(dotResult);
	float directionLitInten = mul(resultSa, SunIntensity);
	float totalLitInten = directionLitInten + AmbientIntensity;
	float clampedTotalLitInten = saturate(totalLitInten);
	float4 lightColor = float4(clampedTotalLitInten, clampedTotalLitInten, clampedTotalLitInten, 1);
	float4 color = textureColor * vertexColor * lightColor * modelColor;
	clip(color.a - 0.99f);
    return float4(color);
}
)";
