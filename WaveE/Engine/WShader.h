#pragma once
namespace WaveE
{
	struct WShaderDescriptor
	{
		enum ShaderType
		{
			Vertex,
			Pixel,
			Compute
		};

		struct ShaderData
		{
			const void* pShaderBytecode;
			size_t bytecodeLength;
		};

		ShaderType type;
		ShaderData shaderData;
	};


	class WShader
	{
	public:
		WShader(const WShaderDescriptor& rDescriptor);
		~WShader();

		D3D12_SHADER_BYTECODE GetShaderBytecode() const;

	private:
		WShaderDescriptor::ShaderType m_type;
		D3D12_SHADER_BYTECODE m_shaderBytecode;
	};
}
