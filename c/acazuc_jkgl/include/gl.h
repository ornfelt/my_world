#ifndef GL_H
#define GL_H

#include <stdint.h>

#define GL_ZERO 0x0000
#define GL_ONE  0x0001

#define GL_FALSE 0x0000
#define GL_TRUE  0x0001

#define GL_NEVER    0x0200
#define GL_LESS     0x0201
#define GL_EQUAL    0x0202
#define GL_LEQUAL   0x0203
#define GL_GREATER  0x0204
#define GL_NOTEQUAL 0x0205
#define GL_GEQUAL   0x0206
#define GL_ALWAYS   0x0207

#define GL_SRC_COLOR           0x0300
#define GL_ONE_MINUS_SRC_COLOR 0x0301
#define GL_SRC_ALPHA           0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_DST_ALPHA           0x0304
#define GL_ONE_MINUS_DST_ALPHA 0x0305
#define GL_DST_COLOR           0x0306
#define GL_ONE_MINUS_DST_COLOR 0x0307
#define GL_SRC_ALPHA_SATURATE  0x0308

#define GL_FRONT_LEFT     0x0400
#define GL_FRONT_RIGHT    0x0401
#define GL_BACK_LEFT      0x0402
#define GL_BACK_RIGHT     0x0403
#define GL_FRONT          0x0404
#define GL_BACK           0x0405
#define GL_LEFT           0x0406
#define GL_RIGHT          0x0407
#define GL_FRONT_AND_BACK 0x0408

#define GL_NO_ERROR          0x0000
#define GL_INVALID_ENUM      0x0500
#define GL_INVALID_VALUE     0x0501
#define GL_INVALID_OPERATION 0x0502
#define GL_STACK_OVERFLOW    0x0503
#define GL_STACK_UNDERFLOW   0x0504
#define GL_OUT_OF_MEMORY     0x0505

#define GL_EXP  0x0800
#define GL_EXP2 0x0801

#define GL_CW  0x0900
#define GL_CCW 0x0901

#define GL_POINT_SMOOTH 0x0B10
#define GL_POINT_SIZE   0x0B11
#define GL_LINE_SMOOTH  0x0B20
#define GL_LINE_WIDTH   0x0B21

#define GL_CULL_FACE 0x0B44

#define GL_LIGHTING 0x0B50

#define GL_FOG         0x0B60
#define GL_FOG_MODE    0x0B65
#define GL_FOG_DENSITY 0x0B62
#define GL_FOG_START   0x0B63
#define GL_FOG_END     0x0B64
#define GL_FOG_INDEX   0x0B61
#define GL_FOG_COLOR   0x0B66

#define GL_DEPTH_RANGE       0x0B70
#define GL_DEPTH_TEST        0x0B71
#define GL_DEPTH_WRITEMASK   0x0B72
#define GL_DEPTH_CLEAR_VALUE 0x0B73
#define GL_DEPTH_FUNC        0x0B74

#define GL_STENCIL_CLEAR_VALUE 0x0B91

#define GL_MATRIX_MODE            0x0BA0
#define GL_MODELVIEW_STACK_DEPTH  0x0BA3
#define GL_PROJECTION_STACK_DEPTH 0x0BA4
#define GL_MODELVIEW_MATRIX       0x0BA6
#define GL_PROJECTION_MATRIX      0x0BA7

#define GL_BLEND 0x0BE2

#define GL_SCISSOR_TEST 0x0C11

#define GL_COLOR_CLEAR_VALUE 0x0C22
#define GL_COLOR_WRITEMASK   0x0C23

#define GL_MAX_LIGHTS                 0x0D31
#define GL_MAX_MODELVIEW_STACK_DEPTH  0x0D36
#define GL_MAX_PROJECTION_STACK_DEPTH 0x0D38

#define GL_TEXTURE_1D 0x0DE0
#define GL_TEXTURE_2D 0x0DE1

#define GL_AMBIENT               0x1200
#define GL_DIFFUSE               0x1201
#define GL_SPECULAR              0x1202
#define GL_POSITION              0x1203
#define GL_SPOT_DIRECTION        0x1204
#define GL_SPOT_EXPONENT         0x1205
#define GL_SPOT_CUTOFF           0x1206
#define GL_CONSTANT_ATTENUATION  0x1207
#define GL_LINEAR_ATTENUATION    0x1208
#define GL_QUADRATIC_ATTENUATION 0x1209

