//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "stdafx.h"
#include "WaveManager.h"

#include "WMesh.h"
#include "WResourceManager.h"
#include "WMeshBuilder.h"
#include "WMaterial.h"

using namespace WaveE;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    WaveManager::Init({});
    
    // Create meshes
	ResourceID<WMesh> cubMeshID = WResourceManager::Instance()->GetMeshID("cube");
	ResourceID<WMesh> planeMeshID = WResourceManager::Instance()->GetMeshID("plane");
	ResourceID<WMesh> icosphereMeshID = WResourceManager::Instance()->GetMeshID("icosphere");
	ResourceID<WMesh> sphereMeshID = WResourceManager::Instance()->GetMeshID("sphere");
	ResourceID<WMesh> concaveLensMeshID = WResourceManager::Instance()->GetMeshID("concaveLens");
	ResourceID<WMesh> convexLenseMeshID = WResourceManager::Instance()->GetMeshID("convexLense");
	ResourceID<WMesh> glassMesh;

	// Create world matrices
	wma::mat4 cubeWorldMatrix;
	{
		WaveManager::WorldMatrixDescriptor worldMatrixDesc;

		worldMatrixDesc.worldPos = wma::vec3{ 2, 1, 10 };
		worldMatrixDesc.xRotation = 90;

		WaveInstance->CreateWorldMatrix(cubeWorldMatrix, worldMatrixDesc);
	}
	wma::mat4 planeWorldMatrix;
	{
		WaveManager::WorldMatrixDescriptor worldMatrixDesc;
		worldMatrixDesc.worldPos = wma::vec3{ 0, 0, 5 };
		worldMatrixDesc.scale = 10;

		WaveInstance->CreateWorldMatrix(planeWorldMatrix, worldMatrixDesc);
	}
	wma::mat4 icosphereWorldMatrix;
	{
		WaveManager::WorldMatrixDescriptor worldMatrixDesc;

		worldMatrixDesc.worldPos = wma::vec3{ -2, 1, 10 };

		WaveInstance->CreateWorldMatrix(icosphereWorldMatrix, worldMatrixDesc);
	}

	// Create textures
	ResourceID<WTexture> waveAlbedoTexture = WResourceManager::Instance()->GetTextureID("WaveE_A_L");
	ResourceID<WTexture> waveNormalTexture = WResourceManager::Instance()->GetTextureID("WaveE_N_L");
	ResourceID<WTexture> tilesAlbedoTexture = WResourceManager::Instance()->GetTextureID("Tiles_A_L");
	ResourceID<WTexture> tilesNormalTexture = WResourceManager::Instance()->GetTextureID("Tiles_N_L");	
	ResourceID<WTexture> iceAlbedoTexture = WResourceManager::Instance()->GetTextureID("Ice_A_L");
	ResourceID<WTexture> iceNormalTexture = WResourceManager::Instance()->GetTextureID("Ice_N_L");

	// Glass shader textures
	ResourceID<WTexture> glassScreenAlbedoTexture;
	ResourceID<WTexture> glassScreenDepthTexture;
	ResourceID<WTexture> glassBackDepthTexture;
	ResourceID<WTexture> glassFrountDepthTexture;
	ResourceID<WTexture> glassBackNormalTexture;
	ResourceID<WTexture> glassFrountNormalTexture;
	WDescriptorHeapManager::Allocation glassShaderAllocation = WaveManager::Instance()->GetCBV_SRV_UAVHeap()->Allocate(7);
	{
		WTextureDescriptor textureDesc;
		textureDesc.format = WTextureDescriptor::RGBA;
		textureDesc.usage = WTextureDescriptor::ShaderResource;
		textureDesc.startAsShaderResource = true;
		textureDesc.width = WaveInstance->GetWidth();
		textureDesc.height = WaveInstance->GetHeight();
		glassScreenAlbedoTexture = WResourceManager::Instance()->CreateResource(textureDesc, glassShaderAllocation, 1);
		textureDesc.format = WTextureDescriptor::DepthTypeless;
		glassScreenDepthTexture = WResourceManager::Instance()->CreateResource(textureDesc, glassShaderAllocation, 2);
		textureDesc.usage = WTextureDescriptor::ResourceAndTarget;
		textureDesc.startAsShaderResource = false;
		glassBackDepthTexture = WResourceManager::Instance()->CreateResource(textureDesc, glassShaderAllocation, 3);
		glassFrountDepthTexture = WResourceManager::Instance()->CreateResource(textureDesc, glassShaderAllocation, 4);
		textureDesc.format = WTextureDescriptor::RGBAF16;
		glassBackNormalTexture = WResourceManager::Instance()->CreateResource(textureDesc, glassShaderAllocation, 5);
		glassFrountNormalTexture = WResourceManager::Instance()->CreateResource(textureDesc, glassShaderAllocation, 6);
	}
	ResourceID<WTexture> glassScreenAlbedoTextureCopyFrom;
	ResourceID<WTexture> glassScreenDepthTextureCopyFrom;
	{
		WTextureDescriptor textureDesc;
		textureDesc.format = WTextureDescriptor::RGBA;
		textureDesc.usage = WTextureDescriptor::RenderTarget;
		textureDesc.startAsShaderResource = false;
		textureDesc.width = WaveInstance->GetWidth();
		textureDesc.height = WaveInstance->GetHeight();
		glassScreenAlbedoTextureCopyFrom = WResourceManager::Instance()->CreateResource(textureDesc);
		textureDesc.format = WTextureDescriptor::DepthTypeless;
		glassScreenDepthTextureCopyFrom = WResourceManager::Instance()->CreateResource(textureDesc);
	}

	// Create materials
	ResourceID<WMaterial> waveMaterial;
	{
		WMaterialDescriptor materialDesc;
		waveMaterial = WResourceManager::Instance()->CreateResource(materialDesc);
		waveMaterial.GetResource()->SwapTexture(waveAlbedoTexture, 0);
		waveMaterial.GetResource()->SwapTexture(waveNormalTexture, 1);
	}
	ResourceID<WMaterial> tilesMaterial;
	{
		WMaterialDescriptor materialDesc;
		tilesMaterial = WResourceManager::Instance()->CreateResource(materialDesc);
		tilesMaterial.GetResource()->SwapTexture(tilesAlbedoTexture, 0);
		tilesMaterial.GetResource()->SwapTexture(tilesNormalTexture, 1);
	}
	ResourceID<WMaterial> iceMaterial;
	{
		WMaterialDescriptor materialDesc;
		iceMaterial = WResourceManager::Instance()->CreateResource(materialDesc);
		iceMaterial.GetResource()->SwapTexture(iceAlbedoTexture, 0);
		iceMaterial.GetResource()->SwapTexture(iceNormalTexture, 1);
	}

	// Create pipeline states
	ResourceID<WPipeline> frountNormalPipeline;
	ResourceID<WPipeline> backNormalPipeline;
	{
		WPipelineDescriptor pipelineDescriptor;
		pipelineDescriptor.pVertexShader = WResourceManager::Instance()->GetShader("glassNormalShader_VS");
		pipelineDescriptor.pPixelShader = WResourceManager::Instance()->GetShader("glassNormalShader_PS");
		pipelineDescriptor.rtvFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
		pipelineDescriptor.dsvFormat = DXGI_FORMAT_D32_FLOAT;
		frountNormalPipeline = WResourceManager::Instance()->CreateResource(pipelineDescriptor);
		pipelineDescriptor.rasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
		backNormalPipeline = WResourceManager::Instance()->CreateResource(pipelineDescriptor);
	}
	ResourceID<WPipeline> glassPipeline;
	{
		WPipelineDescriptor pipelineDescriptor;
		pipelineDescriptor.pVertexShader = WResourceManager::Instance()->GetShader("glassShader_VS");
		pipelineDescriptor.pPixelShader = WResourceManager::Instance()->GetShader("glassShader_PS");
		pipelineDescriptor.dsvFormat = DXGI_FORMAT_D32_FLOAT;
		glassPipeline = WResourceManager::Instance()->CreateResource(pipelineDescriptor);
	}
	ResourceID<WPipeline> scenePipeline;
	{
		WPipelineDescriptor pipelineDescriptor;
		pipelineDescriptor.pVertexShader = WResourceManager::Instance()->GetShader("SimpleLighting_VS");
		pipelineDescriptor.pPixelShader = WResourceManager::Instance()->GetShader("SimpleLighting_PS");
		pipelineDescriptor.dsvFormat = DXGI_FORMAT_D32_FLOAT;
		scenePipeline = WResourceManager::Instance()->CreateResource(pipelineDescriptor);
	}

	// Buffers
	// Create per draw buffer
	ResourceID<WBuffer> drawBuffer;
	{
		WBufferDescriptor drawBufferDesc;
		drawBufferDesc.sizeBytes = sizeof(wma::mat4);

		drawBuffer = WResourceManager::Instance()->CreateResource(drawBufferDesc);
	}
	// Glass buffer

	int screenWidth = static_cast<int>(WaveInstance->GetWidth());
	int screenHeight = static_cast<float>(WaveInstance->GetHeight());

	struct GlassBuffer
	{
		wma::vec4 colourAbsorption;
		wma::vec4 useInternalReflections;
		wma::vec2 screenRes;
		float refractionIndex;
		float maxItterations;
	};
	GlassBuffer glassBufferData;
	glassBufferData.colourAbsorption = { 1, 1, 1, 0 };
	glassBufferData.useInternalReflections = { 1, 0, 0, 0 };
	glassBufferData.screenRes = { static_cast<float>(screenWidth), static_cast<float>(screenHeight) };
	glassBufferData.refractionIndex = 1.5f;
	glassBufferData.maxItterations = 500;

	ResourceID<WBuffer> glassBuffer;
	{
		WBufferDescriptor bufferDesc;
		bufferDesc.sizeBytes = sizeof(GlassBuffer);
		bufferDesc.pInitalData = &glassBufferData;

		glassBuffer = WResourceManager::Instance()->CreateResource(bufferDesc, glassShaderAllocation, 0);
	}

	WaveManager::Light light1;
	WaveManager::Light light2;
	light1.colour = wma::vec4{ 1, 1, 1, 1 };
	light1.position = wma::vec4{ 0, 0, 0, 0 };

	light2.colour = wma::vec4{ 0.9f, 0.8f, 0.6f, 1 };
	light2.position = wma::vec4{ 0, 10, 0, 0 };

	WaveInstance->SetLight(light1, 0);
	WaveInstance->SetLight(light2, 1);


	WaveInstance->SetAmbientLight(wma::vec4{ 1, 1, 1, 0.1f });


    WaveManager::EndInit();

	// Glass setup
	ResourceID<WMesh> vAllMeshesForGlass[] = { 
		cubMeshID,
		icosphereMeshID,
		sphereMeshID,
		concaveLensMeshID,
		convexLenseMeshID };

	wma::mat4 vMeshMatrices[] = {
		wma::mat4::identity(),
		wma::mat4::identity(),
		wma::mat4::identity(),
		wma::scale(wma::mat4::identity(), wma::vec3{0.2f, 0.2f, 0.2f}),
		wma::rotate(wma::scale(wma::mat4::identity(), wma::vec3{0.2f, 0.2f, 0.2f}), 90, wma::vec3{1, 0, 0})
	};

	std::vector<ResourceID<WMesh>> onScreenMeshes{ sphereMeshID };
	std::vector<wma::mat4> onScreenMeshMatrices{ wma::mat4::identity() };
	std::vector<GlassBuffer> onScreenMeshData{ glassBufferData };

	UINT numMeshes = _countof(vAllMeshesForGlass);
	UINT glassMeshIndex = 3;

	float maxItterations = 500;
	float redAbsorb = 1;
	float greenAbsorb = 0.5f;
	float blueAbsorb = 0.1f;
	float absorbfactor = 0.1f;
	float refractionIndex = 1.5f;
	int useInternalReflections = 1;

	wma::vec3 camPos = WaveInstance->GetGameCamera().GetPosition();
	wma::vec3 camForwards = WaveInstance->GetGameCamera().GetForwards();
	wma::vec3 camRight = WaveInstance->GetGameCamera().GetRight();
	wma::vec3 camUp = WaveInstance->GetGameCamera().GetUp();
	float distFromCamera = 2;
	float glassScale = 1.f;
	int meshIndex = 0;
	float xRot = 0;
	float yRot = 0;
	bool makeMeshFollowCamera{ true };
	bool showDeubgText{ true };
	wma::vec3 glassPos = camPos + camForwards * distFromCamera;

	wma::mat4 glassRotation;

	wma::mat4 identityMatrix{};

    while (WaveInstance->BeginFrame())
    {
		// Exit if escape is pressed
		if (WInput::Instance()->WasKeyPressed(VK_ESCAPE))
		{
			break;
		}

		//Input
		{
			if (WInput::Instance()->WasKeyPressed(VK_SPACE))
			{
				glassMeshIndex++;
				glassMeshIndex %= numMeshes;
			}

			glassMesh = vAllMeshesForGlass[glassMeshIndex];
		}

		// Update
		{
			cubeWorldMatrix = wma::rotate(cubeWorldMatrix, (float)WaveInstance->GetDeltaTime(), wma::vec3{ 0.f, 0.f, 1.f });
			icosphereWorldMatrix = wma::rotate(icosphereWorldMatrix, -(float)WaveInstance->GetDeltaTime(), wma::vec3{ 0.f, 0.f, 1.f });
		}

		// Update glass const buffer
		{
			onScreenMeshData[0].screenRes.x = screenWidth;
			onScreenMeshData[0].screenRes.y = screenHeight;
			onScreenMeshData[0].refractionIndex = refractionIndex;
			onScreenMeshData[0].useInternalReflections.x = useInternalReflections;
			// First 3 floats are the amount of each RGB value the glass absorbs as the ray travels. The 4th float is how strongly it absorbs the light
			// E.g., a high red and green value will absorb the red and green light making it more blue
			onScreenMeshData[0].colourAbsorption = { redAbsorb, greenAbsorb, blueAbsorb, absorbfactor };
			onScreenMeshData[0].maxItterations = { maxItterations };
		}

		// Glass mesh variables
		{
			camPos = WaveInstance->GetGameCamera().GetPosition();
			camForwards = WaveInstance->GetGameCamera().GetForwards();
			camRight = WaveInstance->GetGameCamera().GetRight();
			camUp = WaveInstance->GetGameCamera().GetUp();

			if (makeMeshFollowCamera)
			{
				glassPos = camPos + camForwards * distFromCamera;
				glassRotation = wma::mat4{ wma::vec4{camRight, 0}, wma::vec4{camUp, 0} , wma::vec4{camForwards, 0}, wma::vec4{0,0,0,1} };
			}

			wma::mat4 glassMatrix = wma::translate(identityMatrix, glassPos) * wma::scale(identityMatrix, { glassScale, glassScale, glassScale }) * glassRotation * wma::rotate(identityMatrix, xRot, {1, 0, 0}) * wma::rotate(identityMatrix, yRot, { 0, 1, 0 }) * vMeshMatrices[meshIndex];

			ResourceID<WMesh> glassObject = vAllMeshesForGlass[meshIndex];

			onScreenMeshes[0] = glassObject;
			onScreenMeshMatrices[0] = glassMatrix;
		}

		// Clear textures
		{
			WaveInstance->ClearRenderTarget(glassScreenAlbedoTextureCopyFrom, { 0.5f, 0.8f, 0.9f });
			WaveInstance->ClearRenderTarget(glassFrountNormalTexture, { 0, 0, 0, 0 });
			WaveInstance->ClearRenderTarget(glassBackNormalTexture, { 0, 0, 0, 0 });

			WaveInstance->ClearDepthStencilTarget(glassScreenDepthTextureCopyFrom);
			WaveInstance->ClearDepthStencilTarget(glassFrountDepthTexture);
			WaveInstance->ClearDepthStencilTarget(glassBackDepthTexture);
		}

		// Draw scene
		{
			WaveInstance->SetDefaultRootSigniture();
			
			WaveInstance->SetRenderTarget(glassScreenAlbedoTextureCopyFrom, glassScreenDepthTextureCopyFrom);

			WaveInstance->SetPipelineState(scenePipeline);

			WaveInstance->BindBuffer(drawBuffer, WaveManager::DRAW_CBV);

			drawBuffer.GetResource()->UploadData(&cubeWorldMatrix, sizeof(wma::mat4));
			WaveInstance->DrawMesh(cubMeshID, waveMaterial);

			drawBuffer.GetResource()->UploadData(&icosphereWorldMatrix, sizeof(wma::mat4));
			WaveInstance->DrawMesh(icosphereMeshID, iceMaterial);

			drawBuffer.GetResource()->UploadData(&planeWorldMatrix, sizeof(wma::mat4));
			WaveInstance->DrawMesh(planeMeshID, tilesMaterial);
		}

		// Sort by dist to camera
		std::vector<int> objectIndexes{};
		for (int i = 0; i < onScreenMeshes.size(); i++)
		{
			objectIndexes.push_back(i);
		}

		auto sortFunc = [&](const int& a, const int& b)
			{
				float distToCameraA = dot(onScreenMeshMatrices[a][3].xyz - camPos, camForwards);
				float distToCameraB = dot(onScreenMeshMatrices[b][3].xyz - camPos, camForwards);
				return distToCameraA > distToCameraB;
			};

		std::sort(objectIndexes.begin(), objectIndexes.end(), sortFunc);

		for (int i = 0; i < objectIndexes.size(); i++)
		{
			int index = objectIndexes[i];
			// Copy render and dept texture
			{
				WaveInstance->CopyTexture(glassScreenAlbedoTexture, glassScreenAlbedoTextureCopyFrom);
				WaveInstance->CopyTexture(glassScreenDepthTexture, glassScreenDepthTextureCopyFrom);
			}

			// Update buffers
			{
				glassBufferData.screenRes.x = screenWidth;
				glassBufferData.screenRes.y = screenHeight;
				glassBufferData.refractionIndex = onScreenMeshData[index].refractionIndex;
				glassBufferData.useInternalReflections.x = useInternalReflections;
				// First 3 floats are the amount of each RGB value the glass absorbs as the ray travels. The 4th float is how strongly it absorbs the light
				// E.g., a high red and green value will absorb the red and green light making it more blue
				glassBufferData.colourAbsorption = onScreenMeshData[index].colourAbsorption;
				glassBufferData.maxItterations = { maxItterations };

				glassBuffer.GetResource()->UploadData(&glassBufferData, sizeof(GlassBuffer));
				drawBuffer.GetResource()->UploadData(&onScreenMeshMatrices[index], sizeof(wma::mat4));
			}

			// Normals
			{
				glassFrountNormalTexture.GetResource()->SetState(WTexture::Output);
				glassBackNormalTexture.GetResource()->SetState(WTexture::Output);
				glassFrountDepthTexture.GetResource()->SetState(WTexture::Output);
				glassBackDepthTexture.GetResource()->SetState(WTexture::Output);

				WaveInstance->SetRenderTarget(glassFrountNormalTexture, glassFrountDepthTexture);

				WaveInstance->SetPipelineState(frountNormalPipeline);
				WaveInstance->DrawMeshWithCurrentParamaters(glassMesh);

				WaveInstance->SetRenderTarget(glassBackNormalTexture, glassBackDepthTexture);

				WaveInstance->SetPipelineState(backNormalPipeline);
				WaveInstance->DrawMeshWithCurrentParamaters(glassMesh);
			}

			// Change states
			{
				glassFrountNormalTexture.GetResource()->SetState(WTexture::Input);
				glassBackNormalTexture.GetResource()->SetState(WTexture::Input);
				glassFrountDepthTexture.GetResource()->SetState(WTexture::Input);
				glassBackDepthTexture.GetResource()->SetState(WTexture::Input);
			}

			// Glass rendering
			{
				WaveInstance->SetPipelineState(glassPipeline);

				WaveInstance->SetRenderTarget(glassScreenAlbedoTextureCopyFrom, glassScreenDepthTextureCopyFrom);

				// Bind buffer and textures in 1 go
				WaveInstance->BindResource(glassShaderAllocation, WaveManager::GLOBAL_CBV_SRV);

				WaveInstance->DrawMeshWithCurrentParamaters(glassMesh);
			}
		}

		// Copy scene to swap chain
		{
			WaveInstance->SetRenderTargetToSwapChain(WaveInstance->GetDefaultDepthTexture());

			ID3D12Resource* pBackBuffer = WaveInstance->GetCurrentBackBuffer();
			ID3D12Resource* pDefaultDepth = WaveInstance->GetDefaultDepthTexture().GetResource()->GetTexture();
			ID3D12Resource* pGlassAlbedo = glassScreenAlbedoTextureCopyFrom.GetResource()->GetTexture();
			ID3D12Resource* pGlassDepth = glassScreenDepthTextureCopyFrom.GetResource()->GetTexture();

			D3D12_TEXTURE_COPY_LOCATION copyLocationDestination;
			D3D12_TEXTURE_COPY_LOCATION copyLocationSource;

			copyLocationDestination.pResource = pBackBuffer;
			copyLocationDestination.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			copyLocationDestination.SubresourceIndex = 0;

			copyLocationSource.pResource = pGlassAlbedo;
			copyLocationSource.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			copyLocationSource.SubresourceIndex = 0;

			WaveInstance->CopyTexture(&copyLocationDestination, &copyLocationSource);

			copyLocationDestination.pResource = pDefaultDepth;
			copyLocationSource.pResource = pGlassDepth;

			WaveInstance->CopyTexture(&copyLocationDestination, &copyLocationSource);
		}

        WaveInstance->EndFrame();

        //break;
    }

    WaveManager::Uninit();
}
