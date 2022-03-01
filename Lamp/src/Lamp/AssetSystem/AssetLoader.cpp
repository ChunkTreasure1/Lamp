#include "lppch.h"
#include "AssetLoader.h"

#include "Lamp/Utility/SerializeMacros.h"
#include "Lamp/Utility/YAMLSerializationHelpers.h"

#include "Lamp/Mesh/Mesh.h"
#include "Lamp/Mesh/Materials/MaterialLibrary.h"
#include "Lamp/AssetSystem/ResourceCache.h"
#include "Lamp/AssetSystem/BaseAssets.h"

#include "Lamp/Rendering/Vertex.h"
#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/Rendering/Textures/Texture2D.h"
#include "Lamp/Rendering/Renderer.h"	
#include "Lamp/Rendering/RenderPipelineLibrary.h"	
#include "Lamp/Rendering/RenderPipeline.h"	

#include <yaml-cpp/yaml.h>

namespace Lamp
{
	void MeshLoader::Save(const Ref<Asset>& asset) const
	{
		Ref<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(asset);

		//Save raw data
		std::ofstream rout(asset->Path, std::ios::out | std::ios::binary);

		for (auto& subMesh : mesh->GetSubMeshes())
		{
			rout.write((char*)subMesh->GetVertices().data(), subMesh->GetVertices().size() * sizeof(Vertex));
			rout.write((char*)subMesh->GetIndices().data(), subMesh->GetIndices().size() * sizeof(uint32_t));
		}

		rout.close();

		LP_CORE_INFO("Saving mesh {0}", mesh->GetName());

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "geometry" << YAML::Value;
		{
			out << YAML::BeginMap;

			LP_SERIALIZE_PROPERTY(name, mesh->GetName(), out);
			LP_SERIALIZE_PROPERTY(handle, mesh->Handle, out);

			out << YAML::Key << "meshes" << YAML::BeginSeq;
			for (auto& subMesh : mesh->GetSubMeshes())
			{
				out << YAML::BeginMap;
				out << YAML::Key << "mesh" << YAML::Value << subMesh->GetMaterialIndex();
				{
					LP_SERIALIZE_PROPERTY(verticeCount, (uint32_t)subMesh->GetVertices().size(), out);
					LP_SERIALIZE_PROPERTY(indiceCount, (uint32_t)subMesh->GetIndices().size(), out);
				}
				out << YAML::EndMap;
			}
			out << YAML::EndSeq; //Meshes

			out << YAML::Key << "materials" << YAML::BeginSeq;
			for (auto& mat : mesh->GetMaterials())
			{
				out << YAML::BeginMap;
				out << YAML::Key << "material" << YAML::Value << mat.second->GetName();
				{
					LP_SERIALIZE_PROPERTY(id, mat.first, out);
				}
				out << YAML::EndMap;
			}
			out << YAML::EndSeq; //Materials

			out << YAML::EndMap; //Geometry 
		}
		out << YAML::EndMap; //Root

		std::ofstream file;
		file.open(asset->Path.string() + ".spec");
		file << out.c_str();
		file.close();

		LP_CORE_INFO("Saved model {0}!", mesh->GetName());
	}

	bool MeshLoader::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		asset = CreateRef<Mesh>();
		Ref<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(asset);

		//Check if file exists
		if (!std::filesystem::exists(path))
		{
			asset->SetFlag(AssetFlag::Missing);
			return false;
		}

		std::ifstream specFile(path.string() + ".spec");
		if (!specFile.is_open())
		{
			asset->SetFlag(AssetFlag::Invalid);
			return false;
		}

		std::stringstream strStream;
		strStream << specFile.rdbuf();

		specFile.close();

		YAML::Node root = YAML::Load(strStream.str());
		YAML::Node geoNode = root["geometry"];

		std::string meshName;
		meshName = geoNode["name"].as<std::string>();
		LP_DESERIALIZE_PROPERTY(handle, mesh->Handle, geoNode, (AssetHandle)0);

		//Meshes
		YAML::Node meshesNode = geoNode["meshes"];
		std::vector<Ref<SubMesh>> subMeshes;

		//Read file data
		std::ifstream in(path, std::ios::in | std::ios::binary);

