#include "texture_opengl.hpp"
#include "graphics_opengl.hpp"
#include <glad/glad.h>

TextureOpenGL::~TextureOpenGL() {
  if (m_texture)
    glDeleteTextures(1, &m_texture);
}

bool TextureOpenGL::setup() {
  if (m_texture)
    return true;

  if (!GraphicsOpenGL::get().is_initialized()) {
    assert(false);
    return false;
  }

  glGenTextures(1, &m_texture);
  glBindTexture(GL_TEXTURE_2D, m_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  return true;
}

bool TextureOpenGL::set_data(unsigned char* data,
                             int width,
                             int height,
                             int num_channels) {
  if (!m_texture) {
    if (!setup())
      return false;
  }

  m_width = width;
  m_height = height;
  m_num_channels = num_channels;

  const int tex_channels = (m_num_channels == 3 ? GL_RGB : GL_RGBA);

  glBindTexture(GL_TEXTURE_2D, m_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, tex_channels, m_width, m_height, 0,
               tex_channels, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  return true;
}
