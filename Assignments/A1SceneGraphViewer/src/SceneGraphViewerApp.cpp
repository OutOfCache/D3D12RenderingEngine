// Help: https://github.com/shuhuai/DeferredShadingD3D12/blob/master/DeferredRender.cpp
#include "SceneGraphViewerApp.hpp"
#include "SceneFactory.hpp"
#include <gimslib/contrib/d3dx12/d3dx12.h>
#include <gimslib/contrib/stb/stb_image.h>
#include <gimslib/d3d/DX12Util.hpp>
#include <gimslib/d3d/UploadHelper.hpp>
#include <gimslib/dbg/HrException.hpp>
#include <gimslib/io/CograBinaryMeshFile.hpp>
#include <gimslib/sys/Event.hpp>
#include <imgui.h>
#include <iostream>
#include <vector>
using namespace gims;

SceneGraphViewerApp::SceneGraphViewerApp(const DX12AppConfig config, const std::filesystem::path pathToScene)
    : DX12App(config)
    , m_examinerController(true)
    , m_scene(SceneGraphFactory::createFromAssImpScene(pathToScene, getDevice(), getCommandQueue()))
{
  m_examinerController.setTranslationVector(f32v3(0, -0.25f, 1.5));
  m_shader = HLSLProgram(L"../../../Assignments/A1SceneGraphViewer/Shaders/TriangleMesh.hlsl", "VS_main", "PS_main");
  createRootSignature();
  createComputeRootSignature();
  createSceneConstantBuffer();
  createPipeline();
  createPipelineBoundingBox();

  {
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors             = 4;
    srvHeapDesc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    throwIfFailed(getDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_descriptorHeap)));

    const auto descSize = getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    for (ui8 index = 0; index < numDeferredRTV; ++index)
    {
      m_offscreenTarget_CPU_UAV[index] =
          CD3DX12_CPU_DESCRIPTOR_HANDLE(m_descriptorHeap->GetCPUDescriptorHandleForHeapStart(), index, descSize);
      m_offscreenTarget_GPU_UAV[index] =
          CD3DX12_GPU_DESCRIPTOR_HANDLE(m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(), index, descSize);
    }
  }

  {
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors             = numDeferredRTV;
    rtvHeapDesc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    throwIfFailed(getDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvDescriptorHeap)));

    const auto descSize = getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    for (ui8 index = 0; index < numDeferredRTV; ++index)
    {
      m_offscreenTarget_CPU_RTV[index] =
          CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), index, descSize);
    }
  }

  createRenderTargetTexture();
}

void SceneGraphViewerApp::onDraw()
{
  if (!ImGui::GetIO().WantCaptureMouse)
  {
    bool pressed  = ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right);
    bool released = ImGui::IsMouseReleased(ImGuiMouseButton_Left) || ImGui::IsMouseReleased(ImGuiMouseButton_Right);
    if (pressed || released)
    {
      bool left = ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseReleased(ImGuiMouseButton_Left);
      m_examinerController.click(pressed, left == true ? 1 : 2,
                                 ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl),
                                 getNormalizedMouseCoordinates());
    }
    else
    {
      m_examinerController.move(getNormalizedMouseCoordinates());
    }
  }

  const auto commandList = getCommandList();
  const auto rtvHandle   = getRTVHandle();
  const auto dsvHandle   = getDSVHandle();

  commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

  const float clearColor[] = {m_uiData.m_backgroundColor.x, m_uiData.m_backgroundColor.y, m_uiData.m_backgroundColor.z,
                              1.0f};
  commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
  commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

  commandList->RSSetViewports(1, &getViewport());
  commandList->RSSetScissorRects(1, &getRectScissor());

  drawScene(commandList);

  // now draw bounding box
  if (m_uiData.m_showBoundingBox)
  {
    drawSceneBoundingBox(commandList);
  }
}

