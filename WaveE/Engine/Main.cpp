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

	// Create world matrices
	wma::mat4 cubeWorldMatrix;
	{
		WaveManager::WorldMatrixDescriptor worldMatrixDesc;

		worldMatrixDesc.worldPos = wma::vec3{ 0, 1, 5 };
		worldMatrixDesc.xRotation = 90;

		WaveInstance->CreateWorldMatrix(cubeWorldMatrix, worldMatrixDesc);
	}
	wma::mat4 planeWorldMatrix;
	{
		WaveManager::WorldMatrixDescriptor worldMatrixDesc;

		worldMatrixDesc.scale = 10;

		WaveInstance->CreateWorldMatrix(planeWorldMatrix, worldMatrixDesc);
	}

	// Create textures
	ResourceID<WTexture> waveAlbidoTexture = WResourceManager::Instance()->GetTextureID("WaveE_A_L");
	ResourceID<WTexture> waveNormalTexture = WResourceManager::Instance()->GetTextureID("WaveE_N_L");
	ResourceID<WTexture> tilesAlbidoTexture = WResourceManager::Instance()->GetTextureID("Tiles_A_L");
	ResourceID<WTexture> tilesNormalTexture = WResourceManager::Instance()->GetTextureID("Tiles_N_L.");

	// Create material
	ResourceID<WMaterial> waveMaterial;
	{
		WMaterialDescriptor materialDesc;
		waveMaterial = WResourceManager::Instance()->CreateResource(materialDesc);
		waveMaterial.GetResource()->SwapTexture(waveAlbidoTexture, 0);
		waveMaterial.GetResource()->SwapTexture(waveNormalTexture, 1);
	}
	ResourceID<WMaterial> tilesMaterial;
	{
		WMaterialDescriptor materialDesc;
		tilesMaterial = WResourceManager::Instance()->CreateResource(materialDesc);
		tilesMaterial.GetResource()->SwapTexture(tilesAlbidoTexture, 0);
		tilesMaterial.GetResource()->SwapTexture(tilesNormalTexture, 1);
	}

	// Create per draw buffer
	ResourceID<WBuffer> drawBuffer;
	{
		WBufferDescriptor drawBufferDesc;
		drawBufferDesc.sizeBytes = sizeof(wma::mat4);

		drawBuffer = WResourceManager::Instance()->CreateResource(drawBufferDesc);
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

    while (WaveInstance->BeginFrame())
    {
		cubeWorldMatrix = wma::rotate(cubeWorldMatrix, (float)WaveInstance->GetDeltaTime(), wma::vec3{ 0.f, 0.f, 1.f });

		WaveInstance->SetDefaultRootSigniture();

		WaveInstance->ClearBackBuffer();
		WaveInstance->ClearDepthStencilTarget(WaveInstance->GetDefaultDepthTexture());
		WaveInstance->SetPipelineState(WaveInstance->GetDefaultPipelineState());
		WaveInstance->SetRenderTargetToSwapChain(WaveInstance->GetDefaultDepthTexture());

		WaveInstance->BindBuffer(drawBuffer, WaveManager::DRAW_CBV);

		drawBuffer.GetResource()->UploadData(&cubeWorldMatrix, sizeof(wma::mat4));
		WaveInstance->DrawMesh(cubMeshID, waveMaterial);

		drawBuffer.GetResource()->UploadData(&planeWorldMatrix, sizeof(wma::mat4));
		WaveInstance->DrawMesh(planeMeshID, tilesMaterial);

        WaveInstance->EndFrame();

        //break;
    }

    WaveManager::Uninit();
}
