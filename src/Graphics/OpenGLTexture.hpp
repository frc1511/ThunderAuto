#pragma once

#include <ThunderAuto/Graphics/Texture.hpp>

class TextureOpenGL final : public Texture {
  unsigned int m_texture = 0;
  int m_width = 1, m_height = 1, m_numChannels = 4;

 public:
  ~TextureOpenGL();

  static std::unique_ptr<TextureOpenGL> make() { return std::make_unique<TextureOpenGL>(); }

  int width() const noexcept override { return m_width; }
  int height() const noexcept override { return m_height; }
  int numChannels() const noexcept override { return m_numChannels; }

 private:
  bool setup() noexcept override;
  bool setData(unsigned char* data, int width, int height, int numChannels) noexcept override;

  ImTextureID textureID() const noexcept override { return (ImTextureID)m_texture; }
};
