struct VertexShaderOutput
{
  float4 clipSpacePosition : SV_POSITION;
  float3 viewSpacePosition : POSITION;
  float3 viewSpaceNormal : NORMAL;
  float2 texCoord : TEXCOOD;
};

/// <summary>
/// Constants that can change every frame.
/// </summary>
cbuffer PerFrameConstants : register(b0)
{
  float4x4 projectionMatrix;
}

/// <summary>
/// Constants that can change per Mesh/Drawcall
/// </summary>
cbuffer PerMeshConstants : register(b1)
{
  float4x4 modelViewMatrix;
}

/// <summary>
/// Consttants that are really constant for the entire scene.
/// </summary>
cbuffer Material : register(b2)
{
  float4 ambientColor;
  float4 diffuseColor;
  float4 specularColorAndExponent;
}

Texture2D<float3> g_textureAmbient : register(t0);
Texture2D<float4> g_textureDiffuse : register(t1);
Texture2D<float3> g_textureSpecular : register(t2);
Texture2D<float3> g_textureEmissive : register(t3);
Texture2D<float3> g_textureNormal : register(t4);

SamplerState g_sampler : register(s0);

VertexShaderOutput VS_main(float3 position : POSITION, float3 normal : NORMAL, float2 texCoord : TEXCOORD)
{
  VertexShaderOutput output;


  float4 p4                = mul(modelViewMatrix, float4(position, 1.0f));
  output.viewSpacePosition = p4.xyz;
  output.viewSpaceNormal   = mul(modelViewMatrix, float4(normal, 0.0f)).xyz;
  output.clipSpacePosition = mul(projectionMatrix, p4);
  output.texCoord          = texCoord;

  return output;
}

float4 PS_main(VertexShaderOutput input)
    : SV_TARGET
{
  float3 l = normalize(float3(-1.0f, 0.0f, 0.0f));
  float3 n = normalize(input.viewSpaceNormal);

  float3 v = normalize(-input.viewSpacePosition);
  float3 h = normalize(l + v);

  float f_diffuse = max(0.0f, dot(n, l));
  float f_specular = pow(max(0.0f, dot(n, h)), specularColorAndExponent.w);

  float3 textureColorAmbient = g_textureAmbient.Sample(g_sampler, input.texCoord, 0);
  float4 textureColorDiffuse = g_textureDiffuse.Sample(g_sampler, input.texCoord, 0);
  float3 textureColorSpecular = g_textureSpecular.Sample(g_sampler, input.texCoord, 0);
  float3 textureColorEmissive = g_textureEmissive.Sample(g_sampler, input.texCoord, 0);
  float3 textureColorHeight = g_textureNormal.Sample(g_sampler, input.texCoord, 0);

  if (textureColorDiffuse.w == 0)
  {
    discard;
  }

  return float4(ambientColor.xyz * textureColorAmbient.xyz + textureColorEmissive.xyz +
                    textureColorDiffuse.xyz * diffuseColor.xyz +
                    f_specular * textureColorSpecular * specularColorAndExponent.xyz,
                1);
}

struct DeferredOutput
{
    float4 emissive : SV_TARGET0;
    float4 albedo   : SV_TARGET1;
    float4 position : SV_TARGET2;
    float4 normal   : SV_TARGET3;
};

DeferredOutput PS_deferred(VertexShaderOutput input)
    : SV_TARGET
{
  DeferredOutput output;

  float3 l = normalize(float3(-1.0f, 0.0f, 0.0f));
  float3 n = normalize(input.viewSpaceNormal);

  float3 v = normalize(-input.viewSpacePosition);
  float3 h = normalize(l + v);

  float f_specular = pow(max(0.0f, dot(n, h)), specularColorAndExponent.w);

  float3 textureColorAmbient  = g_textureAmbient.Sample(g_sampler, input.texCoord, 0);
  float4 textureColorDiffuse  = g_textureDiffuse.Sample(g_sampler, input.texCoord, 0);
  float3 textureColorSpecular = g_textureSpecular.Sample(g_sampler, input.texCoord, 0);
  float3 textureColorEmissive = g_textureEmissive.Sample(g_sampler, input.texCoord, 0);

  if (textureColorDiffuse.w == 0)
  {
    discard;
  }

    output.position = float4(input.viewSpacePosition, 1);
    output.normal   = float4(input.viewSpaceNormal, 0);
    output.albedo   = float4(textureColorDiffuse.xyz * diffuseColor.xyz, 1);
    output.emissive = float4(textureColorEmissive, 1);
    
    return output;
}

RWTexture2D<float4> output : register(u0);
RWTexture2D<float4> albedo : register(u1);
RWTexture2D<float4> positions : register(u2);
RWTexture2D<float4> normals : register(u3);
RWTexture2D<float>  depth : register(u4);

struct RootConstants
{
    float4 backgroundColor;
    int width;
    int height;
    int finalRTV;
};
ConstantBuffer<RootConstants> rootConstants : register(b0);

[numthreads(16, 16, 1)] void CS_lighting(int3 tid
                                         : SV_DispatchThreadID)
{
  float4 inputColors[5] = {output[tid.xy], albedo[tid.xy], positions[tid.xy] + float4(0.5f, 0.5f, 0, 0),
                           abs(normalize(normals[tid.xy])), float4(0, 0, (depth[tid.xy] - 0.99f) * 100, 1)};

  if (tid.x < rootConstants.width && tid.y < rootConstants.height)
  {
    if (normals[tid.xy].w != 0)
    {
      output[tid.xy] = rootConstants.backgroundColor;
      return;
    }

    if (rootConstants.finalRTV < 5)
    {
      output[tid.xy] = inputColors[rootConstants.finalRTV];
    }
    else
    {
      float3 pos            = output[tid.xy].xyz;
      float3 lightDirection = float3(0.0f, 0.0f, -1.0f);

      float3 l = normalize(lightDirection);
      float3 n = normalize((normals[tid.xy]).xyz);

      float3 v = normalize(pos);
      float3 h = normalize(l + v);

      float d_nl = dot(n, l);

      float  f_diffuse = max(0.0f, d_nl);
      float3 l_diffuse = f_diffuse * albedo[tid.xy].xyz;

      output[tid.xy] = float4(l_diffuse + output[tid.xy].xyz, 1);
    }
  }
}

struct VertexShaderOutput_BoundingBox
{
  float4 position : SV_POSITION;
};

VertexShaderOutput_BoundingBox VS_BoundingBox_main(float3 position : POSITION)
{
  VertexShaderOutput_BoundingBox output;

  float4 p4 = mul(projectionMatrix, mul(modelViewMatrix, float4(position, 1.0f)));

  output.position = p4;
  return output;
}

float4 PS_BoundingBox_main(VertexShaderOutput_BoundingBox input)
    : SV_TARGET
{
  return float4(1.0f, 1.0f, 1.0f, 1.0f);
}