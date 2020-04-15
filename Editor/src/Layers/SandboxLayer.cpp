#include "Layers/SandboxLayer.h"
#include "Systems/BulletSystem.h"
#include "Systems/LevelSystem.h"
#include "Systems/EnemyDeathCircleSystem.h"
#include "Systems/GameCameraController.h"
#include "Systems/ScoreSystem.h"
#include "Events/ActionEvents.h"
#include "iw/engine/Systems/PhysicsSystem.h"
#include "iw/engine/Components/CameraController.h"
#include "iw/engine/Time.h"
#include "iw/physics/Collision/SphereCollider.h"
#include "iw/physics/Collision/PlaneCollider.h"
#include "iw/physics/Dynamics/SmoothPositionSolver.h"
#include "iw/physics/Dynamics/ImpulseSolver.h"
#include "iw/physics/Dynamics/Rigidbody.h"
#include "iw/graphics/MeshFactory.h"
#include "iw/graphics/Font.h"
#include "iw/graphics/Model.h"
#include "iw/graphics/TextureAtlas.h"
#include "iw/input/Devices/Mouse.h"
#include "iw/audio/AudioSpaceStudio.h"
#include "imgui/imgui.h"

#include "iw/engine/Systems/Debug/DrawCollidersSystem.h"


namespace iw {
	struct ModelUBO {
		matrix4 model;
		vector3 albedo;
	};

	struct ModelComponents {
		Transform* Transform;
		Model* Model;
	};

	struct CameraComponents {
		Transform* Transform;
		CameraController* Controller;
	};

	struct PlaneComponents {
		Transform* Transform;
		PlaneCollider* Collider;
	};

	SandboxLayer::SandboxLayer()
		: Layer("Sandbox")
		, playerSystem(nullptr)
		, light(nullptr)
		, sun(nullptr)
		, textCam(nullptr)
		, textMesh(nullptr)
		, scene(nullptr)
	{}

	int forestInstance = 0;

	iw::matrix4 persp = iw::matrix4::create_perspective_field_of_view(1.17f, 1.778f, 1.0f, 500.0f);
	iw::matrix4 ortho = iw::matrix4::create_orthographic(16 * 4, 9 * 4, -100, 100);
	float blend;

