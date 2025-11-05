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

		VkShaderModule GetShaderModule() const { return m_pShaderModule; };

	private:
		WShaderDescriptor::ShaderType m_type;
		VkShaderModule m_pShaderModule{ nullptr };
	};
}
