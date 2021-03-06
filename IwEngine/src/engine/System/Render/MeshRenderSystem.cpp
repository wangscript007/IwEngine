#include "iw/engine/Systems/Render/MeshRenderSystem.h"
#include "iw/engine/Components/UiElement_temp.h"

namespace iw {
namespace Engine {
	MeshRenderSystem::MeshRenderSystem(
		Scene* scene)
		: System<Transform, Mesh>("Mesh Render")
		, m_scene(scene)
	{}

	void MeshRenderSystem::Update(
		EntityComponentArray& eca)
	{
		Renderer->BeginScene(m_scene);
		
		for (auto entity : eca) {
			if (Space->HasComponent<iw::UiElement>(entity.Handle)) {
				continue;
			}

			auto [transform, mesh] = entity.Components.Tie<Components>();
			
			Renderer->DrawMesh(transform, mesh);
		}
			
		Renderer->EndScene(); 
	}
}
}
