#include "Basic.hlsli"

VSOutput main(float4 pos : POSITION , float2 uv : TEXCODE) {

	VSOutput output; //�s�N�Z���V�F�[�_�[�ɓn���l
	output.svpos = pos;
	output.uv = uv;
	return output;

}