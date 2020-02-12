#include "iw/entity/ChunkList.h"
#include <assert.h>
#include "iw/log/logger.h"

namespace iw {
namespace ECS {
	using iterator = ChunkList::iterator;

	// replace with free list like thing but for valid entities 
	iterator& iterator::operator++() {
		do {
			m_index++;
			if (m_index == m_chunk->EndIndex()) {
				do {
					m_chunk = m_chunk->Next;
					if (m_chunk) {
						m_index = m_chunk->BeginIndex();
					}
				} while (m_chunk && m_chunk->Count == 0);
			}
		} while (m_chunk && !m_chunk->GetEntity(m_index)->Alive);

		return *this;
	}

	bool iterator::operator==(
		const iterator& itr) const
	{
		return this->m_chunk == itr.m_chunk
			&& this->m_index == itr.m_index;
	}

	bool iterator::operator!=(
		const iterator& itr) const
	{
		return !operator==(itr);
	}

	EntityComponentData iterator::operator*() {
		for (size_t i = 0; i < m_indices->Count; i++) {
			m_data->Components[i] = m_chunk->GetComponentPtr(
				m_archetype->Layout[m_indices->Indices[i]], 
				m_index
			);
		}

		EntityHandle* entity = m_chunk->GetEntity(m_index);

		return EntityComponentData { entity->Index, entity->Version, *m_data };
	}

	iterator::iterator(
		Chunk* chunk,
		size_t index,
		const iw::ref<Archetype>& archetype,
		const iw::ref<ComponentQuery>& query,
		iw::pool_allocator& componentPool)
		: m_chunk(chunk)
		, m_index(index)
		, m_archetype(archetype)
	{
		size_t cdSize = sizeof(ComponentData)
			+ sizeof(size_t)
			* query->Count;

		m_data = componentPool.alloc_ref_t<ComponentData>(cdSize);
		
		size_t cdisSize = sizeof(ComponentDataIndices)
			+ sizeof(size_t)
			* query->Count;

		m_indices = componentPool.alloc_ref_t<ComponentDataIndices>(cdisSize);
	
		m_indices->Count = query->Count;
		for (size_t i = 0; i < query->Count; i++) {
			for (size_t j = 0; j < archetype->Count; j++) {
				if (query->Components[i]->Type == archetype->Layout[j].Component->Type) {
					m_indices->Indices[i] = j;
					break;
				}
			}
		}
	}

	ChunkList::ChunkList(
		const iw::ref<Archetype>& archetype,
		size_t chunkSize,
		iw::pool_allocator& componentPool,
		iw::pool_allocator& chunkPool)
		: m_root(nullptr)
		, m_count(0)
		, m_chunkCount(0)
		, m_archetype(archetype)
		, m_chunkSize(chunkSize)
		, m_chunkCapacity(GetChunkCapacity(archetype))
		, m_componentPool(componentPool)
		, m_chunkPool(chunkPool)
	{}

	size_t ChunkList::ReserveComponents(
		const EntityHandle& entity)
	{
		Chunk& chunk = FindOrCreateChunk();
		size_t index = chunk.ReserveComponents();

		EntityHandle* entityComponent = chunk.GetEntity(index);
		*entityComponent = entity;

		++m_count;

		return index;
	}

	bool ChunkList::ReinstateComponents(
		const iw::ref<EntityData>& entityData)
	{
		Chunk* chunk = FindChunk(entityData->ChunkIndex);
		if (chunk) {
			chunk->ReinstateComponents();

			EntityHandle* entityComponent = chunk->GetEntity(entityData->ChunkIndex);
			*entityComponent = entityData->Entity;

			for (size_t i = 0; i < m_archetype->Count; i++) {
				ArchetypeLayout& layout = m_archetype->Layout[i];

				void* old = chunk->GetComponentPtr(layout, entityData->ChunkIndex);
				memset(old, 0, layout.Component->Size); // could put this in Chunk.h if it sounds better
			}

			++m_count;

			return true;
		}

		return false;
	}

	bool ChunkList::FreeComponents(
		size_t index)
	{
		Chunk* chunk = FindChunk(index);
		if (chunk) {
			chunk->FreeComponents();

			EntityHandle* entityComponent = chunk->GetEntity(index);
			entityComponent->Alive = false;

			--m_count;

			// If chunk is empty free it
			if (chunk->Count == 0) {			
				if (chunk->Previous) {
					chunk->Previous->Next = chunk->Next;
				}

				if (chunk->Next) {
					chunk->Next->Previous = chunk->Previous;
				}

				if (chunk == m_root) {
					m_root = m_root->Next;
				}

				LOG_DEBUG << "Deleting Chunk " << chunk->IndexOffset / chunk->Capacity;

				--m_chunkCount;
				free(chunk);
			}

			return true;
		}

		return false;
	}

