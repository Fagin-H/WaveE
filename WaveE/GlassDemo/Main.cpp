#include "pch.h"
#include "WaveManager.h"
#include "DX12Helper.h"

#include "WMesh.h"
#include "WResourceManager.h"
#include "WMeshBuilder.h"
#include "WMaterial.h"
#include <algorithm>

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
	ResourceID<WMesh> skyboxMeshID = WResourceManager::Instance()->GetMeshID("skybox");

	// Create world matrices
	wma::mat4 cubeWorldMatrix;
	{
		WaveManager::WorldMatrixDescriptor worldMatrixDesc;

		worldMatrixDesc.worldPos = wma::vec3{ 2, 1, 15 };
		worldMatrixDesc.xRotation = 90;

		WaveInstance->CreateWorldMatrix(cubeWorldMatrix, worldMatrixDesc);
	}
	wma::mat4 planeWorldMatrix;
	{
		WaveManager::WorldMatrixDescriptor worldMatrixDesc;
		worldMatrixDesc.worldPos = wma::vec3{ 0, 0, 10 };
		worldMatrixDesc.scale = 10;

		WaveInstance->CreateWorldMatrix(planeWorldMatrix, worldMatrixDesc);
	}
	wma::mat4 icosphereWorldMatrix;
	{
		WaveManager::WorldMatrixDescriptor worldMatrixDesc;

		worldMatrixDesc.worldPos = wma::vec3{ -2, 1, 15 };

		WaveInstance->CreateWorldMatrix(icosphereWorldMatrix, worldMatrixDesc);
	}
	wma::mat4 glassCubeWorldMatrix;
	{
		WaveManager::WorldMatrixDescriptor worldMatrixDesc;

		worldMatrixDesc.worldPos = wma::vec3{ -2, 1, 10 };

		WaveInstance->CreateWorldMatrix(glassCubeWorldMatrix, worldMatrixDesc);
	}
	wma::mat4 glassIcosphereWorldMatrix;
	{
		WaveManager::WorldMatrixDescriptor worldMatrixDesc;

		worldMatrixDesc.worldPos = wma::vec3{ 2, 1, 10 };

		WaveInstance->CreateWorldMatrix(glassIcosphereWorldMatrix, worldMatrixDesc);
	}

	// Create textures
	ResourceID<WTexture> waveAlbedoTexture = WResourceManager::Instance()->GetTextureID("WaveE_A_L");
	ResourceID<WTexture> waveNormalTexture = WResourceManager::Instance()->GetTextureID("WaveE_N_L");
	ResourceID<WTexture> tilesAlbedoTexture = WResourceManager::Instance()->GetTextureID("Tiles_A_L");
	ResourceID<WTexture> tilesNormalTexture = WResourceManager::Instance()->GetTextureID("Tiles_N_L");	
	ResourceID<WTexture> iceAlbedoTexture = WResourceManager::Instance()->GetTextureID("Ice_A_L");
	ResourceID<WTexture> iceNormalTexture = WResourceManager::Instance()->GetTextureID("Ice_N_L");
	ResourceID<WTexture> skyboxTexture = WResourceManager::Instance()->GetTextureID("Skybox_L");

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
	ResourceID<WPipeline> skyboxPipeline;
	{
		WPipelineDescriptor pipelineDescriptor;
		pipelineDescriptor.pVertexShader = WResourceManager::Instance()->GetShader("skyboxShader_VS");
		pipelineDescriptor.pPixelShader = WResourceManager::Instance()->GetShader("skyboxShader_PS");
		pipelineDescriptor.rasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
		pipelineDescriptor.depthStencilState.DepthEnable = FALSE;
		skyboxPipeline = WResourceManager::Instance()->CreateResource(pipelineDescriptor);
	}

	// Create materials
	ResourceID<WMaterial> waveMaterial;
	{
		WMaterialDescriptor materialDesc;
		materialDesc.pipeline = scenePipeline;
		waveMaterial = WResourceManager::Instance()->CreateResource(materialDesc);
		waveMaterial.GetResource()->SwapTexture(waveAlbedoTexture, 0);
		waveMaterial.GetResource()->SwapTexture(waveNormalTexture, 1);
	}
	ResourceID<WMaterial> tilesMaterial;
	{
		WMaterialDescriptor materialDesc;
		materialDesc.pipeline = scenePipeline;
		tilesMaterial = WResourceManager::Instance()->CreateResource(materialDesc);
		tilesMaterial.GetResource()->SwapTexture(tilesAlbedoTexture, 0);
		tilesMaterial.GetResource()->SwapTexture(tilesNormalTexture, 1);
	}
	ResourceID<WMaterial> iceMaterial;
	{
		WMaterialDescriptor materialDesc;
		materialDesc.pipeline = scenePipeline;
		iceMaterial = WResourceManager::Instance()->CreateResource(materialDesc);
		iceMaterial.GetResource()->SwapTexture(iceAlbedoTexture, 0);
		iceMaterial.GetResource()->SwapTexture(iceNormalTexture, 1);
	}
	ResourceID<WMaterial> skyboxMaterial;
	{
		WMaterialDescriptor materialDesc;
		materialDesc.pipeline = skyboxPipeline;
		skyboxMaterial = WResourceManager::Instance()->CreateResource(materialDesc);
		skyboxMaterial.GetResource()->SwapTexture(skyboxTexture, 0);
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
		wma::scale(wma::mat4::identity(), wma::vec3{0.4f, 0.4f, 0.4f}),
		wma::rotate(wma::scale(wma::mat4::identity(), wma::vec3{1.5f, 1.5f, 1.5f}), wma::radians(90), wma::vec3{1, 0, 0})
	};

	std::vector<ResourceID<WMesh>> onScreenMeshes{ sphereMeshID, cubMeshID, icosphereMeshID };
	std::vector<wma::mat4> onScreenMeshMatrices{ wma::mat4::identity(), glassCubeWorldMatrix, glassIcosphereWorldMatrix };
	std::vector<GlassBuffer> onScreenMeshData{ glassBufferData, glassBufferData, glassBufferData };

	UINT numMeshes = _countof(vAllMeshesForGlass);

	int numPermentGlassObjects = 3;
	float deltaTime;
	float rotSpeed = 1;
	float moveSpeed = 1;
	float multiplicativeSpeed = 0.5f;
	float absorbChangeFactor = 0.2f;
	float refractionIndexChangeFactor = 0.003f;

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
	float distFromCamera = 5;
	float glassScale = 1.f;
	int glassMeshIndex = 2;
	float xRot = 0;
	float yRot = 0;
	bool makeMeshFollowCamera{ true };
	bool showDeubgText{ true };
	wma::vec3 glassPos = camPos + camForwards * distFromCamera;

	wma::mat4 glassRotation{};

	wma::mat4 identityMatrix{};

	wma::mat4 skyboxMatrix;

    while (WaveInstance->BeginFrame())
    {
		// Exit if escape is pressed
		if (WInput::Instance()->WasKeyPressed(VK_ESCAPE))
		{
			break;
		}

		deltaTime = WaveInstance->GetDeltaTime();

		//Input
		{
			if (WInput::Instance()->IsKeyDown(VK_SHIFT))
			{
				if (WInput::Instance()->IsKeyDown(VK_UP))
				{
					xRot += rotSpeed * deltaTime;
				}
				if (WInput::Instance()->IsKeyDown(VK_DOWN))
				{
					xRot -= rotSpeed * deltaTime;
				}
				if (WInput::Instance()->IsKeyDown(VK_RIGHT))
				{
					yRot -= rotSpeed * deltaTime;
				}
				if (WInput::Instance()->IsKeyDown(VK_LEFT))
				{
					yRot += rotSpeed * deltaTime;
				}

				if (WInput::Instance()->IsKeyDown('R'))
				{
					redAbsorb -= absorbChangeFactor * deltaTime;
				}
				if (WInput::Instance()->IsKeyDown('G'))
				{
					greenAbsorb -= absorbChangeFactor * deltaTime;
				}
				if (WInput::Instance()->IsKeyDown('B'))
				{
					blueAbsorb -= absorbChangeFactor * deltaTime;
				}
				if (WInput::Instance()->IsKeyDown('F'))
				{
					absorbfactor -= absorbChangeFactor * deltaTime;
				}

				if (WInput::Instance()->WasKeyPressed(VK_SPACE))
				{
					glassMeshIndex = glassMeshIndex - 1;
					if (glassMeshIndex < 0)
					{
						glassMeshIndex = numMeshes - 1;
					}
				}
			}
			else if (WInput::Instance()->IsKeyDown(VK_CONTROL))
			{
				if (!makeMeshFollowCamera)
				{
					if (WInput::Instance()->IsKeyDown(VK_UP))
					{
						glassPos.y += moveSpeed * deltaTime;
					}
					if (WInput::Instance()->IsKeyDown(VK_DOWN))
					{
						glassPos.y -= moveSpeed * deltaTime;
					}
					if (WInput::Instance()->IsKeyDown(VK_RIGHT))
					{
						glassPos += moveSpeed * camRight * deltaTime;
					}
					if (WInput::Instance()->IsKeyDown(VK_LEFT))
					{
						glassPos -= moveSpeed * camRight * deltaTime;
					}
				}
			}
			else
			{
				if (WInput::Instance()->IsKeyDown(VK_UP) && makeMeshFollowCamera)
				{
					distFromCamera *= 1 + multiplicativeSpeed * deltaTime;
				}
				if (WInput::Instance()->IsKeyDown(VK_DOWN) && makeMeshFollowCamera)
				{
					distFromCamera /= 1 + multiplicativeSpeed * deltaTime;
				}
				if (WInput::Instance()->IsKeyDown(VK_RIGHT))
				{
					glassScale *= 1 + multiplicativeSpeed * deltaTime;
				}
				if (WInput::Instance()->IsKeyDown(VK_LEFT))
				{
					glassScale /= 1 + multiplicativeSpeed * deltaTime;
				}

				if (WInput::Instance()->IsKeyDown('R'))
				{
					redAbsorb += absorbChangeFactor * deltaTime;
				}
				if (WInput::Instance()->IsKeyDown('G'))
				{
					greenAbsorb += absorbChangeFactor * deltaTime;
				}
				if (WInput::Instance()->IsKeyDown('B'))
				{
					blueAbsorb += absorbChangeFactor * deltaTime;
				}
				if (WInput::Instance()->IsKeyDown('F'))
				{
					absorbfactor += absorbChangeFactor * deltaTime;
				}

				if (WInput::Instance()->WasKeyPressed(VK_SPACE))
				{
					glassMeshIndex = (glassMeshIndex + 1) % numMeshes;
				}
			}

			if (WInput::Instance()->WasKeyPressed(VK_RETURN))
			{
				makeMeshFollowCamera = !makeMeshFollowCamera;
			}

			if (WInput::Instance()->WasKeyPressed('H'))
			{
				showDeubgText = !showDeubgText;
			}

			if (WInput::Instance()->WasKeyPressed('I'))
			{
				useInternalReflections = 1 - useInternalReflections;
			}

			if (WInput::Instance()->WasKeyPressed('P'))
			{
				onScreenMeshes.push_back(onScreenMeshes[0]);
				onScreenMeshMatrices.push_back(onScreenMeshMatrices[0]);
				onScreenMeshData.push_back(onScreenMeshData[0]);
			}

			if (WInput::Instance()->WasKeyPressed('O'))
			{
				if (onScreenMeshes.size() > numPermentGlassObjects)
				{
					onScreenMeshes.pop_back();
					onScreenMeshMatrices.pop_back();
					onScreenMeshData.pop_back();
				}
			}

			if (WInput::Instance()->IsKeyDown(VK_OEM_PERIOD))
			{
				refractionIndex += refractionIndexChangeFactor * deltaTime;
			}
			if (WInput::Instance()->IsKeyDown(VK_OEM_COMMA))
			{
				refractionIndex -= refractionIndexChangeFactor * deltaTime;
			}

			{
				if (WInput::Instance()->WasKeyPressed('1'))
				{
					maxItterations = 100;
				}
				if (WInput::Instance()->WasKeyPressed('2'))
				{
					maxItterations = 200;
				}
				if (WInput::Instance()->WasKeyPressed('3'))
				{
					maxItterations = 300;
				}
				if (WInput::Instance()->WasKeyPressed('4'))
				{
					maxItterations = 400;
				}
				if (WInput::Instance()->WasKeyPressed('5'))
				{
					maxItterations = 500;
				}
				if (WInput::Instance()->WasKeyPressed('6'))
				{
					maxItterations = 600;
				}
				if (WInput::Instance()->WasKeyPressed('7'))
				{
					maxItterations = 700;
				}
				if (WInput::Instance()->WasKeyPressed('8'))
				{
					maxItterations = 800;
				}
				if (WInput::Instance()->WasKeyPressed('9'))
				{
					maxItterations = 900;
				}
			}
		}

		// Update
		{
			cubeWorldMatrix = wma::rotate(cubeWorldMatrix, (float)WaveInstance->GetDeltaTime(), wma::vec3{ 0.f, 0.f, 1.f });
			icosphereWorldMatrix = wma::rotate(icosphereWorldMatrix, -(float)WaveInstance->GetDeltaTime(), wma::vec3{ 0.f, 0.f, 1.f });
			onScreenMeshMatrices[1] = wma::rotate(onScreenMeshMatrices[1], (float)WaveInstance->GetDeltaTime(), wma::vec3{ 0.f, 1.f, 0.f });
			onScreenMeshMatrices[2] = wma::rotate(onScreenMeshMatrices[2], (float)WaveInstance->GetDeltaTime(), wma::vec3{ 0.f, 1.f, 0.f });
			skyboxMatrix = wma::translate(identityMatrix, WaveInstance->GetGameCamera().GetPosition());
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
				glassRotation = WaveInstance->GetGameCamera().GetRotation();
			}

			wma::mat4 glassMatrix =
				vMeshMatrices[glassMeshIndex] *
				wma::rotate(identityMatrix, yRot, { 0, 1, 0 }) *
				wma::rotate(identityMatrix, xRot, { 1, 0, 0 }) *
				glassRotation *
				wma::scale(identityMatrix, { glassScale, glassScale, glassScale }) *
				wma::translate(identityMatrix, glassPos);

			ResourceID<WMesh> glassObject = vAllMeshesForGlass[glassMeshIndex];

			onScreenMeshes[0] = glassObject;
			onScreenMeshMatrices[0] = glassMatrix;
		}

		// Draw scene
		{
			WaveInstance->ClearRenderTarget(glassScreenAlbedoTextureCopyFrom);
			WaveInstance->ClearDepthStencilTarget(glassScreenDepthTextureCopyFrom);

			WaveInstance->SetDefaultRootSigniture();
			
			WaveInstance->SetRenderTarget(glassScreenAlbedoTextureCopyFrom, glassScreenDepthTextureCopyFrom);

			WaveInstance->BindBuffer(drawBuffer, WaveManager::DRAW_CBV);
			
			// Skybox
			drawBuffer.GetResource()->UploadData(&skyboxMatrix, sizeof(wma::mat4));
			WaveInstance->DrawMesh(skyboxMeshID, skyboxMaterial);

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

				WaveInstance->ClearRenderTarget(glassFrountNormalTexture, { 0, 0, 0, 1 });
				WaveInstance->ClearRenderTarget(glassBackNormalTexture, { 0, 0, 0, 1 });
				WaveInstance->ClearDepthStencilTarget(glassFrountDepthTexture);
				WaveInstance->ClearDepthStencilTarget(glassBackDepthTexture);

				WaveInstance->SetRenderTarget(glassFrountNormalTexture, glassFrountDepthTexture);

				WaveInstance->SetPipelineState(frountNormalPipeline);
				WaveInstance->DrawMeshWithCurrentParamaters(onScreenMeshes[index]);

				WaveInstance->SetRenderTarget(glassBackNormalTexture, glassBackDepthTexture);

				WaveInstance->SetPipelineState(backNormalPipeline);
				WaveInstance->DrawMeshWithCurrentParamaters(onScreenMeshes[index]);
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

				WaveInstance->DrawMeshWithCurrentParamaters(onScreenMeshes[index]);
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

			WaveECommandList* pCommandList = WaveInstance->GetCommandList();

			// Transition and copy albedo to swap chain
			{				
				D3D12_RESOURCE_BARRIER barrierBeforeCopySource = CreateTransitionBarrier(pGlassAlbedo, glassScreenAlbedoTextureCopyFrom.GetResource()->GetCurrentState(), D3D12_RESOURCE_STATE_COPY_SOURCE);
				pCommandList->ResourceBarrier(1, &barrierBeforeCopySource);

				D3D12_RESOURCE_BARRIER barrierBeforeCopyDest = CreateTransitionBarrier(
					pBackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);
				pCommandList->ResourceBarrier(1, &barrierBeforeCopyDest);

				WaveInstance->CopyTexture(&copyLocationDestination, &copyLocationSource);

				D3D12_RESOURCE_BARRIER barrierAfterCopySource = CreateTransitionBarrier(
					pGlassAlbedo, D3D12_RESOURCE_STATE_COPY_SOURCE, glassScreenAlbedoTextureCopyFrom.GetResource()->GetCurrentState());
				pCommandList->ResourceBarrier(1, &barrierAfterCopySource);

				D3D12_RESOURCE_BARRIER barrierAfterCopyDest = CreateTransitionBarrier(
					pBackBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
				pCommandList->ResourceBarrier(1, &barrierAfterCopyDest);
			}

			copyLocationDestination.pResource = pDefaultDepth;
			copyLocationSource.pResource = pGlassDepth;

			// Transition and copy depth to default depth
			{
				D3D12_RESOURCE_BARRIER barrierBeforeCopySource = CreateTransitionBarrier(
					pGlassDepth, glassScreenDepthTextureCopyFrom.GetResource()->GetCurrentState(), D3D12_RESOURCE_STATE_COPY_SOURCE);
				pCommandList->ResourceBarrier(1, &barrierBeforeCopySource);

				D3D12_RESOURCE_BARRIER barrierBeforeCopyDest = CreateTransitionBarrier(
					pDefaultDepth, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COPY_DEST);
				pCommandList->ResourceBarrier(1, &barrierBeforeCopyDest);

				WaveInstance->CopyTexture(&copyLocationDestination, &copyLocationSource);

				D3D12_RESOURCE_BARRIER barrierAfterCopySource = CreateTransitionBarrier(
					pGlassDepth, D3D12_RESOURCE_STATE_COPY_SOURCE, glassScreenDepthTextureCopyFrom.GetResource()->GetCurrentState());
				pCommandList->ResourceBarrier(1, &barrierAfterCopySource);

				D3D12_RESOURCE_BARRIER barrierAfterCopyDest = CreateTransitionBarrier(
					pDefaultDepth, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_DEPTH_WRITE);
				pCommandList->ResourceBarrier(1, &barrierAfterCopyDest);
			}
		}

        WaveInstance->EndFrame();

        //break;
    }

    WaveManager::Uninit();
}
