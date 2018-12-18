#pragma once

#include "component_registry.h"
#include "entity_manager.h"

namespace iwecs {
	class ecs_manager {
	private:
		component_registry m_component_reg;
		entity_manager m_entity_mgr;
	public:
		template<
			typename... _components_t>
		entity_t create_entity(
			const _components_t&... args)
		{
			std::size_t component_index
				= m_component_reg.create<_components_t...>(args...);

			return add_entity(component_index);
		}

		template<
			typename... _components_t>
		entity_t create_entity(
			_components_t&&... args) 
		{
			std::size_t component_index
				= m_component_reg.create<_components_t...>(
					std::forward<_components_t>(args)...);

			return add_entity(component_index);
		}

		template<
			typename... _components_t>
		entity_t create_entity(
			_components_t&... args)
		{
			return create_entity(std::forward<_components_t>(args)...);
		}

		void destroy_entity(
			entity_t entity)
		{
			entity_data data = m_entity_mgr.get_entity_data(entity);
			m_component_reg.destroy(data.archetype_id, data.component_index);
		}

		template<
			typename... _components_t>
		/* component_view */ void view_components() {
			return m_component_reg.view_components<_components_t...>();
		}
	private:
		template<
			typename _components_t>
		entity_t add_entity(
			std::size_t component_index)
		{
			std::size_t id = iwutil::archetype<_components_t...>::id;
			return m_entity_mgr.add_entity(id, component_index);
		}
	};
}