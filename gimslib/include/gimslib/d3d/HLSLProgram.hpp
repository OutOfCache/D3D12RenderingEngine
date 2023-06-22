#pragma once
#include <d3d12.h>
#include <filesystem>
#include <gimslib/d3d/HLSLShader.hpp>
#include <wrl.h>
using Microsoft::WRL::ComPtr;
namespace gims
{
class HLSLProgram
{
public:
  HLSLProgram();
  HLSLProgram(std::filesystem::path pathToShader, char const* const vertexShaderMain,
              char const* const pixelShaderMain);
  const ComPtr<ID3DBlob>& getVertexShader() const;
  const ComPtr<ID3DBlob>& getPixelShader() const;

private:
  HLSLShader m_vertexShader;
  HLSLShader m_pixelShader;
};
} // namespace gims