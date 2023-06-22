#include <gimslib/d3d/HLSLShader.hpp>
#include <D3Dcompiler.h>
#include <filesystem>
#include <gimslib/dbg/HrException.hpp>
#include <iostream>
using namespace gims;
namespace
{
ComPtr<ID3DBlob> compileShader(std::filesystem::path pathToShader, char const* const shaderMain,
                               char const* const shaderModel)
{
#ifdef _DEBUG
  const auto compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
  const auto compileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
  ComPtr<ID3DBlob> shader;
  ComPtr<ID3DBlob> shaderErrorBlob;
  auto             compileResults = D3DCompileFromFile(pathToShader.c_str(), nullptr, nullptr, shaderMain, shaderModel,
                                                       compileFlags, 0, &shader, &shaderErrorBlob);
  if (FAILED(compileResults))
  {
    if (nullptr != shaderErrorBlob.Get())
    {
      OutputDebugStringA((char*)shaderErrorBlob->GetBufferPointer());
      throw std::exception((char*)shaderErrorBlob->GetBufferPointer());
    }
    throw HrException(compileResults);
  }
  return shader;
}
} // namespace


namespace gims
{
HLSLShader::HLSLShader(std::filesystem::path pathToShader, char const* const shaderMain, char const* const shaderModel)
    : m_shader()
{
  const auto absolutePath = std::filesystem::weakly_canonical(pathToShader);
  if (!std::filesystem::exists(absolutePath))
  {
    throw std::exception((absolutePath.string() + std::string(" does not exist.")).c_str());
  }
  m_shader = ::compileShader(absolutePath, shaderMain, shaderModel);
}

const ComPtr<ID3DBlob>& HLSLShader::getShader() const
{
  return m_shader;
}

ComPtr<ID3DBlob> HLSLShader::getShader()
{
  return m_shader;
}
} // namespace gims