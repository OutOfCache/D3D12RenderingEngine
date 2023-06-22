#pragma once
#include <d3d12.h>
#include <filesystem>
#include <wrl.h>
using Microsoft::WRL::ComPtr;

namespace gims
{
class HLSLShader
{
public:
  
  HLSLShader(std::filesystem::path pathToShader, char const* const shaderMain, char const* const shaderModel);
  HLSLShader()                                               = default;  
  HLSLShader(const HLSLShader& other)                        = default;
  HLSLShader(HLSLShader&& other) noexcept                    = default;
  HLSLShader& operator=(const HLSLShader& other)             = default;
  HLSLShader& operator=(HLSLShader&& other) noexcept         = default;

  const ComPtr<ID3DBlob>& getShader() const;
  ComPtr<ID3DBlob> getShader();

private:
  ComPtr<ID3DBlob> m_shader;
};
} // namespace gims