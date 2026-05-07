
#pragma once

#include <d3d11.h>
#include <wrl/client.h>

namespace DirectX
{
    HRESULT CreateDDSTextureFromFile(
        ID3D11Device* d3dDevice,
        const wchar_t* szFileName,
        ID3D11Resource** texture,
        ID3D11ShaderResourceView** textureView,
        size_t maxsize = 0
    );
}
