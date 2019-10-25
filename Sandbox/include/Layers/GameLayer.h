#pragma once

#include "iw/engine/Layer.h"
#include "iw/entity/Space.h"
#include "iw/graphics/RenderQueue.h"
#include "iw/graphics/Loaders/ModelLoader.h"
#include "iw/renderer/Device.h"
#include "iw/renderer/Pipeline.h"
#include "iw/graphics/Material.h"

class GameLayer
	: public IwEngine::Layer
{
private: // tmp
	IW::ModelLoader loader;
	IW::IPipeline* pipeline;  
	IW::IPipeline* pipeline_line;

	IW::Mesh* line;

public:
	GameLayer(
		IwEntity::Space& space,
		IW::Graphics::Renderer& renderer);

	int Initialize(
		IwEngine::InitOptions& options) override;

	void PostUpdate()  override;
	void FixedUpdate() override;
	void ImGui()       override;
};
