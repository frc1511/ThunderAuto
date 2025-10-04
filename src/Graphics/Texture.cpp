#include <ThunderAuto/Graphics/Texture.hpp>

#include <ThunderAuto/Error.hpp>

#if THUNDERAUTO_DIRECTX11
#include "DX11Texture.hpp"
#elif THUNDERAUTO_OPENGL
#include "OpenGLTexture.hpp"
#else
#error "Unknown graphics API"
#endif

#include <stb_image.h>

void Texture::loadFromMemory(unsigned char* data, size_t size) {
  if (!data || size == 0) {
    throw InvalidArgumentError::Construct("Texture data is null or size is zero");
  }

  if (!textureID()) {
    if (!setup()) {
      throw RuntimeError::Construct("Failed to setup texture");
    }
  }

  ThunderAutoLogger::Info("Loading texture from memory, size: {} bytes", size);

  int width = 0, height = 0, numChannels = 0;

  unsigned char* imgData = stbi_load_from_memory(data, int(size), &width, &height, &numChannels, 0);
  if (!imgData) {
    throw RuntimeError::Construct("Failed to load image from memory: {}", stbi_failure_reason());
  }

  bool result = setData(imgData, width, height, numChannels);

  stbi_image_free(imgData);

  if (!result) {
    throw RuntimeError::Construct("Failed to set texture data from memory");
  }
}

void Texture::loadFromFile(const std::filesystem::path& path) {
  if (path.empty()) {
    throw InvalidArgumentError::Construct("Texture file path is empty");
  }
  if (!std::filesystem::exists(path)) {
    throw RuntimeError::Construct("Texture file '{}' does not exist", path.string().c_str());
  }

  if (!textureID()) {
    if (!setup()) {
      throw RuntimeError::Construct("Failed to setup texture");
    }
  }

  ThunderAutoLogger::Info("Loading texture from file '{}'", path.string().c_str());

  int width = 0, height = 0, numChannels = 0;

  unsigned char* imgData = stbi_load(path.c_str(), &width, &height, &numChannels, 0);
  if (!imgData) {
    throw RuntimeError::Construct("Failed to load image from file '{}': {}", path.string().c_str(),
                                  stbi_failure_reason());
  }

  bool result = setData(imgData, width, height, numChannels);

  stbi_image_free(imgData);

  if (!result) {
    throw RuntimeError::Construct("Failed to set texture data from file '{}'", path.string().c_str());
  }
}

ImTextureID Texture::id() noexcept {
  ImTextureID id = textureID();
  if (id)
    return id;

  (void)setup();
  return textureID();
}

std::unique_ptr<Texture> PlatformTexture::make() {
#if THUNDERAUTO_DIRECTX11
  return std::make_unique<TextureDirectX11>();
#else  // THUNDERAUTO_OPENGL
  return std::make_unique<TextureOpenGL>();
#endif
}

std::unique_ptr<Texture> PlatformTexture::make(unsigned char* data, size_t size) {
  std::unique_ptr<Texture> texture = PlatformTexture::make();
  texture->loadFromMemory(data, size);  // will throw if error
  return texture;
}

std::unique_ptr<Texture> PlatformTexture::make(const std::filesystem::path& path) {
  std::unique_ptr<Texture> texture = PlatformTexture::make();
  texture->loadFromFile(path);  // will throw if error
  return texture;
}