		for (auto entry : meshesNode)
		{
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;

			uint32_t matIdInt;
			uint32_t verticeCountInt;
			uint32_t indiceCountInt;

			LP_DESERIALIZE_PROPERTY(mesh, matIdInt, entry, 0);
			LP_DESERIALIZE_PROPERTY(verticeCount, verticeCountInt, entry, 0);
			LP_DESERIALIZE_PROPERTY(indiceCount, indiceCountInt, entry, 0);


			for (size_t i = 0; i < verticeCountInt; i++)
			{
				Vertex vert;
				in.read((char*)&vert, sizeof(Vertex));

				vertices.push_back(vert);
			}
			for (size_t i = 0; i < indiceCountInt; i++)
			{
				uint32_t indice;
				in.read((char*)&indice, sizeof(uint32_t));

				indices.push_back(indice);
			}

			subMeshes.push_back(CreateRef<SubMesh>(vertices, indices, matIdInt));
		}
		in.close();

		//Materials
		YAML::Node materialsNode = geoNode["materials"];
		std::map<uint32_t, Ref<Material>> materials;

		for (auto entry : materialsNode)
		{
			uint32_t idInt;
			std::string matName;

			matName = entry["material"].as<std::string>();
			LP_DESERIALIZE_PROPERTY(id, idInt, entry, 0);

			if (MaterialLibrary::Get().IsMaterialLoaded(matName))
			{
				materials[idInt] = MaterialLibrary::GetMaterial(matName);
			}
			else
			{
				LP_CORE_WARN("Material {0} not loaded!", matName);
			}
		}

		asset = CreateRef<Mesh>(meshName, subMeshes, materials);
		asset->Path = path;

