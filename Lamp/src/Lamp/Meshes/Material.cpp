#include "lppch.h"
#include "Material.h"

namespace Lamp
{
	void Material::UploadData()
	{
		m_pShader->Bind();

		m_pShader->UploadInt("u_Material.diffuse", 0);
		m_pShader->UploadInt("u_Material.specular", 1);
		m_pShader->UploadInt("u_Material.shininess", m_Shininess);
	}
}