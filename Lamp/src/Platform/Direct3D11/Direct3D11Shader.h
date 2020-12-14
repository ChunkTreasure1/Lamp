#pragma once

#include "Lamp/Rendering/Shader/Shader.h"
#include <wrl.h>
#include <d3d11.h>
#include "Lamp/Math/Matrix.h"

namespace Lamp
{
	class Direct3D11Shader : public Shader
	{
	public:
		Direct3D11Shader(std::initializer_list<std::string> paths);
		virtual ~Direct3D11Shader() override;

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void UploadData(const ShaderData& data) override;

		virtual const std::string& GetName() override { return m_Name; }
		virtual std::string& GetFragmentPath() override { return m_FragmentPath; }
		virtual std::string& GetVertexPath() override { return m_VertexPath; }

		Microsoft::WRL::ComPtr<ID3DBlob>& GetVertexBlob() { return m_pVertexBlob; }

	private:
		struct ShaderBuffer
		{
			bool* bools = nullptr;
			int* ints = nullptr;
			float* floats = nullptr;
			glm::vec2* vec2s = nullptr;
			glm::vec3* vec3s = nullptr;
			glm::vec4* vec4s = nullptr;
			Math::mat4* mat4s = nullptr;

			~ShaderBuffer()
			{
				if (bools) delete[] bools;
				if (ints) delete[] ints;
				if (floats) delete[] floats;
				if (vec2s) delete[] vec2s;
				if (vec3s) delete[] vec3s;
				if (vec4s) delete[] vec4s;
				if (mat4s) delete[] mat4s;
			}
		};

		void AllocateUniforms(ShaderBuffer& b, const uint32_t* values);

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