		return true;
	}

	bool TextureLoader::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		asset = Texture2D::Create(path);
		asset->Path = path;

		return true;
	}

	void TextureLoader::Save(const Ref<Asset>& asset) const
	{
	}

	bool EnvironmentLoader::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		return false;
	}

	void MaterialLoader::Save(const Ref<Asset>& asset) const
	{
		Ref<Material> mat = std::dynamic_pointer_cast<Material>(asset);

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "material" << YAML::Value;
		{
			out << YAML::BeginMap;

			LP_SERIALIZE_PROPERTY(name, mat->GetName(), out);
			LP_SERIALIZE_PROPERTY(handle, mat->Handle, out);

			out << YAML::Key << "textures" << YAML::Value;
			out << YAML::BeginMap;
			{
				//TODO: fix
				//for (auto& tex : mat->GetTextures())
				//{
				//	LP_SERIALIZE_PROPERTY_STRING(tex.first, tex.second->Handle, out);
				//}
			}
			out << YAML::EndMap;

			LP_SERIALIZE_PROPERTY(shader, mat->GetShader()->GetName(), out);
			LP_SERIALIZE_PROPERTY(renderPipeline, (uint32_t)RenderPipelineLibrary::Get().GetTypeFromPipeline(mat->GetPipeline()), out);

			const auto& matData = mat->GetMaterialData();
			LP_SERIALIZE_PROPERTY(useBlending, matData.useBlending, out);
			LP_SERIALIZE_PROPERTY(useAlbedo, matData.useAlbedo, out);
			LP_SERIALIZE_PROPERTY(useNormal, matData.useNormal, out);
			LP_SERIALIZE_PROPERTY(useMRO, matData.useMRO, out);
			LP_SERIALIZE_PROPERTY(useDetailNormal, matData.useDetailNormal, out);
			LP_SERIALIZE_PROPERTY(useTranslucency, matData.useTranslucency, out);

			LP_SERIALIZE_PROPERTY(blendingMultiplier, matData.blendingMultiplier, out);
			LP_SERIALIZE_PROPERTY(mroColor, matData.mroColor, out);
			LP_SERIALIZE_PROPERTY(albedoColor, matData.albedoColor, out);
			LP_SERIALIZE_PROPERTY(normalColor, matData.normalColor, out);

			out << YAML::EndMap;
		}

		out << YAML::EndMap;

		std::ofstream fout(asset->Path);
		fout << out.c_str();
		fout.close();
	}

	bool MaterialLoader::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		asset = Material::Create();
		Ref<Material> mat = std::dynamic_pointer_cast<Material>(asset);

		if (!std::filesystem::exists(path))
		{
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		std::ifstream stream(path);
		if (!stream.is_open())
		{
			return false;
		}

		std::stringstream strStream;
		strStream << stream.rdbuf();

		stream.close();

		YAML::Node root = YAML::Load(strStream.str());
		YAML::Node materialNode = root["material"];

		mat->SetName(materialNode["name"].as<std::string>());
		LP_DESERIALIZE_PROPERTY(handle, asset->Handle, materialNode, AssetHandle(0));
		mat->SetRenderPipeline(RenderPipelineLibrary::Get().GetPipeline(materialNode["renderPipeline"] ? (ERenderPipeline)materialNode["renderPipeline"].as<uint32_t>() : ERenderPipeline::Deferred));

		YAML::Node textureNode = materialNode["textures"];

		for (auto& texName : mat->GetShader()->GetSpecification().inputTextureNames)
		{
			if (textureNode[texName])
			{
				AssetHandle textureHandle = textureNode[texName].as<AssetHandle>();
				auto path = g_pEnv->pAssetManager->GetPathFromAssetHandle(textureHandle);

				mat->SetTexture(texName, ResourceCache::GetAsset<Texture2D>(path));
			}
			else
			{
				mat->SetTexture(texName, Renderer::Get().GetDefaults().whiteTexture);
			}
		}

		auto& matData = const_cast<MaterialData&>(mat->GetMaterialData());

		LP_DESERIALIZE_PROPERTY(useBlending, matData.useBlending, materialNode, false);
		LP_DESERIALIZE_PROPERTY(useAlbedo, matData.useAlbedo, materialNode, true);
		LP_DESERIALIZE_PROPERTY(useNormal, matData.useNormal, materialNode, true);
		LP_DESERIALIZE_PROPERTY(useMRO, matData.useMRO, materialNode, true);
		LP_DESERIALIZE_PROPERTY(useDetailNormal, matData.useDetailNormal, materialNode, false);
		LP_DESERIALIZE_PROPERTY(useTranslucency, matData.useTranslucency, materialNode, false);

		LP_DESERIALIZE_PROPERTY(blendingMultiplier, matData.blendingMultiplier, materialNode, 1.f);

		LP_DESERIALIZE_PROPERTY(mroColor, matData.mroColor, materialNode, glm::vec2(0.f, 1.f));
		LP_DESERIALIZE_PROPERTY(albedoColor, matData.albedoColor, materialNode, glm::vec4(1.f, 1.f, 1.f, 1.f));
		LP_DESERIALIZE_PROPERTY(normalColor, matData.normalColor, materialNode, glm::vec4(0.f, 1.f, 0.f, 0.f));

		asset->Path = path;

		return true;
	}

	void MeshSourceLoader::Save(const Ref<Asset>& asset) const
	{
		Ref<MeshSource> mesh = std::dynamic_pointer_cast<MeshSource>(asset);

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "meshsource" << YAML::Value;
		{
			out << YAML::BeginMap;

			LP_SERIALIZE_PROPERTY(handle, mesh->Handle, out);
			LP_SERIALIZE_PROPERTY(sourcePath, mesh->m_importSettings.path.string(), out);
			LP_SERIALIZE_PROPERTY(units, (uint32_t)mesh->m_importSettings.units, out);
			LP_SERIALIZE_PROPERTY(upDir, mesh->m_importSettings.upDirection, out);
			LP_SERIALIZE_PROPERTY(compileStatic, mesh->m_importSettings.compileStatic, out);
			LP_SERIALIZE_PROPERTY(meshHandle, mesh->m_mesh, out);

			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		std::ofstream fout(asset->Path);
		fout << out.c_str();
		fout.close();
	}

	bool MeshSourceLoader::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		std::ifstream stream(path);
		if (!stream.is_open())
		{
			return false;
		}

		std::stringstream strStream;
		strStream << stream.rdbuf();
		stream.close();

		YAML::Node root = YAML::Load(strStream.str());
		YAML::Node meshNode = root["meshsource"];

		asset = CreateRef<MeshSource>();
		Ref<MeshSource> meshSource = std::dynamic_pointer_cast<MeshSource>(asset);

		meshSource->m_importSettings.units = meshNode["units"] ? (Units)meshNode["units"].as<uint32_t>() : Units::Centimeters;
		LP_DESERIALIZE_PROPERTY(upDir, meshSource->m_importSettings.upDirection, meshNode, glm::vec3(0.f));
		LP_DESERIALIZE_PROPERTY(compileStatic, meshSource->m_importSettings.compileStatic, meshNode, false);
		LP_DESERIALIZE_PROPERTY(handle, asset->Handle, meshNode, AssetHandle(0));
		meshSource->m_importSettings.path = std::filesystem::path(meshNode["sourcePath"].as<std::string>());
		if (!std::filesystem::exists(meshSource->m_importSettings.path))
		{
			LP_CORE_ERROR("Mesh {0} not found!", meshSource->m_importSettings.path.string());
			meshSource->SetFlag(AssetFlag::Missing, true);

			return false;
		}

		LP_DESERIALIZE_PROPERTY(meshHandle, meshSource->m_mesh, meshNode, AssetHandle(0));
		meshSource->Path = path;

		return true;
	}
}