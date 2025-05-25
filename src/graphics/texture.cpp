#include <ThunderAuto/graphics/texture.hpp>

#if THUNDER_AUTO_DIRECTX11
#include "texture_directx11.hpp"
#elif THUNDER_AUTO_OPENGL
#include "texture_opengl.hpp"
#else
#error "Unknown graphics API"
#endif

#include <stb_image.h>

bool Texture::load_from_memory(unsigned char* data, size_t size) {
  if (!texture_id()) {
    if (!setup())
      return false;
  }

  int width = 0, height = 0, num_channels = 0;

  unsigned char* img_data = stbi_load_from_memory(
      data, int(size), &width, &height, &num_channels, 0);
  if (!img_data) return false;

  bool result = set_data(img_data, width, height, num_channels);

  stbi_image_free(img_data);

  return (m_loaded = result);
}

bool Texture::load_from_file(const char* path) {
  if (!texture_id()) {
    if (!setup())
      return false;
  }

  int width = 0, height = 0, num_channels = 0;

  unsigned char* img_data =
      stbi_load(path, &width, &height, &num_channels, 0);
  if (!img_data)
    return false;

  bool result = set_data(img_data, width, height, num_channels);

  stbi_image_free(img_data);

  return (m_loaded = result);
}

ImTextureID Texture::id() {
  ImTextureID id = texture_id();
  if (id)
    return id;

  (void)setup();
  return texture_id();
}

std::unique_ptr<Texture> PlatformTexture::make() {
#if THUNDER_AUTO_DIRECTX11
  return std::make_unique<TextureDirectX11>();
#else  // THUNDER_AUTO_OPENGL
  return std::make_unique<TextureOpenGL>();
#endif
}

std::unique_ptr<Texture> PlatformTexture::make(unsigned char* data, size_t size) {
  std::unique_ptr<Texture> texture = PlatformTexture::make();
  assert(texture);
  texture->load_from_memory(data, size);
  return texture;
}

std::unique_ptr<Texture> PlatformTexture::make(const char* path) {
  std::unique_ptr<Texture> texture = PlatformTexture::make();
  assert(texture);
  texture->load_from_file(path);
  return texture;
}
