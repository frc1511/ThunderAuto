#pragma once

#include <ThunderAuto/Graphics/Texture.hpp>

#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>

class TextureDirectX11 final : public Texture {
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureView;
  Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;

  int m_textureWidth = -1, m_textureHeight = -1;

  int m_width = 1, m_height = 1, m_numChannels = 4;

 public:
  static std::unique_ptr<TextureDirectX11> make() { return std::make_unique<TextureDirectX11>(); }

  int width() const noexcept override { return m_width; }
  int height() const noexcept override { return m_height; }
  int numChannels() const noexcept override { return m_numChannels; }

 private:
  bool setup() noexcept override;
  bool setData(unsigned char* data, int width, int height, int num_channels) noexcept override;

  ImTextureID textureID() const noexcept override { return (ImTextureID)m_textureView.Get(); }
};