#define GL_COMPILE             1300
#define GL_COMPILE_AND_EXECUTE 1301

#define GL_BYTE           0x1400
#define GL_UNSIGNED_BYTE  0x1401
#define GL_SHORT          0x1402
#define GL_UNSIGNED_SHORT 0x1403
#define GL_INT            0x1404
#define GL_UNSIGNED_INT   0x1405
#define GL_FLOAT          0x1406
#define GL_DOUBLE         0x140A

#define GL_EMISSION            0x1600
#define GL_SHININESS           0x1601
#define GL_AMBIENT_AND_DIFFUSE 0x1602

#define GL_MODELVIEW  0x1700
#define GL_PROJECTION 0x1701

#define GL_RGBA 0x1908

#define GL_FLAT   0x1D00
#define GL_SMOOTH 0x1D01

#define GL_NEAREST 0x2600
#define GL_LINEAR  0x2601

#define GL_NEAREST_MIPMAP_NEAREST 0x2700
#define GL_LINEAR_MIPMAP_NEAREST  0x2701
#define GL_NEAREST_MIPMAP_LINEAR  0x2702
#define GL_LINEAR_MIPMAP_LINEAR   0x2703

#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S     0x2802
#define GL_TEXTURE_WRAP_T     0x2803

#define GL_CLAMP  0x2900
#define GL_REPEAT 0x2901

#define GL_R3_G3_B2 0x2A10

#define GL_LIGHT0 0x4000
#define GL_LIGHT1 0x4001
#define GL_LIGHT2 0x4002
#define GL_LIGHT3 0x4003
#define GL_LIGHT4 0x4004
#define GL_LIGHT5 0x4005
#define GL_LIGHT6 0x4006
#define GL_LIGHT7 0x4007

#define GL_CONSTANT_COLOR           0x8001
#define GL_ONE_MINUS_CONSTANT_COLOR 0x8002
#define GL_CONSTANT_ALPHA           0x8003
#define GL_ONE_MINUS_CONSTANT_ALPHA 0x8004
#define GL_BLEND_COLOR              0x8005
#define GL_FUNC_ADD                 0x8006
#define GL_MIN                      0x8007
#define GL_MAX                      0x8008
#define GL_BLEND_EQUATION_RGB       0x8009
#define GL_FUNC_SUBSTRACT           0x800A
#define GL_FUNC_REVERSE_SUBSTRACT   0x800B

#define GL_ALPHA4  0x803B
#define GL_ALPHA8  0x803C
#define GL_ALPHA16 0x803E

#define GL_RGB4     0x804F
#define GL_RGB5     0x8050
#define GL_RGB8     0x8051
#define GL_RGB10    0x8052
#define GL_RGB12    0x8053
#define GL_RGB16    0x8054
#define GL_RGBA2    0x8055
#define GL_RGBA4    0x8056
#define GL_RGB5_A1  0x8057
#define GL_RGBA8    0x8058
#define GL_RGB10_A2 0x8059
#define GL_RGBA12   0x805A
#define GL_RGBA16   0x805B

#define GL_TEXTURE_BINDING_1D 0x8068
#define GL_TEXTURE_BINDING_2D 0x8069
#define GL_TEXTURE_BINDING_3D 0x806A
#define GL_TEXTURE_3D         0x806F

#define GL_TEXTURE_WRAP_R 0x8072

#define GL_VERTEX_ARRAY                0x8074
#define GL_NORMAL_ARRAY                0x8075
#define GL_COLOR_ARRAY                 0x8076
#define GL_TEXTURE_COORD_ARRAY         0x8078
#define GL_VERTEX_ARRAY_SIZE           0x807A
#define GL_VERTEX_ARRAY_TYPE           0x807B
#define GL_VERTEX_ARRAY_STRIDE         0x807C
#define GL_NORMAL_ARRAY_TYPE           0x807E
#define GL_NORMAL_ARRAY_STRIDE         0x807F
#define GL_COLOR_ARRAY_SIZE            0x8081
#define GL_COLOR_ARRAY_TYPE            0x8082
#define GL_COLOR_ARRAY_STRIDE          0x8083
#define GL_TEXTURE_COORD_ARRAY_SIZE    0x8088
#define GL_TEXTURE_COORD_ARRAY_TYPE    0x8089
#define GL_TEXTURE_COORD_ARRAY_STRIDE  0x808A
#define GL_VERTEX_ARRAY_POINTER        0x808E
#define GL_NORMAL_ARRAY_POINTER        0x808F
#define GL_COLOR_ARRAY_POINTER         0x8090
#define GL_TEXTURE_COORD_ARRAY_POINTER 0x8092

