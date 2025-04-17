#pragma once

#include <ThunderAuto/thunder_auto.hpp>

class Texture {
#if THUNDER_AUTO_DIRECTX11
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture_view;
  Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;
#else // THUNDER_AUTO_OPENGL
  unsigned int m_texture = 0;
#endif

  bool m_loaded = false;
  int m_width = 1, m_height = 1, m_nr_channels = 4;

public:
  Texture() = default;
  Texture(unsigned char* data, size_t size);
  explicit Texture(const char* path);
  ~Texture();

  bool is_loaded() const { return m_loaded; }
  operator bool() const { return m_loaded; }

  void load_from_memory(unsigned char* data, size_t size);
  void load_from_file(const char* path);

  int width() const { return m_width; }
  int height() const { return m_height; }
  int nr_channels() const { return m_nr_channels; }

  ImTextureID id();

private:
  void setup();

  void set_data(unsigned char* data, int width, int height, int nr_channels);
};

