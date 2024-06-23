#include "stdafx.h"
#include "WShader.h"

namespace WaveE
{
	WShader::WShader(const WShaderDescriptor& rDescriptor)
		: m_type{ rDescriptor.type }
	{
		m_pBytecodeData = _aligned_malloc(rDescriptor.shaderData.bytecodeLength, 16);
		WAVEE_ASSERT_MESSAGE(m_pBytecodeData, "Failed to alloc byte code data!");
		m_shaderBytecode.pShaderBytecode = m_pBytecodeData;
		m_shaderBytecode.BytecodeLength = rDescriptor.shaderData.bytecodeLength;
		memcpy(m_pBytecodeData, rDescriptor.shaderData.pShaderBytecode, rDescriptor.shaderData.bytecodeLength);
	}

	WShader::~WShader()
	{
		if (m_shaderBytecode.pShaderBytecode)
		{
			_aligned_free(m_pBytecodeData);
			m_pBytecodeData = nullptr;
			m_shaderBytecode.pShaderBytecode = nullptr;
			m_shaderBytecode.BytecodeLength = 0;
		}
	}

	D3D12_SHADER_BYTECODE WShader::GetShaderBytecode() const
	{
		return m_shaderBytecode;
	}
}