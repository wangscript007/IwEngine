#include "iw/renderer/Platform/OpenGL/GLPipelineParam.h"
#include "iw/renderer/Platform/OpenGL/GLTexture.h"
#include "gl/glew.h"

namespace IW {
	GLPipelineParam::GLPipelineParam(
		int location,
		unsigned int& textureCount)
		: m_location(location)
		, m_textureCount(textureCount)
	{}

	void GLPipelineParam::SetAsBool(
		bool value)
	{
		SetAsInt(value);
	}

	void GLPipelineParam::SetAsBools(
		const void* values,
		unsigned count,
		unsigned stride)
	{
		SetAsInts(values, count, stride);
	}

	void GLPipelineParam::SetAsInt(
		int value)
	{
		glUniform1i(m_location, value);
	}

	void GLPipelineParam::SetAsInts(
		const void* values,
		unsigned count,
		unsigned stride)
	{
		if (stride) {
			switch (stride) {
				case 1: glUniform1iv(m_location, count, (const GLint*)values); break;
				case 2: glUniform2iv(m_location, count, (const GLint*)values); break;
				case 3: glUniform3iv(m_location, count, (const GLint*)values); break;
				case 4: glUniform4iv(m_location, count, (const GLint*)values); break;
				default: LOG_WARNING << "Stride is too large (" << (stride) << ")"; break;
			}
		}

		else {
			const GLint* v = (const GLint*)values;
			switch (count) {
				case 1: glUniform1i(m_location, v[0]); break;
				case 2: glUniform2i(m_location, v[0], v[1]); break;
				case 3: glUniform3i(m_location, v[0], v[1], v[2]); break;
				case 4: glUniform4i(m_location, v[0], v[1], v[2], v[3]); break;
				default: LOG_WARNING << "Stride is too large (" << (stride) << ")"; break;
			}
		}
	}

	void GLPipelineParam::SetAsUInt(
		unsigned int value)
	{
		glUniform1ui(m_location, value);
	}

	void GLPipelineParam::SetAsUInts(
		const void* values,
		unsigned count,
		unsigned stride)
	{
		if (stride) {
			switch (stride) {
				case 1: glUniform1uiv(m_location, count, (const GLuint*)values); break;
				case 2: glUniform2uiv(m_location, count, (const GLuint*)values); break;
				case 3: glUniform3uiv(m_location, count, (const GLuint*)values); break;
				case 4: glUniform4uiv(m_location, count, (const GLuint*)values); break;
				default: LOG_WARNING << "Stride is too large (" << (stride) << ")"; break;
			}
		}

		else {
			const GLuint* v = (const GLuint*)values;
			switch (count) {
				case 1: glUniform1ui(m_location, v[0]); break;
				case 2: glUniform2ui(m_location, v[0], v[1]); break;
				case 3: glUniform3ui(m_location, v[0], v[1], v[2]); break;
				case 4: glUniform4ui(m_location, v[0], v[1], v[2], v[3]); break;
				default: LOG_WARNING << "Stride is too large (" << (stride) << ")"; break;
			}
		}
	}

	void GLPipelineParam::SetAsFloat(
		float value)
	{
		glUniform1f(m_location, value);
	}

	void GLPipelineParam::SetAsFloats(
		const void* values,
		unsigned count,
		unsigned stride)
	{
		if (stride) {
			switch (stride) {
				case 1: glUniform1fv(m_location, count, (const GLfloat*)values); break;
				case 2: glUniform2fv(m_location, count, (const GLfloat*)values); break;
				case 3: glUniform3fv(m_location, count, (const GLfloat*)values); break;
				case 4: glUniform4fv(m_location, count, (const GLfloat*)values); break;
				default: LOG_WARNING << "Stride is too large (" << (stride) << ")";  break;
			}
		}

		else {
			const GLfloat* v = (const GLfloat*)values;
			switch (count) {
				case 1: glUniform1f(m_location, v[0]); break;
				case 2: glUniform2f(m_location, v[0], v[1]); break;
				case 3: glUniform3f(m_location, v[0], v[1], v[2]); break;
				case 4: glUniform4f(m_location, v[0], v[1], v[2], v[3]); break;
				default: LOG_WARNING << "Stride is too large (" << (stride) << ")"; break;
			}
		}
	}

	void GLPipelineParam::SetAsDouble(
		double value)
	{
		glUniform1d(m_location, value);
	}

	void GLPipelineParam::SetAsDoubles(
		const void* values,
		unsigned count,
		unsigned stride)
	{
		if (stride) {
			switch (stride) {
				case 1: glUniform1dv(m_location, count, (const GLdouble*)values); break;
				case 2: glUniform2dv(m_location, count, (const GLdouble*)values); break;
				case 3: glUniform3dv(m_location, count, (const GLdouble*)values); break;
				case 4: glUniform4dv(m_location, count, (const GLdouble*)values); break;
				default: LOG_WARNING << "Stride is too large (" << (stride) << ")"; break;
			}
		}

		else {
			const GLdouble* v = (const GLdouble*)values;
			switch (count) {
				case 1: glUniform1d(m_location, v[0]); break;
				case 2: glUniform2d(m_location, v[0], v[1]); break;
				case 3: glUniform3d(m_location, v[0], v[1], v[2]); break;
				case 4: glUniform4d(m_location, v[0], v[1], v[2], v[3]); break;
				default: LOG_WARNING << "Stride is too large (" << (stride) << ")"; break;
			}
		}
	}

	void GLPipelineParam::SetAsMat2(
		const iw::matrix2& matrix)
	{
		glUniformMatrix2fv(m_location, 1, GL_FALSE, matrix.elements);
	}

	void GLPipelineParam::SetAsMat3(
		const iw::matrix3& matrix)
	{
		glUniformMatrix3fv(m_location, 1, GL_FALSE, matrix.elements);
	}

	void GLPipelineParam::SetAsMat4(
		const iw::matrix4& matrix)
	{
		glUniformMatrix4fv(m_location, 1, GL_FALSE, matrix.elements);
	}

	void GLPipelineParam::SetAsTexture(
		const ITexture* texture)
	{
		if (m_textureCount > 32) {
			LOG_WARNING << "Already bound 32 textures";
			return;
		}

		glUniform1i(m_location, m_textureCount);

		glActiveTexture(GL_TEXTURE0 + m_textureCount);
		static_cast<const GLTexture*>(texture)->Bind();

		++m_textureCount;
	}
}
