#pragma once

#include "IwRenderer.h"
#include "Texture.h"
#include "iw/math/matrix2.h"
#include "iw/math/matrix3.h"
#include "iw/math/matrix4.h"
#include "iw/log/logger.h"

namespace IW {
inline namespace RenderAPI {
	//leak that state

	// if stride is 0 then count is used inplace

	class IWRENDERER_API IPipelineParam {
	public:
		virtual ~IPipelineParam() {}

		virtual void SetAsBool(
			bool value) = 0;

		virtual void SetAsBools(
			const void* values,
			size_t count,
			size_t stride = 0) = 0;

		virtual void SetAsInt(
			int value) = 0;

		virtual void SetAsInts(
			const void* values,
			size_t count,
			size_t stride = 0) = 0;

		virtual void SetAsUInt(
			unsigned int value) = 0;

		virtual void SetAsUInts(
			const void* values,
			size_t count,
			size_t stride = 0) = 0;

		virtual void SetAsFloat(
			float value) = 0;

		virtual void SetAsFloats(
			const void* values,
			size_t count,
			size_t stride = 0) = 0;

		virtual void SetAsDouble(
			double value) = 0;

		virtual void SetAsDoubles(
			const void* values,
			size_t count,
			size_t stride = 0) = 0;

		virtual void SetAsMat2(
			const iwm::matrix2& matrix) = 0;

		virtual void SetAsMat3(
			const iwm::matrix3& matrix) = 0;

		virtual void SetAsMat4(
			const iwm::matrix4& matrix) = 0;

		virtual void SetAsTexture(
			const ITexture* texture) = 0;
	};
}
}
