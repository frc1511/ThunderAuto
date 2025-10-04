#include "OpenGLTexture.hpp"
#include "OpenGLGraphics.hpp"
#include <glad/glad.h>

#include <ThunderAuto/Logger.hpp>

TextureOpenGL::~TextureOpenGL() {
  if (m_texture)
    glDeleteTextures(1, &m_texture);
}

bool TextureOpenGL::setup() noexcept {
  if (m_texture)
    return true;

  if (!GraphicsOpenGL::get().isInitialized()) {
    ThunderAutoLogger::Error("OpenGL Graphics singleton is not initialized, cannot create texture");
    return false;
  }

  glGenTextures(1, &m_texture);
  glBindTexture(GL_TEXTURE_2D, m_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  return true;
}

bool TextureOpenGL::setData(unsigned char* data, int width, int height, int numChannels) noexcept {
  if (!m_texture) {
    if (!setup())
      return false;
  }

  m_width = width;
  m_height = height;
  m_numChannels = numChannels;

  const int texChannels = (m_numChannels == 3 ? GL_RGB : GL_RGBA);

  glBindTexture(GL_TEXTURE_2D, m_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, texChannels, m_width, m_height, 0, texChannels, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  return true;
}
