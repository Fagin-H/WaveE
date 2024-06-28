#include <iostream>
#include <fstream>
#include <filesystem>
#include <d3d12.h>
#include <d3dcompiler.h>

namespace fs = std::filesystem;

// Function to compile a single HLSL shader
bool CompileShader(const std::wstring& sourceFile, const std::string& entryPoint,
	const std::string& target, const std::wstring& outputFilePath,
	bool debug)
{
	UINT compileFlags = debug ? D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION : 0;
	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	HRESULT hr = D3DCompileFromFile(sourceFile.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entryPoint.c_str(), target.c_str(),
		compileFlags, 0, &shaderBlob, &errorBlob);
	
	if (FAILED(hr))
	{
		if (errorBlob)
		{
			std::string errorMsg = static_cast<char*>(errorBlob->GetBufferPointer());
			if (errorMsg.find("X3501") != std::string::npos)
			{

			}
			else
			{
				std::cerr << "Compilation failed: " << entryPoint << '\n' << errorMsg << '\n';
			}

			errorBlob->Release();
		}
		else
		{
			std::cerr << "Compilation failed: " << entryPoint << '\n';
		}
		if (shaderBlob)
			shaderBlob->Release();

		return false;
	}

	std::ofstream outFile(outputFilePath, std::ios::binary);
	if (!outFile)
	{
		shaderBlob->Release();
		return false;
	}

	outFile.write(static_cast<const char*>(shaderBlob->GetBufferPointer()),
		shaderBlob->GetBufferSize());

	shaderBlob->Release();
	return true;
}

// Function to recursively find HLSL files in a directory
void FindHLSLFiles(const fs::path& directory, std::vector<fs::path>& foundFiles)
{
	for (const auto& entry : fs::directory_iterator(directory))
	{
		if (entry.is_directory())
		{
			FindHLSLFiles(entry.path(), foundFiles);
		}
		else if (entry.path().extension() == ".hlsl")
		{
			foundFiles.push_back(entry.path());
		}
	}
}

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		std::cerr << "Usage: ShaderCompiler <input_directory> <output_directory>\n";
		return 1;
	}

	std::wstring inputDirectory = fs::absolute(argv[1]);
	std::wstring outputDirectory = fs::absolute(argv[2]);

	// Ensure output directories exist
	fs::create_directories(outputDirectory + L"/Debug");
	fs::create_directories(outputDirectory + L"/Release");

	fs::current_path(argv[1]);

	// Find all HLSL files recursively in the input directory
	std::vector<fs::path> hlslFiles;
	FindHLSLFiles(inputDirectory, hlslFiles);

	// Compile each HLSL file
	for (const auto& hlslFile : hlslFiles)
	{
		std::wstring filename = hlslFile.filename().replace_extension(L"");
		std::wstring debugOutputPath = outputDirectory + L"/Debug/" + filename;
		std::wstring releaseOutputPath = outputDirectory + L"/Release/" + filename;

		// Compile pixel shader
		CompileShader(hlslFile.wstring(), "PSMain", "ps_5_0", debugOutputPath + L"_PS.cso", true);
		CompileShader(hlslFile.wstring(), "PSMain", "ps_5_0", releaseOutputPath + L"_PS.cso", false);

		// Compile vertex shader
		CompileShader(hlslFile.wstring(), "VSMain", "vs_5_0", debugOutputPath + L"_VS.cso", true);
		CompileShader(hlslFile.wstring(), "VSMain", "vs_5_0", releaseOutputPath + L"_VS.cso", false);

		// Compile compute shader
		CompileShader(hlslFile.wstring(), "CSMain", "cs_5_0", debugOutputPath + L"_CS.cso", true);
		CompileShader(hlslFile.wstring(), "CSMain", "cs_5_0", releaseOutputPath + L"_CS.cso", false);
	}

	return 0;
}