	int SandboxLayer::Initialize() {
		vector3 verts[] = {
			vector3(0, 0, 0),
			vector3(1, 0, 0),
			vector3(0, 1, 0),
			vector3(1, 1, 0)
		};

		vector2 uvs[] = {
			vector2(0, 0),
			vector2(1, 0),
			vector2(0, 1),
			vector2(1, 1)
		};

		unsigned indices[] = {
			0, 1, 2,
			1, 3, 2
		};
		
		VertexBufferLayout f3;
		f3.Push<float>(3);

		VertexBufferLayout f2;
		f2.Push<float>(2);

		MeshDescription description;
		description.DescribeBuffer(bName::POSITION, f3);
		description.DescribeBuffer(bName::UV,       f2);

		MeshData data(description);

		data.SetBufferData(0, 4, verts);
		data.SetBufferData(1, 4, uvs);
		data.SetIndexData(6, indices);
		data.SetTopology(MeshTopology::TRIANGLES);

		Mesh mesh = data.MakeInstance();

		AudioSpaceStudio* studio = (AudioSpaceStudio*)Audio->AsStudio();

		studio->SetVolume(0.25f);

		studio->LoadMasterBank("Master.bank");
		studio->LoadMasterBank("Master.strings.bank");
		studio->CreateEvent("swordAttack");
		studio->CreateEvent("forestAmbiance");
		studio->CreateEvent("playerDamaged");
		studio->CreateEvent("enemyDeath");

		forestInstance = studio->CreateInstance("forestAmbiance", false);

		// Fonts

		font = Asset->Load<Font>("fonts/arial.fnt");
		font->Initialize(Renderer->Device);

		textMesh = font->GenerateMesh("Use arrow keys to move and x to attack.\ni: debug menu\nt: freecam", .005f, 1);

		fontShader = Asset->Load<Shader>("shaders/font.shader");
		Renderer->InitShader(fontShader, CAMERA);

		ref<Material> textMat = REF<Material>(fontShader);

		textMat->Set("color", vector3::one);
		textMat->SetTexture("fontMap", font->GetTexture(0));

		textMesh->SetMaterial(textMat);
		textMesh->Initialize(Renderer->Device);

		textTransform = Transform(vector3(-6.8, -1.8, 0), vector3::one, quaternion::identity);

		// Shader

		ref<Shader> shader   = Asset->Load<Shader>("shaders/pbr.shader");
		ref<Shader> gaussian = Asset->Load<Shader>("shaders/filters/gaussian.shader");
		ref<Shader> dirShadowShader   = Asset->Load<Shader>("shaders/lights/directional.shader");
		ref<Shader> pointShadowShader = Asset->Load<Shader>("shaders/lights/point.shader");
		
		Renderer->InitShader(shader,   CAMERA | SHADOWS | LIGHTS);
		Renderer->InitShader(gaussian, CAMERA);
		Renderer->InitShader(dirShadowShader, CAMERA);
		Renderer->InitShader(pointShadowShader);

		// Directional light shadow map textures & target

		ref<Texture> dirShadowColor = ref<Texture>(new Texture(1024, 1024, TEX_2D, RG, FLOAT, BORDER));
		ref<Texture> dirShadowDepth = ref<Texture>(new Texture(1024, 1024, TEX_2D, DEPTH, FLOAT, BORDER));

		ref<RenderTarget> dirShadowTarget = REF<RenderTarget>(1024, 1024);
		dirShadowTarget->AddTexture(dirShadowColor);
		dirShadowTarget->AddTexture(dirShadowDepth);

		dirShadowTarget->Initialize(Renderer->Device);

		//ref<Texture> texBlur = REF<Texture>(1024, 1024, TEX_2D, ALPHA, FLOAT, BORDER);

		//ref<RenderTarget> targetBlur = REF<RenderTarget>(1024, 1024);
		//targetBlur->AddTexture(texBlur);
		//targetBlur->Initialize(Renderer->Device);

		// Point light shadow map textures & target

		ref<Texture> pointShadowDepth = ref<Texture>(new Texture(1024, 1024, TEX_CUBE, DEPTH, FLOAT, EDGE));

		ref<RenderTarget> pointShadowTarget = REF<RenderTarget>(1024, 1024, true);
		pointShadowTarget->AddTexture(pointShadowDepth);

		pointShadowTarget->Initialize(Renderer->Device);

		Asset->Give<iw::Texture>("SunShadowMap",   dirShadowTarget->Tex(0));
		Asset->Give<iw::Texture>("LightShadowMap", pointShadowTarget->Tex(0));

		// Scene 

		scene = new Scene();
		
		//	Lights

		sun   = new DirectionalLight(100, OrthographicCamera(60, 32, -100, 100), dirShadowShader, dirShadowTarget);
		light = new PointLight(30, 30, pointShadowShader, pointShadowTarget);

		sun->SetRotation(quaternion(0.872f, 0.0f, 0.303f, 0.384f));
		light->SetPosition(vector3(0, 10, 0));

		scene->AddLight(sun);
		scene->AddLight(light);

		//	Cameras

		scene->SetMainCamera(new PerspectiveCamera()); // projection from up top
		textCam = new OrthographicCamera(vector3::one, quaternion::from_axis_angle(vector3::unit_y, Pi), 16, 9, -10, 10);

		iw::quaternion camrot = 
			  iw::quaternion::from_axis_angle(iw::vector3::unit_x, iw::Pi / 2)
			* iw::quaternion::from_axis_angle(iw::vector3::unit_z, iw::Pi);

		iw::Entity camera = Space->CreateEntity<iw::Transform, iw::CameraController>();

		iw::Transform* transform = camera.SetComponent<iw::Transform>(vector3(0, 27.18f, 0), iw::vector3::one, camrot);
		                           camera.SetComponent<iw::CameraController>(scene->MainCamera());

		scene->MainCamera()->SetTrans(transform);

		// Materials

		ref<Material> smat = REF<Material>();
		ref<Material> tmat = REF<Material>();

		smat->SetShader(shader);
		tmat->SetShader(shader);

		smat->Set("albedo", vector4(1, 1, 1, 1));
		tmat->Set("albedo", vector4(1, 1, 1, 1));

		smat->Set("roughness", 0.8f);
		tmat->Set("roughness", 0.8f);
		
		smat->Set("metallic", 0.2f);
		tmat->Set("metallic", 0.2f);

		smat->SetTexture("shadowMap",  dirShadowTarget->Tex(0));    // shouldnt really be part of material
		tmat->SetTexture("shadowMap",  dirShadowTarget->Tex(0));

		smat->SetTexture("shadowMap2", pointShadowTarget->Tex(0));
		tmat->SetTexture("shadowMap2", pointShadowTarget->Tex(0));

		smat->Initialize(Renderer->Device);
		tmat->Initialize(Renderer->Device);

		// Models

		Mesh* smesh = MakeUvSphere(25, 30);
		Mesh* tmesh = MakeTetrahedron(5);

		smesh->SetMaterial(smat);
		tmesh->SetMaterial(smat);

		smesh->GenTangents();
		tmesh->GenTangents();

		smesh->Initialize(Renderer->Device);
		tmesh->Initialize(Renderer->Device);

		Model sm { smesh, 1 };
		Model tm { tmesh, 1 };

		Asset->Give<Model>("Sphere", &sm);
		Asset->Give<Model>("Tetrahedron", &tm);

		//	Player

		Model pm { smesh, 1 };
		pm.Meshes[0].Material = REF<Material>(smat->Instance());

		Asset->Give<Model>("Player", &pm);

		//	Door

		Mesh* dmesh = MakeIcosphere(0);
		dmesh->Material = REF<Material>(smat->Instance());

		dmesh->Material->SetTransparency(Transparency::ADD);

		dmesh->Initialize(Renderer->Device);

		Model dm { dmesh, 1 };
		Asset->Give<Model>("Door", &dm);

		Physics->SetGravity(vector3(0, -9.8f, 0));
		Physics->AddSolver(new ImpulseSolver());
		Physics->AddSolver(new SmoothPositionSolver());

		// Rendering pipeline

		//generateShadowMap    = new GenerateShadowMap(Renderer, Space);
		//postProcessShadowMap = new FilterTarget(Renderer);
		//mainRender           = new Render(Renderer, Space);

		//generateShadowMap   ->SetLight(light);
		//postProcessShadowMap->SetIntermediate(targetBlur);
		//postProcessShadowMap->SetShader(gaussian);
		//mainRender          ->SetLight(light);
		//mainRender          ->SetAmbiance(0.008f);
		//mainRender          ->SetGamma(2.2f);

		//pipeline.first(generateShadowMap)
		//	.then(postProcessShadowMap)
		//	.then(mainRender);

		// Systems

		playerSystem = PushSystem<PlayerSystem>();
		enemySystem  = PushSystem<EnemySystem>(playerSystem->GetPlayer());
		PushSystem<PhysicsSystem>();
		PushSystem<BulletSystem>(playerSystem->GetPlayer());
		PushSystem<GameCameraController>(playerSystem->GetPlayer(), scene);
		PushSystem<EnemyDeathCircleSystem>();

		PushSystem<LevelSystem>(playerSystem->GetPlayer());

		PushSystem<ScoreSystem>(playerSystem->GetPlayer(), scene->MainCamera(), textCam);

		return Layer::Initialize();
	}

