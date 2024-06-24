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
using namespace WaveE;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    WaveManager::Init({});
    
    // Create meshes
    ResourceID<WMesh> cubeRedMeshID;
    ResourceID<WMesh> cubeGreenMeshID;
    ResourceID<WMesh> cubeBlueMeshID;
    ResourceID<WMesh> cubeWhiteMeshID;
    {
		std::vector<DefaultVertex> vertices;
		std::vector<UINT> indices;

		{
			WMeshBuilder::CreateCubeMesh(vertices, indices, glm::vec3{ 1 }, glm::vec4{ 1, 0, 0, 1 });

			WMeshDescriptor meshDesc;
			meshDesc.pVertexData = vertices.data();
			meshDesc.vertexCount = vertices.size();
			meshDesc.pIndexData = indices.data();
			meshDesc.indexCount = indices.size();

			cubeRedMeshID = WResourceManager::Instance()->CreateResource(meshDesc);
		}
		{
			WMeshBuilder::CreateCubeMesh(vertices, indices, glm::vec3{ 1 }, glm::vec4{ 0, 1, 0, 1 });

			WMeshDescriptor meshDesc;
			meshDesc.pVertexData = vertices.data();
			meshDesc.vertexCount = vertices.size();
			meshDesc.pIndexData = indices.data();
			meshDesc.indexCount = indices.size();

			cubeGreenMeshID = WResourceManager::Instance()->CreateResource(meshDesc);
		}
		{
			WMeshBuilder::CreateCubeMesh(vertices, indices, glm::vec3{ 1 }, glm::vec4{ 0, 0, 1, 1 });

			WMeshDescriptor meshDesc;
			meshDesc.pVertexData = vertices.data();
			meshDesc.vertexCount = vertices.size();
			meshDesc.pIndexData = indices.data();
			meshDesc.indexCount = indices.size();

			cubeBlueMeshID = WResourceManager::Instance()->CreateResource(meshDesc);
		}
		{
			WMeshBuilder::CreateCubeMesh(vertices, indices, glm::vec3{ 1 }, glm::vec4{ 1, 1, 1, 1 });

			WMeshDescriptor meshDesc;
			meshDesc.pVertexData = vertices.data();
			meshDesc.vertexCount = vertices.size();
			meshDesc.pIndexData = indices.data();
			meshDesc.indexCount = indices.size();

			cubeWhiteMeshID = WResourceManager::Instance()->CreateResource(meshDesc);
		}
    }

	// Create world matrices
	glm::mat4 cubeRedWorldMatrix;
	glm::mat4 cubeGreenWorldMatrix;
	glm::mat4 cubeBlueWorldMatrix;
	glm::mat4 cubeWhiteWorldMatrix;
	{
		WaveManager::WorldMatrixDescriptor worldMatrixDesc;
		worldMatrixDesc.worldPos = glm::vec3{ 1, 0, 0 };
		WaveInstance->CreateWorldMatrix(cubeRedWorldMatrix, worldMatrixDesc);

		worldMatrixDesc.worldPos = glm::vec3{ -1, 0, 0 };
		WaveInstance->CreateWorldMatrix(cubeGreenWorldMatrix, worldMatrixDesc);

		worldMatrixDesc.worldPos = glm::vec3{ 0, 0, 1 };
		WaveInstance->CreateWorldMatrix(cubeBlueWorldMatrix, worldMatrixDesc);

		worldMatrixDesc.worldPos = glm::vec3{ 0, 0, -1 };
		WaveInstance->CreateWorldMatrix(cubeWhiteWorldMatrix, worldMatrixDesc);
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
	light.position = glm::vec4{ 0, 2, 0, 0 };

	WaveInstance->SetLight(light, 0);
	WaveInstance->SetAmbientLight(glm::vec4{ 1, 1, 1, 0.1f });

    WaveManager::EndInit();

    while (WaveInstance->BeginFrame())
    {
		WaveInstance->GetGameCamera().Rotate(1, 0);

        WaveInstance->SetDefaultRootSigniture();

        WaveInstance->ClearBackBuffer();
        WaveInstance->SetPipelineState(WaveInstance->GetDefaultPipelineState());
        WaveInstance->SetRenderTargetToSwapChain();

		WaveInstance->BindBuffer(drawBuffer, WaveManager::DRAW_CBV);

		drawBuffer.GetResource()->UploadData(&cubeRedWorldMatrix, sizeof(glm::mat4));
		WaveInstance->DrawMeshWithCurrentParamaters(cubeRedMeshID);

		drawBuffer.GetResource()->UploadData(&cubeGreenWorldMatrix, sizeof(glm::mat4));
		WaveInstance->DrawMeshWithCurrentParamaters(cubeGreenMeshID);

		drawBuffer.GetResource()->UploadData(&cubeBlueWorldMatrix, sizeof(glm::mat4));
		WaveInstance->DrawMeshWithCurrentParamaters(cubeBlueMeshID);

		drawBuffer.GetResource()->UploadData(&cubeWhiteWorldMatrix, sizeof(glm::mat4));
		WaveInstance->DrawMeshWithCurrentParamaters(cubeWhiteMeshID);

        WaveInstance->EndFrame();

        //break;
    }

    WaveManager::Uninit();
}
