#include <ThunderAuto/texture.h>

#include <ThunderAuto/graphics.h>

#if THUNDER_AUTO_OPENGL
#include <glad/glad.h>
#endif

#include <stb_image.h>

Texture::Texture() { setup(); }

Texture::Texture(unsigned char* img, std::size_t img_size) {
  init(img, img_size);
}

Texture::Texture(const char* path) { init(path); }

void Texture::init(unsigned char* data, std::size_t size) {
  unsigned char* img_data = stbi_load_from_memory(data, int(size), &m_width,
                                                  &m_height, &m_nr_channels, 0);

  setup();
  set_data(img_data, m_width, m_height, m_nr_channels);

  stbi_image_free(img_data);

  m_loaded = true;
}

void Texture::init(const char* path) {
  unsigned char* img_data =
      stbi_load(path, &m_width, &m_height, &m_nr_channels, 0);
  m_loaded = (img_data != nullptr);
  if (!m_loaded) return;

  setup();
  set_data(img_data, m_width, m_height, m_nr_channels);

  stbi_image_free(img_data);
}

Texture::~Texture() {
#if THUNDER_AUTO_DIRECTX11
  m_texture_view.Reset();
  m_texture.Reset();
#else // THUNDER_AUTO_OPENGL
  glDeleteTextures(1, &m_texture);
#endif
}

void Texture::setup() {
#if THUNDER_AUTO_DIRECTX11
  m_texture = nullptr;
  m_texture_view = nullptr;

  D3D11_TEXTURE2D_DESC desc;
  ZeroMemory(&desc, sizeof(desc));
  desc.Width = m_width;
  desc.Height = m_height;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  desc.SampleDesc.Count = 1;
  desc.Usage = D3D11_USAGE_DYNAMIC;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  HRESULT hr =
      Graphics::get().device()->CreateTexture2D(&desc, nullptr, &m_texture);
  if (FAILED(hr)) return;

  // Create texture view
  D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
  ZeroMemory(&srvDesc, sizeof(srvDesc));
  srvDesc.Format = desc.Format;
  srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MostDetailedMip = 0;
  srvDesc.Texture2D.MipLevels = desc.MipLevels;
  hr = Graphics::get().device()->CreateShaderResourceView(
      m_texture.Get(), &srvDesc, &m_texture_view);
  if (FAILED(hr)) return;

#else // THUNDER_AUTO_OPENGL
  glGenTextures(1, &m_texture);
  glBindTexture(GL_TEXTURE_2D, m_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#endif
}

void Texture::set_data(unsigned char* data, int width, int height,
                       int nr_channels) {
#if THUNDER_AUTO_DIRECTX11
  assert(nr_channels == 4);
  if (width != m_width || height != m_height) {
#endif
    m_width = width;
    m_height = height;
    m_nr_channels = nr_channels;
#if THUNDER_AUTO_DIRECTX11
    setup();
  }

  ID3D11DeviceContext* context = Graphics::get().context();

  D3D11_MAPPED_SUBRESOURCE mapped_resource;
  HRESULT hr = context->Map(m_texture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0,
                            &mapped_resource);
  if (FAILED(hr)) return;

  for (UINT row = 0; row < m_height; ++row) {
    memcpy(static_cast<unsigned char*>(mapped_resource.pData) +
               row * mapped_resource.RowPitch,
           data + row * m_width * 4, m_width * 4);
  }

  context->Unmap(m_texture.Get(), 0);

#else // THUNDER_AUTO_OPENGL
  const int tex_channels = (m_nr_channels == 3 ? GL_RGB : GL_RGBA);

  glBindTexture(GL_TEXTURE_2D, m_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, tex_channels, m_width, m_height, 0,
               tex_channels, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);
#endif
}