#define GL_BLEND_DST_RGB   0x80C8
#define GL_BLEND_SRC_RGB   0x80C9
#define GL_BLEND_DST_ALPHA 0x80CA
#define GL_BLEND_SRC_ALPHA 0x80CB

#define GL_CLAMP_TO_BORDER 0x812D
#define GL_CLAMP_TO_EDGE   0x812F

#define GL_CUBIC 0x8334

#define GL_MIRRORED_REPEAT 0x8370

#define GL_FOG_COORD_SRC  0x8450
#define GL_FOG_COORD      0x8451
#define GL_FRAGMENT_DEPTH 0x8452

#define GL_BLEND_EQUATION_ALPHA 0x883D

#define GL_SRGB8        0x8C41
#define GL_SRGB8_ALPHA8 0x8C43

#define GL_POINTS         0x0000
#define GL_LINES          0x0001
#define GL_LINE_STRIP     0x0002
#define GL_LINE_LOOP      0x0003
#define GL_TRIANGLES      0x0004
#define GL_TRIANGLE_STRIP 0x0005
#define GL_TRIANGLE_FAN   0x0006
#define GL_QUADS          0x0007
#define GL_QUAD_STRIP     0x0008
#define GL_POLYGONS       0x0009

#define GL_COLOR_BUFFER_BIT   0x4000
#define GL_DEPTH_BUFFER_BIT   0x0100
#define GL_STENCIL_BUFFER_BIT 0x0400

typedef uint32_t GLbitmask;
typedef uint32_t GLenum;
typedef double GLdouble;
typedef float GLfloat;
typedef uint8_t GLboolean;
typedef int64_t GLlong;
typedef uint64_t GLulong;
typedef int32_t GLint;
typedef uint32_t GLuint;
typedef int16_t GLshort;
typedef uint16_t GLushort;
typedef int8_t GLbyte;
typedef uint8_t GLubyte;
typedef int32_t GLsizei;
typedef float GLclampf;
typedef double GLclampd;
typedef void GLvoid;

struct gl_ctx;

void gl_ctx_set(struct gl_ctx *new_ctx);
struct gl_ctx *gl_ctx_new(void);

GLenum glGetError(void);
void glViewport(GLsizei width, GLsizei height);
void glClear(GLbitmask mask);
void glShadeModel(GLenum mode);
void glPointSize(GLfloat size);
void glLineWidth(GLfloat width);
void glFrontFace(GLenum mode);
void glCullFace(GLenum mode);
void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *data);
void glScissor(GLint x, GLint y, GLsizei width, GLsizei height);

void glMatrixMode(GLenum mode);
void glLoadMatrixd(const GLdouble *m);
void glLoadMatrixf(const GLfloat *m);
void glMultMatrixd(const GLdouble *m);
void glMultMatrixf(const GLfloat *m);
void glLoadIdentity(void);
void glMultTransposeMmatrixd(const GLdouble *m);
void glMultTransposeMatrixf(const GLfloat *m);
void glLoadTransposeMatrixd(const GLdouble *m);
void glLoadTransposeMatrixf(const GLfloat *m);
void glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near, GLdouble far);
void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near, GLdouble far);
void glTranslated(GLdouble x, GLdouble y, GLdouble z);
void glTranslatef(GLfloat x, GLfloat y, GLfloat z);
void glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void glScaled(GLdouble x, GLdouble y, GLdouble z);
void glScalef(GLfloat x, GLfloat y, GLfloat z);
void glPushMatrix(void);
void glPopMatrix(void);

GLboolean glGet(GLenum pname, void **data, GLenum *format, GLuint *length);
void glGetBooleanv(GLenum pname, GLboolean *params);
void glGetDoublev(GLenum pname, GLdouble *params);
void glGetFloatv(GLenum pname, GLfloat *params);
void glGetIntegerv(GLenum pname, GLint *params);
void glGetPointerv(GLenum pname, GLvoid **params);

