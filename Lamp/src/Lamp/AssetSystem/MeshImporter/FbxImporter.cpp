#include "lppch.h"
#include "FbxImporter.h"

#include "Lamp/Mesh/SubMesh.h"
#include "Lamp/Mesh/Materials/Material.h"
#include "Lamp/Mesh/Mesh.h"

#include "Lamp/Rendering/RenderPipelineLibrary.h"

namespace Lamp
{
	namespace Utility
	{
		FbxSystemUnit MeshUnitToSystemUnit(MeshUnit unit)
		{
			switch (unit)
			{
				case MeshUnit::Centimeters: return FbxSystemUnit::cm;
				case MeshUnit::Decimeters: return FbxSystemUnit::dm;
				case MeshUnit::Meters: return FbxSystemUnit::m;
			}
			
			return FbxSystemUnit::m;
		}
	}
	
	Ref<Mesh> FbxImporter::ImportMeshImpl(const MeshImportSettings& settings)
	{
		std::map<uint32_t, Ref<Material>> materials;
		std::vector<Ref<SubMesh>> meshes = LoadMesh(settings, materials);

		if (meshes.empty())
		{
			LP_CORE_WARN("Failed to load mesh or no meshes found at: {0}!", settings.path.string());

			Ref<Mesh> asset = CreateRef<Mesh>();
			asset->SetFlag(AssetFlag::Invalid);
			
			return asset;
		}

		Ref<Mesh> mesh = CreateRef<Mesh>(settings.path.stem().string(), meshes, materials);
		return mesh;
	}

	void FbxImporter::FetchGeometryNodes(FbxNode* node, std::vector<FbxNode*>& outNodes)
	{
		if (node->GetNodeAttribute())
		{
			switch (node->GetNodeAttribute()->GetAttributeType())
			{
				case FbxNodeAttribute::eMesh:
				{
					if (node->GetMesh())
					{
						outNodes.emplace_back(node);
					}
				}
			}
		}

		for (uint32_t i = 0; i < (uint32_t)node->GetChildCount(); i++)
		{
			FetchGeometryNodes(node->GetChild(i), outNodes);
		}
	}

	std::vector<Ref<SubMesh>> FbxImporter::LoadMesh(const MeshImportSettings& settings, std::map<uint32_t, Ref<Material>>& materials)
	{
		std::vector<Ref<SubMesh>> subMeshes;

		FbxManager* sdkManager = FbxManager::Create();
		FbxIOSettings* ioSettings = FbxIOSettings::Create(sdkManager, IOSROOT);

		ioSettings->SetBoolProp(IMP_FBX_MATERIAL, true);
		ioSettings->SetBoolProp(IMP_FBX_TEXTURE, false);
		ioSettings->SetBoolProp(IMP_FBX_LINK, false);
		ioSettings->SetBoolProp(IMP_FBX_SHAPE, false);
		ioSettings->SetBoolProp(IMP_FBX_GOBO, false);
		ioSettings->SetBoolProp(IMP_FBX_ANIMATION, false);
		ioSettings->SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);

		sdkManager->SetIOSettings(ioSettings);

		fbxsdk::FbxImporter* importer = fbxsdk::FbxImporter::Create(sdkManager, "");

		bool importStatus = importer->Initialize(settings.path.string().c_str(), -1, sdkManager->GetIOSettings());
		if (!importStatus)
		{
			LP_CORE_ERROR("Unable to import file {0}!", settings.path.string());
			return subMeshes;
		}

		FbxScene* fbxScene = FbxScene::Create(sdkManager, "Scene");

		fbxsdk::FbxAxisSystem axisSystem(fbxsdk::FbxAxisSystem::eOpenGL);
		fbxsdk::FbxSystemUnit units = Utility::MeshUnitToSystemUnit(settings.units);
		
		importer->Import(fbxScene);
		axisSystem.DeepConvertScene(fbxScene);
		units.ConvertScene(fbxScene);

		FbxNode* rootNode = fbxScene->GetRootNode();

