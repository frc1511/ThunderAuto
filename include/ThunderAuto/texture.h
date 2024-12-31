#pragma once

#include <ThunderAuto/thunder_auto.h>

class Texture {
#if TH_DIRECTX11
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture_view;
  Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;
#else // TH_OPENGL
  unsigned int m_texture;
#endif

  bool m_loaded = false;
  int m_width = 1, m_height = 1, m_nr_channels = 4;

public:
  Texture();
  Texture(unsigned char* data, std::size_t size);
  explicit Texture(const char* path);
  ~Texture();

  bool is_loaded() const { return m_loaded; }
  operator bool() const { return m_loaded; }

  void init(unsigned char* data, std::size_t size);
  void init(const char* path);

  void set_data(unsigned char* data, int width, int height, int nr_channels);

  int width() const { return m_width; }
  int height() const { return m_height; }
  int nr_channels() const { return m_nr_channels; }

  ImTextureID id() const {
    return reinterpret_cast<void*>(
#if TH_DIRECTX11
        m_texture_view.Get()
#else // TH_OPENGL
        m_texture
#endif
    );
  }

private:
  void setup();
};

