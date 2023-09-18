

struct VSOUT
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};

VSOUT vs(uint instancID : SV_VertexID)
{
	VSOUT op;

	if (instancID == 0)
	{
		op.pos = float4(0.0, 0.5, 0.5, 1.0);
		op.color = float4(1.0, 0.0, 0.0, 1.0);
	}
	else if (instancID == 1)
	{
		op.pos = float4(0.5, -0.5, 0.5, 1.0);
		op.color = float4(0.0, 1.0, 0.0, 1.0);
	}
	else if (instancID == 2)
	{
		op.pos = float4(-0.5, -0.5, 0.5, 1.0);
		op.color = float4(0.0, 0.0, 1.0, 1.0);
	}

	return op;
;
}

float4 ps(VSOUT input) : SV_TARGET
{
	return input.color;
}