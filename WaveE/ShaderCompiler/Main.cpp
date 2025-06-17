#include <iostream>
#include <fstream>
#include <filesystem>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxcapi.h>
#include <wrl.h>

namespace fs = std::filesystem;
using Microsoft::WRL::ComPtr;

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

// Function to compile Ray Tracing shaders
bool CompileRaytracingShader(const std::wstring& sourceFile, const std::wstring& outputFilePath, bool debug)
{
	// Initialize DXC components
	ComPtr<IDxcUtils> utils;
	ComPtr<IDxcCompiler3> compiler;
	ComPtr<IDxcIncludeHandler> includeHandler;

	if (FAILED(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils)))) return false;
	if (FAILED(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)))) return false;
	if (FAILED(utils->CreateDefaultIncludeHandler(&includeHandler))) return false;

	// Load shader source file
	ComPtr<IDxcBlobEncoding> sourceBlob;
	if (FAILED(utils->LoadFile(sourceFile.c_str(), nullptr, &sourceBlob)))
	{
		std::wcerr << L"Failed to load shader file: " << sourceFile << "\n";
		return false;
	}

	// Prepare compilation arguments
	std::vector<LPCWSTR> arguments = {
		sourceFile.c_str(),                // Optional: file name
		L"-T", L"lib_6_3",                 // Shader model: library for ray tracing
		L"-Fo", outputFilePath.c_str(),    // Output object file
		L"-I", fs::path(sourceFile).parent_path().c_str() // Include directory
	};

	if (debug)
	{
		arguments.push_back(L"-Zi");  // Debug info
		arguments.push_back(L"-Od");  // Disable optimizations
		arguments.push_back(L"-Qembed_debug"); // Embed debug info into the .cso
	}

	// Prepare source buffer
	DxcBuffer sourceBuffer = {};
	sourceBuffer.Ptr = sourceBlob->GetBufferPointer();
	sourceBuffer.Size = sourceBlob->GetBufferSize();
	sourceBuffer.Encoding = DXC_CP_UTF8;

	// Compile the shader
	ComPtr<IDxcResult> result;
	HRESULT hr = compiler->Compile(&sourceBuffer, arguments.data(), (UINT)arguments.size(),
		includeHandler.Get(), IID_PPV_ARGS(&result));
	if (FAILED(hr)) return false;

	// Check compilation errors
	ComPtr<IDxcBlobUtf8> errors;
	result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
	if (errors && errors->GetStringLength() > 0)
	{
		std::cerr << "Shader compilation warnings/errors:\n" << errors->GetStringPointer() << '\n';
	}

	HRESULT status;
	result->GetStatus(&status);
	if (FAILED(status))
	{
		std::cerr << "Shader compilation failed.\n";
		return false;
	}

	// Get compiled shader (DXIL blob)
	ComPtr<IDxcBlob> object;
	result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&object), nullptr);

	// Write to file
	std::ofstream outFile(outputFilePath, std::ios::binary);
	if (!outFile)
	{
		std::wcerr << L"Failed to write compiled shader to: " << outputFilePath << "\n";
		return false;
	}
	outFile.write(reinterpret_cast<const char*>(object->GetBufferPointer()), object->GetBufferSize());
	outFile.close();

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

		// Compile ray tracing shader
		CompileRaytracingShader(hlslFile.wstring(), debugOutputPath + L"_RT.cso", true);
		CompileRaytracingShader(hlslFile.wstring(), releaseOutputPath + L"_RT.cso", false);
	}

	return 0;
}
