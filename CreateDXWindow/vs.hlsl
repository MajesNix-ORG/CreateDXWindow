cbuffer PerFrameCB
{
	float2 screenSize;
	float2 deltaTime;
	float4x4 rotation;
}

struct VSOUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
};

VSOUTPUT main( float4 position : POSITION0, float4 color : COLOR0)
{
	VSOUTPUT OUT= (VSOUTPUT)0;

	OUT.position = mul(position, rotation);
	OUT.position.w = 1.0;
	OUT.position.xy = ((OUT.position.xy / screenSize.xy) * 2) - 1;
	
	OUT.color = color;
	return OUT;
}