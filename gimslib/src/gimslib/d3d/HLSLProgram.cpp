#include <D3Dcompiler.h>
#include <filesystem>
#include <gimslib/d3d/HLSLProgram.hpp>

#include <iostream>
using namespace gims;

namespace gims
{

HLSLProgram::HLSLProgram()
{
}

HLSLProgram::HLSLProgram(std::filesystem::path pathToShader, char const* const vertexShaderMain,
                         char const* const pixelShaderMain)
    : m_vertexShader(pathToShader, vertexShaderMain, "vs_5_1")
    , m_pixelShader(pathToShader, pixelShaderMain, "ps_5_1")
{
}

const ComPtr<ID3DBlob>& HLSLProgram::getVertexShader() const
{
  return m_vertexShader.getShader();
}

const ComPtr<ID3DBlob>& HLSLProgram::getPixelShader() const
{
  return m_pixelShader.getShader();
}

} // namespace gims