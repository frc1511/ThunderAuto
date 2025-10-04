#pragma once

#include <imgui.h>
#include <cstddef>
#include <memory>
#include <filesystem>

class Texture {
  bool m_loaded = false;

 protected:
  Texture() = default;

 public:
  virtual ~Texture() = default;

  Texture(const Texture&) = delete;
  Texture(Texture&&) = delete;
  Texture& operator=(const Texture&) = delete;
  Texture& operator=(Texture&&) = delete;

  bool isLoaded() const noexcept { return m_loaded; }
  operator bool() const noexcept { return isLoaded(); }

  void loadFromMemory(unsigned char* data, size_t size);
  void loadFromFile(const std::filesystem::path& path);

  virtual int width() const noexcept = 0;
  virtual int height() const noexcept = 0;
  virtual int numChannels() const noexcept = 0;

  ImTextureID id() noexcept;

 protected:
  virtual bool setup() noexcept = 0;
  virtual bool setData(unsigned char* data, int width, int height, int numChannels) noexcept = 0;

  virtual ImTextureID textureID() const noexcept = 0;
};

class PlatformTexture {
 public:
  static std::unique_ptr<Texture> make();
  static std::unique_ptr<Texture> make(unsigned char* data, size_t size);
  static std::unique_ptr<Texture> make(const std::filesystem::path& path);
};