		std::vector<FbxNode*> geomNodes;
		FetchGeometryNodes(rootNode, geomNodes);

		for (auto node : geomNodes)
		{
			FbxGeometryConverter geomConverter(sdkManager);
			if (!geomConverter.Triangulate(node->GetNodeAttribute(), true, true))
			{
				geomConverter.Triangulate(node->GetNodeAttribute(), true, false);
			}

			Ref<SubMesh> subMesh = ProcessMesh(node->GetMesh());
			if (subMesh)
			{
				subMeshes.emplace_back(subMesh);
			}
		}

		for (auto subMesh : subMeshes)
		{
			if (materials.find(subMesh->GetMaterialIndex()) == materials.end())
			{
				std::string name = "Default";
				if (auto material = fbxScene->GetMaterial(subMesh->GetMaterialIndex()))
				{
					name = material->GetName();
				}
				materials.emplace(subMesh->GetMaterialIndex(), Material::Create(name, subMesh->GetMaterialIndex(), RenderPipelineLibrary::GetPipeline("Deferred")));
			}
		}

		importer->Destroy();

		return subMeshes;
	}

	Ref<SubMesh> FbxImporter::ProcessMesh(FbxMesh* mesh)
	{
		if (!mesh)
		{
			return nullptr;
		}

		if (mesh->GetElementBinormalCount() == 0 || mesh->GetElementTangentCount() == 0)
		{
			bool result = mesh->GenerateTangentsData(0, true, false);
			LP_CORE_ASSERT(result, "Unable to generate tangent data!");
		}

		const FbxVector4* ctrlPoints = mesh->GetControlPoints();
		const uint32_t triangleCount = mesh->GetPolygonCount();
		uint32_t vertexCount = 0;

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		vertices.reserve(triangleCount * 3);
		indices.reserve(triangleCount);

		for (uint32_t i = 0; i < triangleCount; i++)
		{
			const int32_t polySize = mesh->GetPolygonSize(i);
			LP_CORE_ASSERT(polySize == 3, "Mesh must be fully triangulated!");

			for (int32_t j = 0; j < polySize; j++)
			{
				Vertex vertex;
				const int32_t ctrlPointIndex = mesh->GetPolygonVertex(i, j);

				vertex.position.x = (float)ctrlPoints[ctrlPointIndex][0];
				vertex.position.y = (float)ctrlPoints[ctrlPointIndex][1];
				vertex.position.z = (float)ctrlPoints[ctrlPointIndex][2];

				const int32_t numUVs = mesh->GetElementUVCount();
				const int32_t texUvIndex = mesh->GetTextureUVIndex(i, j);

				// TODO: Add ability to have multiple UV sets
				for (int32_t uv = 0; uv < numUVs && uv < 4; uv++)
				{
					FbxGeometryElementUV* uvElement = mesh->GetElementUV(uv);
					const auto coord = uvElement->GetDirectArray().GetAt(texUvIndex);

					vertex.textureCoords.x = (float)coord.mData[0];
					vertex.textureCoords.y = (float)coord.mData[1];

					break;
				}

				ReadNormal(mesh, ctrlPointIndex, vertexCount, vertex.normal);
				ReadTangent(mesh, ctrlPointIndex, vertexCount, vertex.tangent);
				ReadBitangent(mesh, ctrlPointIndex, vertexCount, vertex.bitangent);

				vertices.emplace_back(vertex);
				indices.emplace_back(vertices.size() - 1);
				vertexCount++;
			}
		}

		Optimize(vertices, indices);

		int32_t index = 0;
		auto mat = mesh->GetElementMaterial();
		if (mat)
		{
			index = mat->GetIndexArray()[0];
		}

		if (!vertices.empty() && !indices.empty())
		{
			return CreateRef<SubMesh>(vertices, indices, index, mesh->GetName());
		}

		return nullptr;
	}

	void FbxImporter::ReadNormal(FbxMesh* mesh, int32_t ctrlPointIndex, int32_t vertCount, glm::vec3& normal)
	{
		if (mesh->GetElementNormalCount() < 1)
		{
			return;
		}

		FbxGeometryElementNormal* fbxNormal = mesh->GetElementNormal(0);

		switch (fbxNormal->GetMappingMode())
		{
			case FbxGeometryElement::eByControlPoint:
			{
				switch (fbxNormal->GetReferenceMode())
				{
					case FbxGeometryElement::eDirect:
					{
						normal.x = (float)fbxNormal->GetDirectArray().GetAt(ctrlPointIndex)[0];
						normal.y = (float)fbxNormal->GetDirectArray().GetAt(ctrlPointIndex)[1];
						normal.z = (float)fbxNormal->GetDirectArray().GetAt(ctrlPointIndex)[2];

						break;
					}

					case FbxGeometryElement::eIndexToDirect:
					{
						int32_t id = fbxNormal->GetIndexArray().GetAt(ctrlPointIndex);
						normal.x = (float)fbxNormal->GetDirectArray().GetAt(id)[0];
						normal.y = (float)fbxNormal->GetDirectArray().GetAt(id)[1];
						normal.z = (float)fbxNormal->GetDirectArray().GetAt(id)[2];

						break;
					}

					default:
						break;
				}

				break;
			}

			case FbxGeometryElement::eByPolygonVertex:
			{
				switch (fbxNormal->GetReferenceMode())
				{
					case FbxGeometryElement::eDirect:
					{
						normal.x = (float)fbxNormal->GetDirectArray().GetAt(vertCount)[0];
						normal.y = (float)fbxNormal->GetDirectArray().GetAt(vertCount)[1];
						normal.z = (float)fbxNormal->GetDirectArray().GetAt(vertCount)[2];
						break;
					}

					case FbxGeometryElement::eIndexToDirect:
					{
						int32_t id = fbxNormal->GetIndexArray().GetAt(vertCount);
						normal.x = (float)fbxNormal->GetDirectArray().GetAt(id)[0];
						normal.y = (float)fbxNormal->GetDirectArray().GetAt(id)[1];
						normal.z = (float)fbxNormal->GetDirectArray().GetAt(id)[2];

						break;
					}

					default:
						break;
				}

				break;
			}

			break;
		}
	}

	void FbxImporter::ReadTangent(FbxMesh* mesh, int32_t ctrlPointIndex, int32_t vertCount, glm::vec3& tangent)
	{
		if (mesh->GetElementTangentCount() < 1)
		{
			return;
		}

		FbxGeometryElementTangent* fbxTangent = mesh->GetElementTangent(0);

		switch (fbxTangent->GetMappingMode())
		{
			case FbxGeometryElement::eByControlPoint:
			{
				switch (fbxTangent->GetReferenceMode())
				{
					case FbxGeometryElement::eDirect:
					{
						tangent.x = (float)fbxTangent->GetDirectArray().GetAt(ctrlPointIndex)[0];
						tangent.y = (float)fbxTangent->GetDirectArray().GetAt(ctrlPointIndex)[1];
						tangent.z = (float)fbxTangent->GetDirectArray().GetAt(ctrlPointIndex)[2];

						break;
					}

					case FbxGeometryElement::eIndexToDirect:
					{
						int32_t id = fbxTangent->GetIndexArray().GetAt(ctrlPointIndex);
						tangent.x = (float)fbxTangent->GetDirectArray().GetAt(id)[0];
						tangent.y = (float)fbxTangent->GetDirectArray().GetAt(id)[1];
						tangent.z = (float)fbxTangent->GetDirectArray().GetAt(id)[2];

						break;
					}

					default:
						break;
				}

				break;
			}

			case FbxGeometryElement::eByPolygonVertex:
			{
				switch (fbxTangent->GetReferenceMode())
				{
					case FbxGeometryElement::eDirect:
					{
						tangent.x = (float)fbxTangent->GetDirectArray().GetAt(vertCount)[0];
						tangent.y = (float)fbxTangent->GetDirectArray().GetAt(vertCount)[1];
						tangent.z = (float)fbxTangent->GetDirectArray().GetAt(vertCount)[2];
						break;
					}

					case FbxGeometryElement::eIndexToDirect:
					{
						int32_t id = fbxTangent->GetIndexArray().GetAt(vertCount);
						tangent.x = (float)fbxTangent->GetDirectArray().GetAt(id)[0];
						tangent.y = (float)fbxTangent->GetDirectArray().GetAt(id)[1];
						tangent.z = (float)fbxTangent->GetDirectArray().GetAt(id)[2];

						break;
					}

					default:
						break;
				}

				break;
			}

			break;
		}
	}

	void FbxImporter::ReadBitangent(FbxMesh* mesh, int32_t ctrlPointIndex, int32_t vertCount, glm::vec3& bitangent)
	{
		if (mesh->GetElementBinormalCount() < 1)
		{
			return;
		}

		FbxGeometryElementBinormal* fbxBitangent = mesh->GetElementBinormal(0);

		switch (fbxBitangent->GetMappingMode())
		{
			case FbxGeometryElement::eByControlPoint:
			{
				switch (fbxBitangent->GetReferenceMode())
				{
					case FbxGeometryElement::eDirect:
					{
						bitangent.x = (float)fbxBitangent->GetDirectArray().GetAt(ctrlPointIndex)[0];
						bitangent.y = (float)fbxBitangent->GetDirectArray().GetAt(ctrlPointIndex)[1];
						bitangent.z = (float)fbxBitangent->GetDirectArray().GetAt(ctrlPointIndex)[2];

						break;
					}

					case FbxGeometryElement::eIndexToDirect:
					{
						int32_t id = fbxBitangent->GetIndexArray().GetAt(ctrlPointIndex);
						bitangent.x = (float)fbxBitangent->GetDirectArray().GetAt(id)[0];
						bitangent.y = (float)fbxBitangent->GetDirectArray().GetAt(id)[1];
						bitangent.z = (float)fbxBitangent->GetDirectArray().GetAt(id)[2];

						break;
					}

					default:
						break;
				}

				break;
			}

			case FbxGeometryElement::eByPolygonVertex:
			{
				switch (fbxBitangent->GetReferenceMode())
				{
					case FbxGeometryElement::eDirect:
					{
						bitangent.x = (float)fbxBitangent->GetDirectArray().GetAt(vertCount)[0];
						bitangent.y = (float)fbxBitangent->GetDirectArray().GetAt(vertCount)[1];
						bitangent.z = (float)fbxBitangent->GetDirectArray().GetAt(vertCount)[2];
						break;
					}

					case FbxGeometryElement::eIndexToDirect:
					{
						int32_t id = fbxBitangent->GetIndexArray().GetAt(vertCount);
						bitangent.x = (float)fbxBitangent->GetDirectArray().GetAt(id)[0];
						bitangent.y = (float)fbxBitangent->GetDirectArray().GetAt(id)[1];
						bitangent.z = (float)fbxBitangent->GetDirectArray().GetAt(id)[2];

						break;
					}

					default:
						break;
				}

				break;
			}

			break;
		}
	}

	void FbxImporter::Optimize(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
	{
		std::vector<Vertex> uniqueVerts;
		std::vector<uint32_t> uniqueIndices;

		for (uint32_t i = 0; i < vertices.size(); i++)
		{
			if (FindVertex(vertices[i], uniqueVerts) == -1)
			{
				uniqueVerts.emplace_back(vertices[i]);
			}
		}

		for (uint32_t i = 0; i < vertices.size(); i++)
		{
			uniqueIndices.emplace_back(FindVertex(vertices[i], uniqueVerts));
		}

		vertices = uniqueVerts;
		indices = uniqueIndices;
	}

	int32_t FbxImporter::FindVertex(const Vertex& aVert, const std::vector<Vertex>& aUniques)
	{
		for (uint32_t i = 0; i < aUniques.size(); i++)
		{
			if (aVert == aUniques[i])
			{
				return i;
			}
		}

		return -1;
	}
}