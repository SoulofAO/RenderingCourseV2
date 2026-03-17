#pragma once

#include <wrl/client.h>
#include <string>

struct ID3D11Device;
struct ID3D11ShaderResourceView;
class ResourceManager;

class MeshResourceBindingHelper
{
public:
	static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> LoadTexture(
		ResourceManager* Resources,
		ID3D11Device* Device,
		const std::string& TexturePath);
};