	void SandboxLayer::PostUpdate() {
		//font->UpdateMesh(textMesh, std::to_string(1.0f / Time::DeltaTime()), 0.01f, 1); //fps
		textMesh->Update(Renderer->Device);
		scene->MainCamera()->SetProjection(iw::lerp(persp, ortho, blend));
		
		// Shadow maps

		//for(Light* light : scene->DirectionalLights()) {
		//	if (!light->CanCastShadows()) {
		//		continue;
		//	}
		//
		//	Renderer->BeginShadowCast(light);
		//
		//	for (auto m_e : Space->Query<Transform, Model>()) {
		//		auto [m_t, m_m] = m_e.Components.Tie<ModelComponents>();
		//
		//		for (size_t i = 0; i < m_m->MeshCount; i++) {
		//			//iw::ref<Texture> t = m_m->Meshes[i].Material->GetTexture("alphaMaskMap");
		//
		//			//ITexture* it = nullptr;
		//			//if (t) {
		//			//	it = t->Handle();
		//			//}
		//
		//			//light->ShadowShader()->Handle()->GetParam("hasAlphaMask")->SetAsFloat(it != nullptr);
		//			//light->ShadowShader()->Handle()->GetParam("alphaMask")->SetAsTexture(it, 0);
		//			//light->ShadowShader()->Handle()->GetParam("alphaThreshold")->SetAsFloat(threshold);
		//
		//			Renderer->DrawMesh(m_t, &m_m->Meshes[i]);
		//		}
		//	}
		//
		//	Renderer->EndShadowCast();
		//}

		for (Light* light : scene->Lights()) {
			if (!light->CanCastShadows()) {
				continue;
			}
			
			Renderer->BeginShadowCast(light);

				for (auto entity : Space->Query<Transform, Model>()) {
					auto [transform, model] = entity.Components.Tie<ModelComponents>();
					for (size_t i = 0; i < model->MeshCount; i++) {
						if (model->Meshes[i].Material->CastShadows()) {
							Renderer->DrawMesh(transform, &model->Meshes[i]);
						}
					}
				}

			Renderer->EndShadowCast();
		}

		// Main render

		Renderer->BeginScene(scene);

			for (auto entity : Space->Query<Transform, Model>()) {
				auto [transform, model] = entity.Components.Tie<ModelComponents>();
				for (size_t i = 0; i < model->MeshCount; i++) {
					Renderer->DrawMesh(transform, &model->Meshes[i]);
				}
			}

		Renderer->EndScene();

		Renderer->BeginScene(textCam);
			Renderer->DrawMesh(&textTransform, textMesh);
		Renderer->EndScene();

		//float blurw = 1.0f / target->Width() * blurAmount;
		//float blurh = 1.0f / target->Height() * blurAmount;

		//generateShadowMap->SetThreshold(threshold);
		//postProcessShadowMap->SetBlur(vector2(blurw, blurh));

		//pipeline.execute();
	}
	
