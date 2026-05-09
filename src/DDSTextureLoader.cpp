
#include "DDSTextureLoader.h"
#include "DirectXTex.h"

using namespace DirectX;

HRESULT DirectX::CreateDDSTextureFromFile(
    ID3D11Device* d3dDevice,
    const wchar_t* szFileName,
    ID3D11Resource** texture,
    ID3D11ShaderResourceView** textureView,
    size_t maxsize)
{
    TexMetadata metadata = {};
    ScratchImage image = {};
    HRESULT hr = LoadFromDDSFile(szFileName, DDS_FLAGS_NONE, &metadata, image);
    if (FAILED(hr)) return hr;

    return CreateShaderResourceView(d3dDevice,
        image.GetImages(), image.GetImageCount(), metadata, textureView);
}
