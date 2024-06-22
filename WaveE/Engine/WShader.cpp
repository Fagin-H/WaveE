#include "stdafx.h"
#include "WShader.h"

namespace WaveE
{
	WShader::WShader(const WShaderDescriptor& rDescriptor)
		: m_type{ rDescriptor.type }
	{
		m_shaderBytecode.pShaderBytecode = rDescriptor.shaderData.pShaderBytecode;
		m_shaderBytecode.BytecodeLength = rDescriptor.shaderData.bytecodeLength;
	}

	WShader::~WShader()
	{
	}

	D3D12_SHADER_BYTECODE WShader::GetShaderBytecode() const
	{
		return m_shaderBytecode;
	}
}