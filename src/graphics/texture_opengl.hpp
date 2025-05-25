#pragma once

#include <ThunderAuto/graphics/texture.hpp>

class TextureOpenGL final : public Texture {
  unsigned int m_texture = 0;
  int m_width = 1, m_height = 1, m_num_channels = 4;

 public:
  ~TextureOpenGL();

  static std::unique_ptr<TextureOpenGL> make() {
    return std::make_unique<TextureOpenGL>();
  }

  int width() const override { return m_width; }
  int height() const override { return m_height; }
  int num_channels() const override { return m_num_channels; }

 private:
  bool setup() override;
  bool set_data(unsigned char* data,
                int width,
                int height,
                int num_channels) override;

  ImTextureID texture_id() const override { return (ImTextureID)m_texture; }
};
