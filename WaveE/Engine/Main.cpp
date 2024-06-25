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

	// Create world matrices
	glm::mat4 cube1WorldMatrix;
	glm::mat4 cube2WorldMatrix;
	glm::mat4 cube3WorldMatrix;
	glm::mat4 cube4WorldMatrix;
	{
		WaveManager::WorldMatrixDescriptor worldMatrixDesc;

		worldMatrixDesc.worldPos = glm::vec3{ 5, 0, 0 };
		WaveInstance->CreateWorldMatrix(cube1WorldMatrix, worldMatrixDesc);

		worldMatrixDesc.worldPos = glm::vec3{ -5, 0, 0 };
		WaveInstance->CreateWorldMatrix(cube2WorldMatrix, worldMatrixDesc);

		worldMatrixDesc.worldPos = glm::vec3{ 0, 0, 5 };
		WaveInstance->CreateWorldMatrix(cube3WorldMatrix, worldMatrixDesc);

		worldMatrixDesc.worldPos = glm::vec3{ 0, 0, -5 };
		WaveInstance->CreateWorldMatrix(cube4WorldMatrix, worldMatrixDesc);
	}

	// Create textures
	ResourceID<WTexture> albidoTexture = WResourceManager::Instance()->GetTextureID("WaveE_A_S");
	ResourceID<WTexture> normalTexture = WResourceManager::Instance()->GetTextureID("WaveE_N_L");

	// Create material
	ResourceID<WMaterial> cubeMaterial;
	{
		WMaterialDescriptor materialDesc;
		cubeMaterial = WResourceManager::Instance()->CreateResource(materialDesc);
		cubeMaterial.GetResource()->SwapTexture(albidoTexture, 0);
		cubeMaterial.GetResource()->SwapTexture(normalTexture, 1);
	}

	// Create per draw buffer
	ResourceID<WBuffer> drawBuffer;
	{
		WBufferDescriptor drawBufferDesc;
		drawBufferDesc.sizeBytes = sizeof(glm::mat4);

		drawBuffer = WResourceManager::Instance()->CreateResource(drawBufferDesc);
	}

	WaveManager::Light light;
	light.colour = glm::vec4{ 1, 1, 1, 1 };
	light.position = glm::vec4{ 0, 0, 0, 0 };

	WaveInstance->SetLight(light, 0);
	WaveInstance->SetAmbientLight(glm::vec4{ 1, 1, 1, 0.1f });


    WaveManager::EndInit();

    while (WaveInstance->BeginFrame())
    {
		WaveInstance->GetGameCamera().Rotate(50 * WaveInstance->GetDeltaTime(), 0);

		WaveInstance->SetDefaultRootSigniture();

		WaveInstance->ClearBackBuffer();
		WaveInstance->ClearDepthStencilTarget(WaveInstance->GetDefaultDepthTexture());
		WaveInstance->SetPipelineState(WaveInstance->GetDefaultPipelineState());
		WaveInstance->SetRenderTargetToSwapChain(WaveInstance->GetDefaultDepthTexture());

		WaveInstance->BindBuffer(drawBuffer, WaveManager::DRAW_CBV);

		drawBuffer.GetResource()->UploadData(&cube1WorldMatrix, sizeof(glm::mat4));
		WaveInstance->DrawMesh(cubMeshID, cubeMaterial);

		drawBuffer.GetResource()->UploadData(&cube2WorldMatrix, sizeof(glm::mat4));
		WaveInstance->DrawMesh(cubMeshID, cubeMaterial);

		drawBuffer.GetResource()->UploadData(&cube3WorldMatrix, sizeof(glm::mat4));
		WaveInstance->DrawMesh(cubMeshID, cubeMaterial);

		drawBuffer.GetResource()->UploadData(&cube4WorldMatrix, sizeof(glm::mat4));
		WaveInstance->DrawMesh(cubMeshID, cubeMaterial);

        WaveInstance->EndFrame();

        //break;
    }

    WaveManager::Uninit();
}
