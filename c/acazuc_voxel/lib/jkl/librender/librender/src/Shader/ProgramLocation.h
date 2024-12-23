#ifndef LIBRENDER_PROGRAM_LOCATION_H
# define LIBRENDER_PROGRAM_LOCATION_H

# include "../Mat/Mat4.h"
# include "../Mat/Mat3.h"
# include "../Vec/Vec4.h"
# include "../Vec/Vec3.h"
# include "../Vec/Vec2.h"
# include "./VertexBuffer.h"
# include "../GL.h"

namespace librender
{

	class ProgramLocation
	{

	private:
		GLint location;

	public:
		ProgramLocation();
		ProgramLocation(GLint location);
		void setVec1i(GLint val);
		void setVec1iv(const GLint *val, GLsizei count);
		void setVec2i(GLint val1, GLint val2);
		void setVec2i(TVec2<GLint> val);
		void setVec2iv(const TVec2<GLint> *val, GLsizei count);
		void setVec3i(GLint val1, GLint val2, GLint val3);
		void setVec3i(TVec3<GLint> val);
		void setVec3iv(const TVec3<GLint> *val, GLsizei count);
		void setVec4i(GLint val1, GLint val2, GLint val3, GLint val4);
		void setVec4i(TVec4<GLint> val);
		void setVec4iv(const TVec4<GLint> *val, GLsizei count);
		void setVec1f(GLfloat val);
		void setVec1fv(const GLfloat *val, GLsizei count);
		void setVec2f(GLfloat val1, GLfloat val2);
		void setVec2f(TVec2<GLfloat> val);
		void setVec2fv(const TVec2<GLfloat> *val, GLsizei count);
		void setVec3f(GLfloat val1, GLfloat val2, GLfloat val3);
		void setVec3f(TVec3<GLfloat> val);
		void setVec3fv(const TVec3<GLfloat> *val, GLsizei count);
		void setVec4f(GLfloat val1, GLfloat val2, GLfloat val3, GLfloat val4);
		void setVec4f(TVec4<GLfloat> val);
		void setVec4fv(const TVec4<GLfloat> *val, GLsizei count);
		void setMat3f(TMat3<GLfloat> val);
		void setMat3fv(const TMat3<GLfloat> *val, GLsizei count);
		void setMat4f(TMat4<GLfloat> val);
		void setMat4fv(const TMat4<GLfloat> *val, GLsizei count);
		void setVertexBuffer(const VertexBuffer &buffer, GLint nb, GLenum type, GLboolean normalized = GL_FALSE, GLsizei stride = 0, GLsizei offset = 0);
		void setVertexAttribArray(bool asArray);
		inline GLint getLocation() {return this->location;};

	};

}

#endif
