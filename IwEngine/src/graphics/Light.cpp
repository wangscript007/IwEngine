#include "iw/graphics/Light.h"
#include "iw/graphics/Renderer.h"

namespace iw {
namespace Graphics {
	Light::Light(
		float power,
		iw::Camera* camera,
		iw::ref<RenderTarget> shadowTarget,
		iw::ref<Shader> shadowShader,
		iw::ref<Shader> particleShadowShader)
		: m_intensity(power)
		, m_shadowCamera(camera)
		, m_shadowTarget(shadowTarget)
		, m_shadowShader(shadowShader)
		, m_particleShadowShader(particleShadowShader)
	{
#ifdef IW_DEBUG
		if (!!shadowShader != !!shadowTarget) {
			LOG_WARNING << "A light needs both a shader and a target or neither.";
		}
#endif
	}

	Light::~Light() {
		delete m_shadowCamera;
	}

	void Light::EndShadowCast(
		Renderer* renderer)
	{
		if (m_shadowTarget) {
			m_shadowTarget->Tex(0)->Handle()->GenerateMipMaps();
		}

		renderer->Device->SetCullFace(BACK);
	}

	bool Light::CanCastShadows() const {
		return m_shadowShader && m_shadowTarget;
	}

	bool Light::Outdated() const {
		return m_shadowCamera->Outdated();
	}

	float Light::Intensity() const {
		return m_intensity;
	}

	const vector3& Light::Position() const {
		return m_shadowCamera->Position();
	}

	const iw::ref<RenderTarget>& Light::ShadowTarget() const {
		return m_shadowTarget;
	}

	const iw::ref<Shader>& Light::ShadowShader() const {
		return m_shadowShader;
	}

	const ref<Shader>& Light::ParticleShadowShader() const {
		return m_particleShadowShader;
	}

	const Camera* Light::ShadowCamera() const {
		return m_shadowCamera;
	}

	void Light::SetIntensity(
		float intensity)
	{
		m_intensity = intensity;
	}

	void Light::SetPosition(
		const vector3& position)
	{
		m_shadowCamera->SetPosition(position);
	}

	void Light::SetShadowTarget(
		ref<RenderTarget>& shadowTarget)
	{
		m_shadowTarget = shadowTarget;
	}

	void Light::SetShadowShader(
		ref<Shader>& shadowShader)
	{
		m_shadowShader = shadowShader;
	}

	void Light::SetParticleShadowShader(
		ref<Shader>& particleShadowShader)
	{
		m_particleShadowShader = particleShadowShader;
	}

	/*void Light::PostProcess() {}

	const iw::ref<Shader>& Light::LightShader() const {
		return m_light;
	}

	const iw::ref<Shader>& Light::NullFilter() const {
		return m_null;
	}

	const iw::ref<RenderTarget>& Light::ShadowTarget() const {
		return m_shadowTarget;
	}

	const iw::ref<Shader>& Light::PostFilter() const {
		return m_post;
	}

	const iw::ref<RenderTarget>& Light::PostTarget() const {
		return m_postTarget;
	}

	void Light::SetLightShader(
		iw::ref<Shader>& lightShader)
	{
		m_light = lightShader;
	}

	void Light::SetNullFilter(
		iw::ref<Shader>& nullFilter)
	{
		m_null = nullFilter;
	}

	void Light::SetShadowTarget(
		iw::ref<RenderTarget>& shadowTarget)
	{
		m_shadowTarget = shadowTarget;
	}

	void Light::SetPostFilter(
		iw::ref<Shader>& post)
	{
		m_post = post;
	}

	void Light::SetPostTarget(
		iw::ref<RenderTarget>& postTarget)
	{
		m_postTarget = postTarget;
	}*/
}
}
