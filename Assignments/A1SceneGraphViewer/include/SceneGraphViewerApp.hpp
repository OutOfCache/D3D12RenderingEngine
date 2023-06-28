#pragma once
#include "Scene.hpp"
#include <gimslib/d3d/DX12App.hpp>
#include <gimslib/d3d/HLSLProgram.hpp>
#include <gimslib/types.hpp>
#include <gimslib/ui/ExaminerController.hpp>
using namespace gims;

/// <summary>
/// An app for viewing an Asset Importer Scene Graph.
/// </summary>
class SceneGraphViewerApp : public gims::DX12App
{
public:
  /// <summary>
  /// Creates the SceneGraphViewerApp and loads a scene.
  /// </summary>
  /// <param name="config">Configuration.</param>
  SceneGraphViewerApp(const DX12AppConfig config, const std::filesystem::path pathToScene);

  ~SceneGraphViewerApp() = default;

  /// <summary>
  /// Called whenever a new frame is drawn.
  /// </summary>
  virtual void onDraw();

  /// <summary>
  /// Draw UI onto of rendered result.
  /// </summary>
  virtual void onDrawUI();

private:
  /// <summary>
  /// Root signature connecting shader and GPU resources.
  /// </summary>
  void createRootSignature();
  /// <summary>
  /// Create root signature for deferred compute shader.
  /// </summary>
  void createComputeRootSignature();

  void createComputePipeline();

  /// <summary>
  /// Creates the pipeline
  /// </summary>
  void createPipeline();

  /// <summary>
  /// Draws the scene.
  /// </summary>
  /// <param name="commandList">Command list to which we upload the buffer</param>
  void drawScene(const ComPtr<ID3D12GraphicsCommandList>& commandList);

  /// <summary>
  /// Creates the pipeline for the bounding boxes.
  /// </summary>
  void createPipelineBoundingBox();

  /// <summary>
  /// Draws the bounding boxes of the meshes in the scene.
  /// </summary>
  /// <param name="commandList">Command list to which we upload the buffer</param>
  void drawSceneBoundingBox(const ComPtr<ID3D12GraphicsCommandList>& commandList);

  void createSceneConstantBuffer();

  void updateSceneConstantBuffer();

  void createRenderTargetTexture();

  void onResize();

  void createMeshConstantBuffer();

  void updateMeshConstantBuffer();

  struct UiData
  {
    f32v3 m_backgroundColor = f32v3(0.25f, 0.25f, 0.25f);
    bool  m_showBoundingBox = false;
    
    ui32        finalRTV = 5;
    const char* names[6] = {"Emissive", "Albedo", "Normals (absolute)", "Depth",
                            "Eye Pos (restored)", "Final lighting (restored pos)"};
  };

  ComPtr<ID3D12PipelineState>      m_pipelineState;
  ComPtr<ID3D12PipelineState>      m_deferredPipelineState;
  ComPtr<ID3D12PipelineState>      m_computePipelineState;
  ComPtr<ID3D12PipelineState>      m_pipelineStateBoundingBox;
  ComPtr<ID3D12RootSignature>      m_rootSignature;
  ComPtr<ID3D12RootSignature>      m_computeRootSignature;
  const static int                 numDeferredRTV = 3;
  const static int                 numDeferredUAV = numDeferredRTV + 1 /*depth*/;     
  ComPtr<ID3D12Resource>           m_rtvTexture[numDeferredRTV];
  ComPtr<ID3D12Resource>           m_depthTexture;
  DXGI_FORMAT m_rtvFormat[3] = {/*emissive*/ DXGI_FORMAT_R8G8B8A8_UNORM, /*albedo*/ DXGI_FORMAT_R8G8B8A8_UNORM, /*normals*/ DXGI_FORMAT_R32G32B32A32_FLOAT};

  ComPtr<ID3D12Resource>       m_offscreenTargets[numDeferredRTV];
  ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;
  ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap;

  CD3DX12_CPU_DESCRIPTOR_HANDLE m_offscreenTarget_CPU_UAV[numDeferredUAV];
  CD3DX12_GPU_DESCRIPTOR_HANDLE m_offscreenTarget_GPU_UAV[numDeferredUAV];

  CD3DX12_CPU_DESCRIPTOR_HANDLE m_offscreenTarget_CPU_RTV[numDeferredRTV];
  CD3DX12_GPU_DESCRIPTOR_HANDLE m_offscreenTarget_GPU_RTV[numDeferredRTV];

  std::vector<ConstantBufferD3D12> m_constantBuffers;
  std::vector<ConstantBufferD3D12> m_constantBuffers_Mesh;
  gims::ExaminerController         m_examinerController;
  gims::HLSLProgram                m_shader;
  gims::HLSLProgram                m_deferredShader;
  gims::HLSLShader                 m_computeShader;
  Scene                            m_scene;
  UiData                           m_uiData;
};
