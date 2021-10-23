#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Lamp/Mesh/Mesh.h"
#include "Lamp/Objects/Object.h"

namespace Lamp
{
class Brush : public Object
{
public:
    Brush(Ref<Mesh> model);

    virtual void OnEvent(Event& e) override;
    virtual void Destroy() override;

    //Getting
    inline const Ref<Mesh>& GetModel() const {
        return m_Mesh;
    }

public:
    static Brush* Create(const std::filesystem::path& path);
    static Brush* Create(const std::filesystem::path& path, const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale, uint32_t layerId, const std::string& name);
    static Brush* Duplicate(Brush* main, bool addToLevel);
    static Brush* Get(uint32_t id);

private:
    bool OnRender(AppRenderEvent& e);
    bool OnUpdate(AppUpdateEvent& e);
    bool OnScaleChanged(ObjectScaleChangedEvent& e);

private:
    Ref<Mesh> m_Mesh;
};
}