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
using namespace WaveE;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    WaveManager::Init({});
    
    // Create mesh
    ResourceID<WMesh> meshID;
    {
        DefaultVertex vertices[] = {
                { { 0.0f, 0.25f, 0.5f}, {} , { 1.0f, 0.0f, 0.0f, 1.0f } },
                { { 0.25f, -0.25f, 0.5f}, {} , { 0.0f, 1.0f, 0.0f, 1.0f } },
                { { -0.25f, -0.25f, 0.5f}, {} , { 0.0f, 0.0f, 1.0f, 1.0f } }
        };
        WMeshDescriptor meshDesc;
        meshDesc.pVertexData = vertices;
        meshDesc.vertexCount = 3;

        meshID = WResourceManager::Instance()->CreateResource(meshDesc);
    }

    // Create buffer
    ResourceID<WBuffer> bufferID;
    {
        struct SceneConstBuffer
        {
            glm::vec4 offset;
        };
        SceneConstBuffer sceneBuffer{ {0.f,0.f,0.f,0.f} };
        WBufferDescriptor bufferDesc;
        bufferDesc.m_sizeBytes = sizeof(SceneConstBuffer);
        bufferDesc.pInitalData = &sceneBuffer;
        bufferID = WResourceManager::Instance()->CreateResource(bufferDesc);
    }

    // Create pipeline state
    ResourceID<WPipeline> pipelineID;
    {
        WPipelineDescriptor pipelineDesc;
        pipelineDesc.pVertexShader = WResourceManager::Instance()->GetShader("testShaders_VS");
        pipelineDesc.pPixelShader = WResourceManager::Instance()->GetShader("testShaders_PS");
        pipelineDesc.depthStencilState.DepthEnable = false;
        pipelineDesc.depthStencilState.StencilEnable = false;
        pipelineID = WResourceManager::Instance()->CreateResource(pipelineDesc);
    }

    WaveManager::EndInit();

    while (WaveInstance->BeginFrame())
    {
        WaveInstance->SetDefaultRootSigniture();

        WaveInstance->ClearBackBuffer();
        WaveInstance->BindBuffer(bufferID, WaveManager::GLOBAL_CBV_SRV);
        WaveInstance->SetPipelineState(pipelineID);
        WaveInstance->SetRenderTargetToSwapChain();
        WaveInstance->DrawMeshWithCurrentParamaters(meshID);

        WaveInstance->EndFrame();

        //break;
    }

    WaveManager::Uninit();
}
