#include "DX11Texture.hpp"
#include "DX11Graphics.hpp"

bool TextureDirectX11::setup() noexcept {
  if (m_width == m_textureWidth && m_height == m_textureHeight)
    return true;

  if (!GraphicsDirectX11::get().isInitialized()) {
    assert(false);
    return false;
  }

  ID3D11Device* device = GraphicsDirectX11::get().device();
  assert(device != nullptr);

  m_texture = nullptr;
  m_textureView = nullptr;

  HRESULT hr;

  D3D11_TEXTURE2D_DESC desc;
  ZeroMemory(&desc, sizeof(desc));
  desc.Width = m_width;
  desc.Height = m_height;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  desc.SampleDesc.Count = 1;
  desc.Usage = D3D11_USAGE_DYNAMIC;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  hr = device->CreateTexture2D(&desc, nullptr, &m_texture);
  if (FAILED(hr))
    return false;

  // Create texture view
  D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
  ZeroMemory(&srvDesc, sizeof(srvDesc));
  srvDesc.Format = desc.Format;
  srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MostDetailedMip = 0;
  srvDesc.Texture2D.MipLevels = desc.MipLevels;

  hr = device->CreateShaderResourceView(m_texture.Get(), &srvDesc, &m_textureView);
  if (FAILED(hr))
    return false;

  m_textureWidth = m_width;
  m_textureHeight = m_height;

  return true;
}

bool TextureDirectX11::setData(unsigned char* data, int width, int height, int numChannels) noexcept {
  if (numChannels != 4) {
    assert(false);
    return false;
  }

  m_width = width;
  m_height = height;
  m_numChannels = numChannels;

  if (!setup())
    return false;

  ID3D11DeviceContext* context = GraphicsDirectX11::get().context();

  D3D11_MAPPED_SUBRESOURCE mappedResource;
  HRESULT hr = context->Map(m_texture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
  if (FAILED(hr))
    return false;

  for (UINT row = 0; row < m_height; ++row) {
    memcpy(static_cast<unsigned char*>(mappedResource.pData) + row * mappedResource.RowPitch,
           data + row * m_width * 4, m_width * 4);
  }

  context->Unmap(m_texture.Get(), 0);

  return true;
}
