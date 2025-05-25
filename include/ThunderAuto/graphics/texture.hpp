#pragma once

#include <ThunderAuto/thunder_auto.hpp>

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

  bool is_loaded() const { return m_loaded; }
  operator bool() const { return is_loaded(); }

  bool load_from_memory(unsigned char* data, size_t size);
  bool load_from_file(const char* path);

  virtual int width() const = 0;
  virtual int height() const = 0;
  virtual int num_channels() const = 0;

  ImTextureID id();

protected:
  virtual bool setup() = 0;
  virtual bool set_data(unsigned char* data, int width, int height,
                        int num_channels) = 0;
  
  virtual ImTextureID texture_id() const = 0;
};

class PlatformTexture {
 public:
  static std::unique_ptr<Texture> make();
  static std::unique_ptr<Texture> make(unsigned char* data, size_t size);
  static std::unique_ptr<Texture> make(const char* path);
};