void SceneGraphViewerApp::onDrawUI()
{
  const auto imGuiFlags = m_examinerController.active() ? ImGuiWindowFlags_NoInputs : ImGuiWindowFlags_None;
  ImGui::Begin("Information", nullptr, imGuiFlags);
  ImGui::Text("Frametime: %f", 1.0f / ImGui::GetIO().Framerate * 1000.0f);
  ImGui::End();
  ImGui::Begin("Configuration", nullptr, imGuiFlags);
  ImGui::ColorEdit3("Background Color", &m_uiData.m_backgroundColor[0]);
  ImGui::Checkbox("Show Bounding Boxes", &m_uiData.m_showBoundingBox);
  ImGui::End();
}

void SceneGraphViewerApp::createRootSignature()
{
  CD3DX12_ROOT_PARAMETER parameters[4] {};

  CD3DX12_DESCRIPTOR_RANGE range1 {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0};
  CD3DX12_DESCRIPTOR_RANGE range2 {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1};
  CD3DX12_DESCRIPTOR_RANGE range3 {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2};
  CD3DX12_DESCRIPTOR_RANGE range4 {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3};
  CD3DX12_DESCRIPTOR_RANGE range5 {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4};

  CD3DX12_DESCRIPTOR_RANGE ranges[5] = {range1, range2, range3, range4, range5};

  parameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
  parameters[1].InitAsConstants(16, 1, D3D12_SHADER_VISIBILITY_ALL);
  parameters[2].InitAsConstantBufferView(2, 0, D3D12_SHADER_VISIBILITY_ALL);
  parameters[3].InitAsDescriptorTable(5, ranges);

  D3D12_STATIC_SAMPLER_DESC sampler = {};
  sampler.Filter                    = D3D12_FILTER_MIN_MAG_MIP_POINT;
  sampler.AddressU                  = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
  sampler.AddressV                  = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
  sampler.AddressW                  = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
  sampler.MipLODBias                = 0;
  sampler.MaxAnisotropy             = 0;
  sampler.ComparisonFunc            = D3D12_COMPARISON_FUNC_NEVER;
  sampler.BorderColor               = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
  sampler.MinLOD                    = 0.0f;
  sampler.MaxLOD                    = D3D12_FLOAT32_MAX;
  sampler.ShaderRegister            = 0;
  sampler.RegisterSpace             = 0;
  sampler.ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;

  CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
  descRootSignature.Init(4, parameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

  ComPtr<ID3DBlob> rootBlob, errorBlob;
  D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &rootBlob, &errorBlob);

  getDevice()->CreateRootSignature(0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(),
                                   IID_PPV_ARGS(&m_rootSignature));
}

void SceneGraphViewerApp::createComputeRootSignature()
{
  CD3DX12_ROOT_PARAMETER parameter[2] = {};

  parameter[1].InitAsConstants(3, 0, 0, D3D12_SHADER_VISIBILITY_ALL);
  CD3DX12_DESCRIPTOR_RANGE uavTable;
  uavTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, numDeferredRTV, 0);
  parameter[0].InitAsDescriptorTable(1, &uavTable);

  CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
  descRootSignature.Init(_countof(parameter), parameter, 0, nullptr,
                         D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

  ComPtr<ID3DBlob> rootBlob, errorBlob;
  D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &rootBlob, &errorBlob);

  getDevice()->CreateRootSignature(0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(),
                                   IID_PPV_ARGS(&m_computeRootSignature));
}

