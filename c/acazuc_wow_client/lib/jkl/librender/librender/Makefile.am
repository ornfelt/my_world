lib_LTLIBRARIES = librender.la

librender_la_SOURCES = src/Color.cpp \
                       src/DrawableBase.cpp \
                       src/DrawableBatch.cpp \
                       src/DrawableBatched.cpp \
                       src/Drawable.cpp \
                       src/DrawableTessellator.cpp \
                       src/Draw.cpp \
                       src/Font/FontAtlas.cpp \
                       src/Font/FontBase.cpp \
                       src/Font/Font.cpp \
                       src/Font/FontModel.cpp \
                       src/Font/Glyph.cpp \
                       src/Shader/FragmentShader.cpp \
                       src/Shader/GeometryShader.cpp \
                       src/Shader/VertexShader.cpp \
                       src/Shader/Program.cpp \
                       src/Shader/VertexBuffer.cpp \
                       src/Shader/VertexArray.cpp \
                       src/Shader/RenderBuffer.cpp \
                       src/Shader/ProgramLocation.cpp \
                       src/Shader/FrameBuffer.cpp \
                       src/Shader/Shader.cpp \
                       src/Shader/Sprite/ShaderSprite.cpp \
                       src/Shader/Sprite/ShaderSpriteBatch.cpp \
                       src/Shader/Sprite/ShaderSpriteBatchEntry.cpp \
                       src/Shader/Sprite/ShaderSpriteEntry.cpp \
                       src/Shader/Text/ShaderText.cpp \
                       src/Shader/Text/ShaderTextBatch.cpp \
                       src/Shader/Text/ShaderTextBatchEntry.cpp \
                       src/Shader/Text/ShaderTextEntry.cpp \
                       src/Shape/Shape.cpp \
                       src/Shape/Rectangle.cpp \
                       src/Shape/RectangleBorder.cpp \
                       src/Shape/Line.cpp \
                       src/Shape/Disk.cpp \
                       src/Shape/Circle.cpp \
                       src/Shape/Triangle.cpp \
                       src/Shape/TriangleBorder.cpp \
                       src/Sprite/Sprite.cpp \
                       src/Sprite/SpriteBase.cpp \
                       src/Sprite/SpriteBatch.cpp \
                       src/Sprite/SpriteBatched.cpp \
                       src/Sprite/SpriteTessellator.cpp \
                       src/Text/Text.cpp \
                       src/Text/TextBase.cpp \
                       src/Text/TextBatch.cpp \
                       src/Text/TextBatched.cpp \
                       src/Text/TextTessellator.cpp \
                       src/Texture/Texture.cpp \
                       src/Window/EventsManager.cpp \
                       src/Window/Window.cpp \
                       src/Window/Monitor.cpp \
                       src/Window/VideoMode.cpp

librender_la_CXXFLAGS = -I$(srcdir)/src $(FREETYPE_CFLAGS)
librender_la_LDFLAGS = $(FREETYPE_LDFLAGS)

nobase_include_HEADERS = librender/Color.h \
                         librender/Draw.h \
                         librender/Drawable.h \
                         librender/DrawableBase.h \
                         librender/DrawableBatch.h \
                         librender/DrawableBatched.h \
                         librender/DrawableBuffers.h \
                         librender/DrawableTessellator.h \
                         librender/GL.h \
                         librender/Quaternion.cpp \
                         librender/Quaternion.h \
                         librender/Font/Font.h \
                         librender/Font/FontAtlas.h \
                         librender/Font/FontBase.h \
                         librender/Font/FontModel.h \
                         librender/Font/Glyph.h \
                         librender/Mat/Mat1.cpp \
                         librender/Mat/Mat1.h \
                         librender/Mat/Mat2.cpp \
                         librender/Mat/Mat2.h \
                         librender/Mat/Mat3.cpp \
                         librender/Mat/Mat3.h \
                         librender/Mat/Mat4.cpp \
                         librender/Mat/Mat4.h \
                         librender/Shader/FragmentShader.h \
                         librender/Shader/GeometryShader.h \
                         librender/Shader/VertexShader.h \
                         librender/Shader/Program.h \
                         librender/Shader/VertexBuffer.h \
                         librender/Shader/VertexArray.h \
                         librender/Shader/RenderBuffer.h \
                         librender/Shader/ProgramLocation.h \
                         librender/Shader/FrameBuffer.h \
                         librender/Shader/Shader.h \
                         librender/Shader/Sprite/ShaderSprite.h \
                         librender/Shader/Sprite/ShaderSpriteBatch.h \
                         librender/Shader/Sprite/ShaderSpriteBatchEntry.h \
                         librender/Shader/Sprite/ShaderSpriteEntry.h \
                         librender/Shader/Text/ShaderText.h \
                         librender/Shader/Text/ShaderTextBatch.h \
                         librender/Shader/Text/ShaderTextBatchEntry.h \
                         librender/Shader/Text/ShaderTextEntry.h \
                         librender/Shape/Shape.h \
                         librender/Shape/Rectangle.h \
                         librender/Shape/RectangleBorder.h \
                         librender/Shape/Line.h \
                         librender/Shape/Disk.h \
                         librender/Shape/Circle.h \
                         librender/Shape/Triangle.h \
                         librender/Shape/TriangleBorder.h \
                         librender/Sprite/Sprite.h \
                         librender/Sprite/SpriteBase.h \
                         librender/Sprite/SpriteBatch.h \
                         librender/Sprite/SpriteBatched.h \
                         librender/Sprite/SpriteTessellator.h \
                         librender/Text/Text.h \
                         librender/Text/TextBase.h \
                         librender/Text/TextBatch.h \
                         librender/Text/TextBatched.h \
                         librender/Text/TextTessellator.h \
                         librender/Texture/Texture.h \
                         librender/Vec/Vec1.cpp \
                         librender/Vec/Vec1.h \
                         librender/Vec/Vec2.cpp \
                         librender/Vec/Vec2.h \
                         librender/Vec/Vec3.cpp \
                         librender/Vec/Vec3.h \
                         librender/Vec/Vec4.cpp \
                         librender/Vec/Vec4.h \
                         librender/Window/Events.h \
                         librender/Window/EventsManager.h \
                         librender/Window/Window.h \
                         librender/Window/Monitor.h \
                         librender/Window/VideoMode.h

EXTRA_DIST = LICENSE
AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4