	bool ChunkList::MoveComponents(
		ChunkList& to,
		size_t index,
		size_t newIndex)
	{
		Chunk* chunk = FindChunk(index);
		if (chunk) {
			for (size_t i = 0; i < m_archetype->Count; i++) {
				ArchetypeLayout& layout = m_archetype->Layout[i];
				iw::ref<Component> component = layout.Component;

				void* ptr  = to.GetComponentPtr(component, newIndex);
				if (!ptr) continue;

				void* from = chunk->GetComponentPtr(layout, index);
				if (!from) continue; // not sure if this can ever trip

				memmove(ptr, from, component->Size); // could put this in Chunk.h if it sounds better
			}

			return true;
		}

		return false;
	}

	void* ChunkList::GetComponentPtr(
		const iw::ref<Component>& component,
		size_t index)
	{
		Chunk* chunk = FindChunk(index);
		if (chunk) {
			size_t i = 0;
			for (; i < m_archetype->Count; i++) {
				if (component->Type == m_archetype->Layout[i].Component->Type) {
					break;
				}
			}

			if (i == m_archetype->Count) {
				return nullptr;
			}

			return chunk->GetComponentPtr(m_archetype->Layout[i], index);
		}

		return nullptr;
	}

	EntityHandle* ChunkList::GetEntity(
		size_t index)
	{
		Chunk* chunk = FindChunk(index);
		if (chunk) {
			return chunk->GetEntity(index);
		}

		return nullptr;
	}

	int ChunkList::IndexOf(
		const iw::ref<Component>& component,
		void* instance)
	{
		int index = -1;
		Chunk* chunk = m_root;
		ArchetypeLayout& layout = *m_archetype->GetLayout(component);
		while (chunk) {
			index = chunk->IndexOf(layout, instance);
			if (index == -1) {
				chunk = chunk->Next;
			}

			else {
				break;
			}
		}

		return index;
	}

	iterator ChunkList::Begin(
		const iw::ref<ComponentQuery>& query)
	{
		Chunk* chunk = m_root;
		size_t index = 0;
		while (chunk && chunk->Count == 0) {
			chunk = chunk->Next;
		}
		
		if (chunk) {
			index = chunk->BeginIndex();
		}

		return iterator(m_root, index, m_archetype, query, m_componentPool);
	}

	iterator ChunkList::End(
		const iw::ref<ComponentQuery>& query)
	{
		Chunk* chunk = m_root;
		size_t index = 0;
		while (chunk && chunk->Next) {
			chunk = chunk->Next;
		}

		if (chunk) {
			index = chunk->EndIndex();
		}

		return iterator(nullptr, index, m_archetype, query, m_componentPool);
	}

	Chunk* ChunkList::FindChunk(
		size_t index)
	{
		Chunk* chunk = m_root;
		while (chunk && !chunk->ContainsIndex(index)) {
			chunk = chunk->Next;
		}

		return chunk;
	}

	Chunk* ChunkList::CreateChunk() {
		Chunk* chunk = m_chunkPool.alloc_t<Chunk>(m_chunkSize);

		chunk->IndexOffset = m_chunkCapacity * m_chunkCount;
		chunk->Capacity    = m_chunkCapacity;

		LOG_DEBUG << "Creating Chunk " << chunk->IndexOffset / chunk->Capacity;

		++m_chunkCount;

		return chunk;
	}

	Chunk& ChunkList::FindOrCreateChunk() {
		if (!m_root) {
			m_root = CreateChunk();
		}

		Chunk* chunk = m_root;
		while (chunk && chunk->CurrentIndex == chunk->Capacity) {
			if (chunk->Next == nullptr) {
				chunk->Next = CreateChunk();
				chunk->Next->Previous = chunk;
			}

			chunk = chunk->Next;
		}

		return *chunk;
	}

	size_t ChunkList::GetChunkCapacity(
		const iw::ref<Archetype>& archetype)
	{
		size_t archetypeSize = archetype->Size + sizeof(EntityHandle);
		size_t bufSize       = m_chunkSize     - sizeof(Chunk);
		size_t padSize       = bufSize % archetypeSize;

		return (bufSize - padSize) / archetypeSize;
	}
}
}