	void SandboxLayer::FixedUpdate() {
		Physics->Step(Time::FixedTime() * ts);
	}

	void SandboxLayer::ImGui() {
		ImGui::Begin("Sandbox");

		ImGui::SliderFloat("Time scale", &ts, 0.001f, 1);

		ImGui::SliderFloat("Ambiance", (float*)&scene->Ambiance(), 0, 1);
		//ImGui::SliderFloat("Gamma", (float*)&mainRender->GetGamma(), 0, 5);
		ImGui::SliderFloat("Camera blend", &blend, 0, 1);


		//ImGui::SliderFloat("Shadow map blur", &blurAmount, 0, 5);
		ImGui::SliderFloat("Shadow map threshold", &threshold, 0, 1);

		ImGui::SliderFloat3("Text pos",   (float*)&textTransform.Position, -8, 8);
		ImGui::SliderFloat3("Text scale", (float*)&textTransform.Scale, 0, 10);
		ImGui::SliderFloat4("Text rot",   (float*)&textTransform.Rotation, 0, 1);

		ImGui::SliderFloat("Volume", &Audio->GetVolume(), 0, 1);

		if (ImGui::Button("Start ambiance")) {
			Audio->AsStudio()->StartInstance(forestInstance);
		}

		if (ImGui::Button("Stop ambiance")) {
			Audio->AsStudio()->StopInstance(forestInstance);
		}

		if (ImGui::Button("Next level (don't spam)")) {
			Bus->push<LoadNextLevelEvent>();
		}

		//if (ImGui::Button("Save level")) {
		//	JsonSerializer jout("assets/levels/working.json", true);
		//	
		//	Level level;
		//	for (auto entity : Space->Query<Transform, Enemy>()) {
		//		auto [transform, enemy] = entity.Components.Tie<EnemySystem::Components>();
		//		level.Positions.push_back({ transform->Position.x, transform->Position.z });
		//		level.Enemies.push_back(*enemy);
		//	}

		//	level.StageName = "models/grass/grass.obj";
		//	
		//	jout.Write(level);
		//}

		//if (ImGui::Button("Load level")) {
		//	loadLevel = "assets/levels/forest/level1.bin";
		//}

		ImGui::End();
	}

	bool settexttocursor = false;

	bool SandboxLayer::On(
		MouseMovedEvent& e)
	{
		if (settexttocursor) {
			str.clear();
			font->UpdateMesh(textMesh, std::to_string((int)e.X) + '\n' + std::to_string((int)e.Y), .01f, 1);
		}

		if (e.X < -1000) {
			settexttocursor = true;
		}

		return false;
	}

	//bool SandboxLayer::On(
	//	KeyTypedEvent& e)
	//{
	//	str += e.Character;
	//	font->UpdateMesh(textMesh, str, .01f, 1);

	//	return false;
	//}

	bool SandboxLayer::On(
		ActionEvent& e)
	{
		switch (e.Action) {
			case iw::val(Actions::GOTO_NEXT_LEVEL): {
				playerSystem->On(e);
				enemySystem ->On(e);
				PopSystem(playerSystem);
				PopSystem(enemySystem);

				if (e.as<GoToNextLevelEvent>().LevelName == "models/block/forest100.dae") {
					font->UpdateMesh(textMesh, "ayy you've gotten to the boss congrats!\nsadly he's out today so\nhave some fun with the physics instead...\nmember you can press i/t", .004f, 1);
				}

				else if (e.as<GoToNextLevelEvent>().LevelName == "models/block/forest08.dae") {
					font->UpdateMesh(textMesh, "So this would be a lil mini boss but that seems\nlike it would be annoying to program xd", .004f, 1);
				}

				else if (e.as<GoToNextLevelEvent>().LevelName == "models/block/forest01.dae") {
					font->UpdateMesh(textMesh, "Lets go! You've finished the play test\nIf you got the time, post some feedback at\nhttps://winter.dev/demo", .004f, 1);
				}

				else {
					font->UpdateMesh(textMesh, "", .01f, 1);
					settexttocursor = false;
				}

				break;
			}
			case iw::val(Actions::AT_NEXT_LEVEL): {
				PushSystemFront(playerSystem);
				PushSystemFront(enemySystem);
				break;
			}
		}

		return Layer::On(e);
	}
}

