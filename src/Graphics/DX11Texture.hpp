#pragma once

#include <ThunderAuto/graphics/texture.hpp>

#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>

class TextureDirectX11 final : public Texture {
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture_view;
  Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;

  int m_texture_width = -1, m_texture_height = -1;

  int m_width = 1, m_height = 1, m_num_channels = 4;

 public:
  static std::unique_ptr<TextureDirectX11> make() { return std::make_unique<TextureDirectX11>(); }

  int width() const override { return m_width; }
  int height() const override { return m_height; }
  int num_channels() const override { return m_num_channels; }

 private:
  bool setup() override;
  bool set_data(unsigned char* data, int width, int height, int num_channels) override;

  ImTextureID texture_id() const override { return (ImTextureID)m_texture_view.Get(); }
};
