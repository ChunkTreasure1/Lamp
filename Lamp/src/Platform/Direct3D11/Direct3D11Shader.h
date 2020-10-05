#pragma once

#include "Lamp/Rendering/Shader/Shader.h"
#include <wrl.h>
#include <d3d11.h>

namespace Lamp
{
	class Direct3D11Shader : public Shader
	{
	public:
		Direct3D11Shader(std::initializer_list<std::string> paths);
		virtual ~Direct3D11Shader() override;

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void UploadBool(const std::string& name, bool value) const override;
		virtual void UploadInt(const std::string& name, int value) const override;
		virtual void UploadFloat(const std::string& name, float value) const override;
		virtual void UploadFloat3(const std::string& name, const glm::vec3& value) const override;
		virtual void UploadFloat4(const std::string& name, const glm::vec4& value) const override;

		virtual void UploadMat4(const std::string& name, const glm::mat4& mat) override;
		virtual void UploadMat3(const std::string& name, const glm::mat3& mat) override;
		virtual void UploadIntArray(const std::string& name, int* values, uint32_t count) const override;

		virtual const std::string& GetName() override { return m_Name; }
		virtual std::string& GetFragmentPath() override { return m_FragmentPath; }
		virtual std::string& GetVertexPath() override { return m_VertexPath; }

		Microsoft::WRL::ComPtr<ID3DBlob>& GetVertexBlob() { return m_pVertexBlob; }

	private:
		std::string m_Name;
		std::string m_FragmentPath;
		std::string m_VertexPath;

		std::vector<std::string> m_Paths;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pVertex;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pPixel;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pBlob;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pVertexBlob;
	};
}