void glDepthMask(GLboolean flag);
void glDepthFunc(GLenum func);
void glDepthRange(GLclampd near_val, GLclampd far_val);
void glDepthRangef(GLclampf near_val, GLclampf far_val);

void glDisable(GLenum cap);
void glEnable(GLenum cap);
GLboolean glIsEnabled(GLenum cap);

void glBlendFunc(GLenum s_factor, GLenum d_factor);
void glBlendFuncSeparate(GLenum src_rgb, GLenum dst_rgb, GLenum src_alpha, GLenum dst_alpha);
void glBlendEquation(GLenum mode);
void glBlendEquationSeparate(GLenum mode_rgb, GLenum mode_alpha);
void glBlendColor(GLclampf red, GLclampf green, GLclampf blue,GLclampf alpha);

void glFogf(GLenum pname, GLfloat param);
void glFogi(GLenum pname, GLint param);
void glFogfv(GLenum pname, GLfloat *param);
void glFogiv(GLenum pname, GLint *param);

void glDisableClientState(GLenum cap);
void glEnableClientState(GLenum cap);
void glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void glNormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer);

void glDrawArrays(GLenum mode, GLint first, GLsizei count);
void glMultiDrawArrays(GLenum mode, GLint *first, GLsizei *count, GLsizei primcount);
void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
void glMultiDrawElements(GLenum mode, const GLsizei *count, GLenum type, const GLvoid **indices, GLsizei primcount);

void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void glClearDepth(GLclampf depth);
void glClearStencil(GLubyte stencil);

void glBegin(GLenum mode);
void glEnd(void);

void glVertex2s(GLshort x, GLshort y);
void glVertex2i(GLint x, GLint y);
void glVertex2f(GLfloat x, GLfloat y);
void glVertex2d(GLdouble x, GLdouble y);
void glVertex3s(GLshort x, GLshort y, GLshort z);
void glVertex3i(GLint x, GLint y, GLint z);
void glVertex3f(GLfloat x, GLfloat y, GLfloat z);
void glVertex3d(GLdouble x, GLdouble y, GLdouble z);
void glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w);
void glVertex4i(GLint x, GLint y, GLint z, GLint w);
void glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w);

void glVertex2sv(const GLshort *v);
void glVertex2iv(const GLint *v);
void glVertex2fv(const GLfloat *v);
void glVertex2dv(const GLdouble *v);
void glVertex3sv(const GLshort *v);
void glVertex3iv(const GLint *v);
void glVertex3fv(const GLfloat *v);
void glVertex3dv(const GLdouble *v);
void glVertex4sv(const GLshort *v);
void glVertex4iv(const GLint *v);
void glVertex4fv(const GLfloat *v);
void glVertex4dv(const GLdouble *v);

void glNormal3b(GLbyte x, GLbyte y, GLbyte z);
void glNormal3s(GLshort x, GLshort y, GLshort z);
void glNormal3i(GLint x, GLint y, GLint z);
void glNormal3f(GLfloat x, GLfloat y, GLfloat z);
void glNormal3d(GLdouble x, GLdouble y, GLdouble z);

void glNormal3bv(const GLbyte *v);
void glNormal3sv(const GLshort *v);
void glNormal3iv(const GLint *v);
void glNormal3fv(const GLfloat *v);
void glNormal3dv(const GLdouble *v);

void glTexCoord1s(GLshort s);
void glTexCoord1i(GLint s);
void glTexCoord1f(GLfloat s);
void glTexCoord1d(GLdouble s);
void glTexCoord2s(GLshort s, GLshort t);
void glTexCoord2i(GLint s, GLint t);
void glTexCoord2f(GLfloat s, GLfloat t);
void glTexCoord2d(GLdouble s, GLdouble t);
void glTexCoord3s(GLshort s, GLshort t, GLshort r);
void glTexCoord3i(GLint s, GLint t, GLint r);
void glTexCoord3f(GLfloat s, GLfloat t, GLfloat r);
void glTexCoord3d(GLdouble s, GLdouble t, GLdouble r);
void glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q);
void glTexCoord4i(GLint s, GLint t, GLint r, GLint q);
void glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
void glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q);

