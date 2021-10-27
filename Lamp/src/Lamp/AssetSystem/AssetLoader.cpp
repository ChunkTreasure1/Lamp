#include "lppch.h"
#include "AssetLoader.h"

#include "Lamp/Utility/SerializeMacros.h"
#include "Lamp/Utility/YAMLSerializationHelpers.h"

#include "Lamp/Mesh/Mesh.h"
#include "Lamp/Mesh/Materials/MaterialLibrary.h"
#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/AssetSystem/ResourceCache.h"
#include "Lamp/AssetSystem/BaseAssets.h"

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
			rout.write((char*)subMesh->GetIndices().data(), subMesh->GetIndices().size() * sizeof(Vertex));
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

			out << YAML::Key << "meshes" << YAML::Value;
			out << YAML::BeginMap;
			{
				uint32_t meshCount = 0;
				for (auto& subMesh : mesh->GetSubMeshes())
				{
					out << YAML::Key << "mesh" + std::to_string(meshCount) << YAML::Value;
					out << YAML::BeginMap;
					{
						LP_SERIALIZE_PROPERTY(matId, subMesh->GetMaterialIndex(), out);
						LP_SERIALIZE_PROPERTY(verticeCount, (uint32_t)subMesh->GetVertices().size(), out);
						LP_SERIALIZE_PROPERTY(indiceCount, (uint32_t)subMesh->GetIndices().size(), out);
					}
					out << YAML::EndMap;
					meshCount++;
				}
			}
			out << YAML::EndMap; //Meshes

			out << YAML::Key << "materials" << YAML::Value;
			out << YAML::BeginMap;
			{
				uint32_t matCount = 0;
				for (auto& mat : mesh->GetMaterials())
				{
					out << YAML::Key << "material" + std::to_string(matCount) << YAML::Value;
					out << YAML::BeginMap;
					{
						LP_SERIALIZE_PROPERTY(name, mat.second->GetName(), out);
						LP_SERIALIZE_PROPERTY(id, mat.first, out);
					}
					out << YAML::EndMap;
					matCount++;
				}
			}
			out << YAML::EndMap; //Materials

			out << YAML::Key << "boundingBox" << YAML::Value;
			out << YAML::BeginMap;
			{
				LP_SERIALIZE_PROPERTY(maxPos, mesh->GetBoundingBox().Max, out);
				LP_SERIALIZE_PROPERTY(minPos, mesh->GetBoundingBox().Min, out);
			}
			out << YAML::EndMap;

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
		struct stat b;
		if (!(stat(path.string().c_str(), &b) == 0))
		{
			asset->SetFlag(AssetFlag::Missing);
			return false;
		}

		std::ifstream specFile(path.string() + ".spec");
		if (!specFile.is_open())
		{
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
		uint32_t meshCount = 0;
		std::vector<Ref<SubMesh>> subMeshes;

		while (YAML::Node meshNode = meshesNode["mesh" + std::to_string(meshCount)])
		{
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;

			uint32_t matIdInt;
			uint32_t verticeCountInt;
			uint32_t indiceCountInt;

			LP_DESERIALIZE_PROPERTY(matId, matIdInt, meshNode, 0);
			LP_DESERIALIZE_PROPERTY(verticeCount, verticeCountInt, meshNode, 0);
			LP_DESERIALIZE_PROPERTY(indiceCount, indiceCountInt, meshNode, 0);

			//Read file data
			std::ifstream in(path, std::ios::in | std::ios::binary);
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
			in.close();

			subMeshes.push_back(CreateRef<SubMesh>(vertices, indices, matIdInt));
			meshCount++;
		}

		//Materials
		YAML::Node materialsNode = geoNode["materials"];
		uint32_t matCount = 0;
		std::map<uint32_t, Ref<Material>> materials;

		while (YAML::Node matNode = materialsNode["material" + std::to_string(matCount)])
		{
			uint32_t idInt;
			std::string matName;

			matName = matNode["name"].as<std::string>();
			LP_DESERIALIZE_PROPERTY(id, idInt, matNode, 0);

			if (MaterialLibrary::IsMaterialLoaded(matName))
			{
				materials[idInt] = MaterialLibrary::GetMaterial(matName);
			}
			else
			{
				LP_CORE_WARN("Material {0} not loaded!", matName);
			}

			matCount++;
		}

		//Bounding box
		YAML::Node bbNode = geoNode["boundingBox"];
		AABB boundingBox;

		LP_DESERIALIZE_PROPERTY(maxPos, boundingBox.StartMax, bbNode, glm::vec3(0.f));
		LP_DESERIALIZE_PROPERTY(minPos, boundingBox.StartMin, bbNode, glm::vec3(0.f));
		boundingBox.Max = boundingBox.StartMax;
		boundingBox.Min = boundingBox.StartMin;

		asset = CreateRef<Mesh>(meshName, subMeshes, materials, boundingBox);
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
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "TextureData" << YAML::Value;
		{
			out << YAML::BeginMap;

			LP_SERIALIZE_PROPERTY(handle, asset->Handle, out);

			out << YAML::EndMap;
		}

		out << YAML::EndMap;
		std::ofstream fout(asset->Path.string() + ".spec");
		fout << out.c_str();
		fout.close();
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
				for (auto& tex : mat->GetTextures())
				{
					LP_SERIALIZE_PROPERTY_STRING(tex.first, tex.second->Handle, out);
				}
			}
			out << YAML::EndMap;

			LP_SERIALIZE_PROPERTY(shader, mat->GetShader()->GetName(), out);

			out << YAML::EndMap;
		}

		out << YAML::EndMap;

		std::ofstream fout(asset->Path);
		fout << out.c_str();
		fout.close();
	}

	bool MaterialLoader::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
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
		YAML::Node materialNode = root["material"];

		asset = CreateRef<Material>();
		Ref<Material> mat = std::dynamic_pointer_cast<Material>(asset);

		mat->SetName(materialNode["name"].as<std::string>());
		LP_DESERIALIZE_PROPERTY(handle, asset->Handle, materialNode, AssetHandle(0));
		mat->SetShader(ShaderLibrary::GetShader(materialNode["shader"].as<std::string>()));

		YAML::Node textureNode = materialNode["textures"];
		
		for (auto& texName : mat->GetShader()->GetSpecifications().TextureNames)
		{
			AssetHandle textureHandle = textureNode[texName].as<AssetHandle>();
			mat->SetTexture(texName, ResourceCache::GetAsset<Texture2D>(g_pEnv->pAssetManager->GetPathFromAssetHandle(textureHandle)));
		}

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
			LP_SERIALIZE_PROPERTY(sourcePath, mesh->m_sourceMesh.string(), out);
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

		LP_DESERIALIZE_PROPERTY(handle, asset->Handle, meshNode, AssetHandle(0));
		meshSource->m_sourceMesh = std::filesystem::path(meshNode["sourcePath"].as<std::string>());
		if (!std::filesystem::exists(meshSource->m_sourceMesh))
		{
			LP_CORE_ERROR("Mesh {0} not found!", meshSource->m_sourceMesh.string());
			meshSource->SetFlag(AssetFlag::Missing, true);

			return false;
		}

		LP_DESERIALIZE_PROPERTY(meshHandle, meshSource->m_mesh, meshNode, AssetHandle(0));
		meshSource->Path = path;

		return true;
	}
}