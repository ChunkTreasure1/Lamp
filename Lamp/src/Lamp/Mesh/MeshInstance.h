#pragma once

namespace Lamp
{
	class Mesh;
	class MaterialInstance;
	class MeshInstance
	{
	public:
		MeshInstance(Ref<Mesh> mesh);

		void Render(const glm::mat4& transform, uint32_t id = -1);
		inline const Ref<Mesh> GetSharedMesh() const { return m_sharedMesh; }

		static Ref<MeshInstance> Create(Ref<Mesh> mesh);

	private:
		Ref<Mesh> m_sharedMesh;
		std::map<uint32_t, Ref<MaterialInstance>> m_materials;
	};
}