void glTexCoord1sv(const GLshort *v);
void glTexCoord1iv(const GLint *v);
void glTexCoord1fv(const GLfloat *v);
void glTexCoord1dv(const GLdouble *v);
void glTexCoord2sv(const GLshort *v);
void glTexCoord2iv(const GLint *v);
void glTexCoord2fv(const GLfloat *v);
void glTexCoord2dv(const GLdouble *v);
void glTexCoord3sv(const GLshort *v);
void glTexCoord3iv(const GLint *v);
void glTexCoord3fv(const GLfloat *v);
void glTexCoord3dv(const GLdouble *v);
void glTexCoord4sv(const GLshort *v);
void glTexCoord4iv(const GLint *v);
void glTexCoord4fv(const GLfloat *v);
void glTexCoord4dv(const GLdouble *v);

void glColor3b(GLbyte r, GLbyte g, GLbyte b);
void glColor3ub(GLubyte r, GLubyte g, GLubyte b);
void glColor3s(GLshort r, GLshort g, GLshort b);
void glColor3us(GLushort r, GLushort g, GLushort b);
void glColor3i(GLint r, GLint g, GLint b);
void glColor3ui(GLuint r, GLuint g, GLuint b);
void glColor3f(GLfloat r, GLfloat g, GLfloat b);
void glColor3d(GLdouble r, GLdouble g, GLdouble b);
void glColor4b(GLbyte r, GLbyte g, GLbyte b, GLbyte a);
void glColor4ub(GLubyte r, GLubyte g, GLubyte b, GLubyte a);
void glColor4s(GLshort r, GLshort g, GLshort b, GLshort a);
void glColor4us(GLushort r, GLushort g, GLushort b, GLushort a);
void glColor4i(GLint r, GLint g, GLint b, GLint a);
void glColor4ui(GLuint r, GLuint g, GLuint b, GLuint a);
void glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
void glColor4d(GLdouble r, GLdouble g, GLdouble b, GLdouble a);

void glColor3bv(const GLbyte *v);
void glColor3ubv(const GLubyte *v);
void glColor3sv(const GLshort *v);
void glColor3usv(const GLushort *v);
void glColor3iv(const GLint *v);
void glColor3uiv(const GLuint *v);
void glColor3fv(const GLfloat *v);
void glColor3dv(const GLdouble *v);
void glColor4bv(const GLbyte *v);
void glColor4ubv(const GLubyte *v);
void glColor4sv(const GLshort *v);
void glColor4usv(const GLushort *v);
void glColor4iv(const GLint *v);
void glColor4uiv(const GLuint *v);
void glColor4fv(const GLfloat *v);
void glColor4dv(const GLdouble *v);

void glGenTextures(GLsizei n, GLuint *textures);
void glDeleteTextures(GLsizei n, const GLuint *textures);
void glBindTexture(GLenum target, GLuint texture);
void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *data);
void glTexParameteri(GLenum target, GLenum pname, GLint param);
void glTexParameterf(GLenum target, GLenum pname, GLfloat param);

void glLightf(GLenum light, GLenum pname, GLfloat param);
void glLighti(GLenum light, GLenum pname, GLint param);
void glLightfv(GLenum light, GLenum pname, GLfloat *params);
void glLightiv(GLenum light, GLenum pname, GLint *params);
void glGetLightfv(GLenum light, GLenum pname, GLfloat *params);
void glGetLightiv(GLenum light, GLenum pname, GLint *params);

void glMaterialf(GLenum face, GLenum pname, GLfloat param);
void glMateriali(GLenum face, GLenum pname, GLint param);
void glMaterialfv(GLenum face, GLenum pname, GLfloat *params);
void glMaterialiv(GLenum face, GLenum pname, GLint *params);
void glGetMaterialfv(GLenum face, GLenum pname, GLfloat *params);
void glGetMaterialiv(GLenum face, GLenum pname, GLint *params);

GLuint glGenLists(GLsizei range);
void glDeleteLists(GLuint list, GLsizei range);
GLboolean glIsList(GLuint list);
void glListBase(GLuint base);
void glNewList(GLuint list, GLenum mode);
void glEndList(void);
void glCallList(GLuint list);
void glCallLists(GLsizei n, GLenum type, const GLvoid *lists);

#endif