void SceneGraphViewerApp::createPipeline()
{
  waitForGPU();
  const auto inputElementDescs = TriangleMeshD3D12::getInputElementDescriptors();

  D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
  psoDesc.InputLayout                        = {inputElementDescs.data(), (ui32)inputElementDescs.size()};
  psoDesc.pRootSignature                     = m_rootSignature.Get();
  psoDesc.VS                                 = CD3DX12_SHADER_BYTECODE(m_shader.getVertexShader().Get());
  psoDesc.PS                                 = CD3DX12_SHADER_BYTECODE(m_shader.getPixelShader().Get());
  psoDesc.RasterizerState                    = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
  psoDesc.RasterizerState.FillMode           = D3D12_FILL_MODE_SOLID;
  psoDesc.RasterizerState.CullMode           = D3D12_CULL_MODE_NONE;
  psoDesc.BlendState                         = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
  psoDesc.DSVFormat                          = getDX12AppConfig().depthBufferFormat;
  psoDesc.DepthStencilState.DepthEnable      = TRUE;
  psoDesc.DepthStencilState.DepthFunc        = D3D12_COMPARISON_FUNC_LESS;
  psoDesc.DepthStencilState.DepthWriteMask   = D3D12_DEPTH_WRITE_MASK_ALL;
  psoDesc.DepthStencilState.StencilEnable    = FALSE;
  psoDesc.SampleMask                         = UINT_MAX;
  psoDesc.PrimitiveTopologyType              = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  psoDesc.NumRenderTargets                   = 1;
  psoDesc.RTVFormats[0]                      = getDX12AppConfig().renderTargetFormat;
  psoDesc.SampleDesc.Count                   = 1;
  throwIfFailed(getDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
}

void SceneGraphViewerApp::drawScene(const ComPtr<ID3D12GraphicsCommandList>& cmdLst)
{
  updateSceneConstantBuffer();

  const auto cb           = m_constantBuffers[getFrameIndex()].getResource()->GetGPUVirtualAddress();
  const auto cameraMatrix = m_examinerController.getTransformationMatrix();

  cmdLst->SetPipelineState(m_pipelineState.Get());

  cmdLst->SetGraphicsRootSignature(m_rootSignature.Get());
  cmdLst->SetGraphicsRootConstantBufferView(0, cb);

  f32m4 modelViewMatrix = cameraMatrix * m_scene.getAABB().getNormalizationTransformation();

  m_scene.addToCommandList(cmdLst, modelViewMatrix, 1, 2, 3);
}

#pragma region Bounding Box

void SceneGraphViewerApp::drawSceneBoundingBox(const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
  updateSceneConstantBuffer();

  const auto cb = m_constantBuffers[getFrameIndex()].getResource()->GetGPUVirtualAddress();

  const auto cameraMatrix = m_examinerController.getTransformationMatrix();

  commandList->SetPipelineState(m_pipelineStateBoundingBox.Get());

  commandList->SetGraphicsRootSignature(m_rootSignature.Get());
  commandList->SetGraphicsRootConstantBufferView(0, cb);

  f32m4 transformation = cameraMatrix * m_scene.getAABB().getNormalizationTransformation();

  m_scene.addToCommandListBoundingBox(commandList, transformation, 1);
}

void SceneGraphViewerApp::createPipelineBoundingBox()
{
  HLSLProgram shaderBoundingBox = HLSLProgram(L"../../../Assignments/A1SceneGraphViewer/Shaders/TriangleMesh.hlsl",
                                              "VS_BoundingBox_main", "PS_BoundingBox_main");

  waitForGPU();
  const auto inputElementDescs = TriangleMeshD3D12::getInputElementDescriptorsBoundingBox();

  D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
  psoDesc.InputLayout                        = {inputElementDescs.data(), (ui32)inputElementDescs.size()};
  psoDesc.pRootSignature                     = m_rootSignature.Get();
  psoDesc.VS                                 = CD3DX12_SHADER_BYTECODE(shaderBoundingBox.getVertexShader().Get());
  psoDesc.PS                                 = CD3DX12_SHADER_BYTECODE(shaderBoundingBox.getPixelShader().Get());
  psoDesc.RasterizerState                    = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
  psoDesc.RasterizerState.FillMode           = D3D12_FILL_MODE_SOLID;
  psoDesc.RasterizerState.CullMode           = D3D12_CULL_MODE_NONE;
  psoDesc.BlendState                         = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
  psoDesc.DSVFormat                          = getDX12AppConfig().depthBufferFormat;
  psoDesc.DepthStencilState.DepthEnable      = TRUE;
  psoDesc.DepthStencilState.DepthFunc        = D3D12_COMPARISON_FUNC_LESS;
  psoDesc.DepthStencilState.DepthWriteMask   = D3D12_DEPTH_WRITE_MASK_ALL;
  psoDesc.DepthStencilState.StencilEnable    = FALSE;
  psoDesc.SampleMask                         = UINT_MAX;
  psoDesc.PrimitiveTopologyType              = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
  psoDesc.NumRenderTargets                   = 1;
  psoDesc.RTVFormats[0]                      = getDX12AppConfig().renderTargetFormat;
  psoDesc.SampleDesc.Count                   = 1;
  throwIfFailed(getDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineStateBoundingBox)));
}

#pragma endregion

namespace PerSceneConstants
{
struct ConstantBuffer
{
  f32m4 projectionMatrix;
};

} // namespace PerSceneConstants

void SceneGraphViewerApp::createSceneConstantBuffer()
{
  const PerSceneConstants::ConstantBuffer cb         = {};
  const auto                              frameCount = getDX12AppConfig().frameCount;
  m_constantBuffers.resize(frameCount);
  for (ui32 i = 0; i < frameCount; i++)
  {
    m_constantBuffers[i] = ConstantBufferD3D12(cb, getDevice());
  }
}

void SceneGraphViewerApp::updateSceneConstantBuffer()
{
  PerSceneConstants::ConstantBuffer cb {};
  cb.projectionMatrix =
      glm::perspectiveFovLH_ZO<f32>(glm::radians(45.0f), (f32)getWidth(), (f32)getHeight(), 1.0f / 256.0f, 256.0f);
  m_constantBuffers[getFrameIndex()].upload(&cb);
}

void SceneGraphViewerApp::createRenderTargetTexture()
{
  D3D12_RESOURCE_DESC tex = {};
  for (auto& rtv : m_offscreenTargets)
    rtv.Reset();

  tex.Dimension        = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  tex.Alignment        = 0;
  tex.Width            = getWidth();
  tex.Height           = getHeight();
  tex.DepthOrArraySize = 1;
  tex.MipLevels        = 1;
  tex.SampleDesc.Count      = 1;
  tex.SampleDesc.Quality    = 0;
  tex.Layout                = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  tex.Flags                 = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
  const auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

  D3D12_CLEAR_VALUE clearValue = {};
  f32v4             clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
  clearValue.Color[0] = clearColor.x;
  clearValue.Color[1] = clearColor.y;
  clearValue.Color[2] = clearColor.z;
  clearValue.Color[3] = clearColor.w;

  for (ui8 index = 0; index < numDeferredRTV; ++index)
  {
    tex.Format        = m_rtvFormat[index];
    clearValue.Format = m_rtvFormat[index];

    throwIfFailed(getDevice()->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &tex,
                                                       D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue,
                                                       IID_PPV_ARGS(&m_offscreenTargets[index])));
  }

  D3D12_UNORDERED_ACCESS_VIEW_DESC unorderAccessViewDesc = {};
  unorderAccessViewDesc.ViewDimension      = D3D12_UAV_DIMENSION_TEXTURE2D;
  unorderAccessViewDesc.Texture2D.MipSlice = 0;

  for (ui8 index = 0; index < numDeferredRTV; ++index)
  {
    unorderAccessViewDesc.Format = m_rtvFormat[index];

    getDevice()->CreateUnorderedAccessView(m_offscreenTargets[index].Get(), nullptr, &unorderAccessViewDesc,
                                           m_offscreenTarget_CPU_UAV[index]);
  }

  D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};

  renderTargetViewDesc.ViewDimension      = D3D12_RTV_DIMENSION_TEXTURE2D;
  renderTargetViewDesc.Texture2D.MipSlice = 0;
  for (ui8 index = 0; index < numDeferredRTV; ++index)
  {
    renderTargetViewDesc.Format = m_rtvFormat[index];

    getDevice()->CreateRenderTargetView(m_offscreenTargets[index].Get(), &renderTargetViewDesc,
                                        m_offscreenTarget_CPU_RTV[index]);
  }
}

void SceneGraphViewerApp::onResize()
{
  createRenderTargetTexture();
}
