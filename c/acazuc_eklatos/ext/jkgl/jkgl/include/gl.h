#ifndef JKGL_GL_H
#define JKGL_GL_H

#include <stdint.h>
#include <stddef.h>

#define GL_NONE 0x0000

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

#define GL_CULL_FACE      0x0B44
#define GL_CULL_FACE_MODE 0x0B45
#define GL_FRONT_FACE     0x0B46

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

#define GL_STENCIL_TEST            0x0B90
#define GL_STENCIL_CLEAR_VALUE     0x0B91
#define GL_STENCIL_FUNC            0x0B92
#define GL_STENCIL_VALUE_MASK      0x0B93
#define GL_STENCIL_FAIL            0x0B94
#define GL_STENCIL_PASS_DEPTH_FAIL 0x0B95
#define GL_STENCIL_PASS_DEPTH_PASS 0x0B96
#define GL_STENCIL_REF             0x0B97
#define GL_STENCIL_WRITEMASK       0x0B98

#define GL_MATRIX_MODE            0x0BA0
#define GL_VIEWPORT               0x0BA2
#define GL_MODELVIEW_STACK_DEPTH  0x0BA3
#define GL_PROJECTION_STACK_DEPTH 0x0BA4
#define GL_MODELVIEW_MATRIX       0x0BA6
#define GL_PROJECTION_MATRIX      0x0BA7

#define GL_BLEND 0x0BE2

#define GL_LOGIC_OP_MODE  0x0BF0
#define GL_COLOR_LOGIC_OP 0x0BF2

#define GL_DRAW_BUFFER 0x0C01

#define GL_SCISSOR_BOX  0x0C10
#define GL_SCISSOR_TEST 0x0C11

#define GL_COLOR_CLEAR_VALUE 0x0C22
#define GL_COLOR_WRITEMASK   0x0C23

#define GL_MAX_LIGHTS                 0x0D31
#define GL_MAX_TEXTURE_SIZE           0x0D33
#define GL_MAX_MODELVIEW_STACK_DEPTH  0x0D36
#define GL_MAX_PROJECTION_STACK_DEPTH 0x0D38

#define GL_TEXTURE_1D 0x0DE0
#define GL_TEXTURE_2D 0x0DE1

#define GL_TEXTURE_BORDER_COLOR 0x1004
#define GL_TEXTURE_TARGET       0x1006

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

#define GL_COMPILE             0x1300
#define GL_COMPILE_AND_EXECUTE 0x1301

#define GL_BYTE           0x1400
#define GL_UNSIGNED_BYTE  0x1401
#define GL_SHORT          0x1402
#define GL_UNSIGNED_SHORT 0x1403
#define GL_INT            0x1404
#define GL_UNSIGNED_INT   0x1405
#define GL_FLOAT          0x1406
#define GL_DOUBLE         0x140A
#define GL_HALF_FLOAT     0x140B

#define GL_CLEAR         0x1500
#define GL_AND           0x1501
#define GL_AND_REVERSE   0x1502
#define GL_COPY          0x1503
#define GL_AND_INVERTED  0x1504
#define GL_NOOP          0x1505
#define GL_XOR           0x1506
#define GL_OR            0x1507
#define GL_NOR           0x1508
#define GL_EQUIV         0x1509
#define GL_INVERT        0x150A
#define GL_OR_REVERSE    0x150B
#define GL_COPY_INVERTED 0x150C
#define GL_OR_INVERTED   0x150D
#define GL_NAND          0x150E
#define GL_SET           0x150F

#define GL_EMISSION            0x1600
#define GL_SHININESS           0x1601
#define GL_AMBIENT_AND_DIFFUSE 0x1602

#define GL_MODELVIEW  0x1700
#define GL_PROJECTION 0x1701
#define GL_TEXTURE    0x1702

#define GL_COLOR   0x1800
#define GL_DEPTH   0x1801
#define GL_STENCIL 0x1802

#define GL_STENCIL_INDEX   0x1901
#define GL_DEPTH_COMPONENT 0x1902
#define GL_RED             0x1903
#define GL_GREEN           0x1904
#define GL_BLUE            0x1905
#define GL_ALPHA           0x1906
#define GL_RGB             0x1907
#define GL_RGBA            0x1908

#define GL_POINT 0x1B00
#define GL_LINE  0x1B01
#define GL_FILL  0x1B02

#define GL_FLAT   0x1D00
#define GL_SMOOTH 0x1D01

#define GL_KEEP    0x1E00
#define GL_REPLACE 0x1E01
#define GL_INCR    0x1E02
#define GL_DECR    0x1E03

#define GL_VENDOR     0x1F00
#define GL_RENDERER   0x1F01
#define GL_VERSION    0x1F02
#define GL_EXTENSIONS 0x1F03

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

#define GL_V2F             0x2A20
#define GL_V3F             0x2A21
#define GL_C4UB_V2F        0x2A22
#define GL_C4UB_V3F        0x2A23
#define GL_C3F_V3F         0x2A24
#define GL_N3F_V3F         0x2A25
#define GL_C4F_N3F_V3F     0x2A26
#define GL_T2F_V3F         0x2A27
#define GL_T4F_V4F         0x2A28
#define GL_T2F_C4UB_V3F    0x2A29
#define GL_T2F_C3F_V3F     0x2A2A
#define GL_T2F_N3F_V3F     0x2A2B
#define GL_T2F_C4F_N3F_V3F 0x2A2C
#define GL_T4F_C4F_N3F_V4F 0x2A2D

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
#define GL_FUNC_SUBTRACT            0x800A
#define GL_FUNC_REVERSE_SUBTRACT    0x800B

#define GL_UNSIGNED_SHORT_4_4_4_4 0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1 0x8034

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

#define GL_TEXTURE_WRAP_R      0x8072
#define GL_MAX_3D_TEXTURE_SIZE 0x8073

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

#define GL_MULTISAMPLE 0x809D

#define GL_BLEND_DST_RGB   0x80C8
#define GL_BLEND_SRC_RGB   0x80C9
#define GL_BLEND_DST_ALPHA 0x80CA
#define GL_BLEND_SRC_ALPHA 0x80CB

#define GL_BGR  0x80E0
#define GL_BGRA 0x80E1

#define GL_CLAMP_TO_BORDER    0x812D
#define GL_CLAMP_TO_EDGE      0x812F
#define GL_TEXTURE_MIN_LOD    0x813A
#define GL_TEXTURE_MAX_LOD    0x813B
#define GL_TEXTURE_BASE_LEVEL 0x813C
#define GL_TEXTURE_MAX_LEVEL  0x813D

#define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING 0x8210
#define GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE 0x8211
#define GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE       0x8212
#define GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE     0x8213
#define GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE      0x8214
#define GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE     0x8215
#define GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE     0x8216
#define GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE   0x8217
#define GL_FRAMEBUFFER_DEFAULT                   0x8218
#define GL_FRAMEBUFFER_UNDEFINED                 0x8219
#define GL_DEPTH_STENCIL_ATTACHMENT              0x821A
#define GL_MAJOR_VERSION                         0x821B
#define GL_MINOR_VERSION                         0x821C
#define GL_NUM_EXTENSIONS                        0x821D

#define GL_COMPRESSED_RED 0x8225
#define GL_COMPRESSED_RG  0x8226
#define GL_RG             0x8227
#define GL_RG_INTEGER     0x8228
#define GL_R8             0x8229
#define GL_R16            0x822A
#define GL_RG8            0x822B
#define GL_RG16           0x822C
#define GL_R16F           0x822D
#define GL_R32F           0x822E
#define GL_RG16F          0x822F
#define GL_RG32F          0x8230
#define GL_R8I            0x8231
#define GL_R8UI           0x8232
#define GL_R16I           0x8233
#define GL_R16UI          0x8234
#define GL_R32I           0x8235
#define GL_R32UI          0x8236
#define GL_RG8I           0x8237
#define GL_RG8UI          0x8238
#define GL_RG16I          0x8239
#define GL_RG16UI         0x823A
#define GL_RG32I          0x823B
#define GL_RG32UI         0x823C

#define GL_VERTEX_BINDING_DIVISOR            0x82D6
#define GL_VERTEX_BINDING_OFFSET             0x82D7
#define GL_VERTEX_BINDING_STRIDE             0x82D8
#define GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET 0x82D9
#define GL_MAX_VERTEX_ATTRIB_BINDINGS        0x82DA
#define GL_MAX_VERTEX_ATTRIB_STRIDE          0x82E5

#define GL_UNSIGNED_SHORT_5_6_5       0x8363
#define GL_UNSIGNED_SHORT_5_6_5_REV   0x8364
#define GL_UNSIGNED_SHORT_4_4_4_4_REV 0x8365
#define GL_UNSIGNED_SHORT_1_5_5_5_REV 0x8366

#define GL_MIRRORED_REPEAT 0x8370

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT  0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3

#define GL_FOG_COORD_SRC  0x8450
#define GL_FOG_COORD      0x8451
#define GL_FRAGMENT_DEPTH 0x8452

#define GL_TEXTURE0       0x84C0
#define GL_TEXTURE1       0x84C1
#define GL_TEXTURE2       0x84C2
#define GL_TEXTURE3       0x84C3
#define GL_TEXTURE4       0x84C4
#define GL_TEXTURE5       0x84C5
#define GL_TEXTURE6       0x84C6
#define GL_TEXTURE7       0x84C7
#define GL_TEXTURE8       0x84C8
#define GL_TEXTURE9       0x84C9
#define GL_TEXTURE10      0x84CA
#define GL_TEXTURE11      0x84CB
#define GL_TEXTURE12      0x84CC
#define GL_TEXTURE13      0x84CD
#define GL_TEXTURE14      0x84CE
#define GL_TEXTURE15      0x84CF
#define GL_TEXTURE16      0x84D0
#define GL_TEXTURE17      0x84D1
#define GL_TEXTURE18      0x84D2
#define GL_TEXTURE19      0x84D3
#define GL_TEXTURE20      0x84D4
#define GL_TEXTURE21      0x84D5
#define GL_TEXTURE22      0x84D6
#define GL_TEXTURE23      0x84D7
#define GL_TEXTURE24      0x84D8
#define GL_TEXTURE25      0x84D9
#define GL_TEXTURE26      0x84DA
#define GL_TEXTURE27      0x84DB
#define GL_TEXTURE28      0x84DC
#define GL_TEXTURE29      0x84DD
#define GL_TEXTURE30      0x84DE
#define GL_TEXTURE31      0x84DF
#define GL_ACTIVE_TEXTURE 0x84E0

#define GL_MAX_RENDERBUFFER_SIZE 0x84E8

#define GL_COMPRESSED_RGB  0x84ED
#define GL_COMPRESSED_RGBA 0x84EE

#define GL_DEPTH_STENCIL              0x84F9
#define GL_UNSIGNED_INT_24_8          0x84FA
#define GL_TEXTURE_MAX_ANISOTROPY     0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY 0x84FF

#define GL_TEXTURE_LOD_BIAS 0x8501

#define GL_INCR_WRAP 0x8507
#define GL_DECR_WRAP 0x8508

#define GL_SRC1_ALPHA 0x8589

#define GL_VERTEX_ARRAY_BINDING 0x85B5

#define GL_DEPTH_CLAMP 0x864F

#define GL_MIRROR_CLAMP_TO_EDGE 0x8743

#define GL_STENCIL_BACK_FUNC            0x8800
#define GL_STENCIL_BACK_FAIL            0x8801
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL 0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_PASS 0x8803

#define GL_RGBA32F 0x8814
#define GL_RGB32F  0x8815
#define GL_RGBA16F 0x881A
#define GL_RGB16F  0x881B

#define GL_MAX_DRAW_BUFFERS 0x8824
#define GL_DRAW_BUFFER0     0x8825
#define GL_DRAW_BUFFER1     0x8826
#define GL_DRAW_BUFFER2     0x8827
#define GL_DRAW_BUFFER3     0x8828
#define GL_DRAW_BUFFER4     0x8829
#define GL_DRAW_BUFFER5     0x882A
#define GL_DRAW_BUFFER6     0x882B
#define GL_DRAW_BUFFER7     0x882C
#define GL_DRAW_BUFFER8     0x882D
#define GL_DRAW_BUFFER9     0x882E
#define GL_DRAW_BUFFER10    0x882F
#define GL_DRAW_BUFFER11    0x8830
#define GL_DRAW_BUFFER12    0x8831
#define GL_DRAW_BUFFER13    0x8832
#define GL_DRAW_BUFFER14    0x8833
#define GL_DRAW_BUFFER15    0x8834

#define GL_BLEND_EQUATION_ALPHA 0x883D

#define GL_TEXTURE_COMPARE_MODE   0x884C
#define GL_TEXTURE_COMPARE_FUNC   0x884D
#define GL_COMPARE_REF_TO_TEXTURE 0x884E

#define GL_MAX_VERTEX_ATTRIBS 0x8869

#define GL_ARRAY_BUFFER                 0x8892
#define GL_ELEMENT_ARRAY_BUFFER         0x8893
#define GL_ARRAY_BUFFER_BINDING         0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING 0x8895

#define GL_STREAM_DRAW  0x88E0
#define GL_STREAM_READ  0x88E1
#define GL_STREAM_COPY  0x88E2
#define GL_STATIC_DRAW  0x88E4
#define GL_STATIC_READ  0x88E5
#define GL_STATIC_COPY  0x88E6
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_DYNAMIC_READ 0x88E9
#define GL_DYNAMIC_COPY 0x88EA

#define GL_DEPTH24_STENCIL8 0x88F0

#define GL_SRC1_COLOR           0x88F9
#define GL_ONE_MINUS_SRC1_COLOR 0x88FA
#define GL_ONE_MINUS_SRC1_ALPHA 0x88FB

#define GL_UNIFORM_BUFFER 0x8A11

#define GL_UNIFORM_BUFFER_BINDING 0x8A28

#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER   0x8B31

#define GL_COMPILE_STATUS  0x8B81
#define GL_LINK_STATUS     0x8B82
#define GL_CURRENT_PROGRAM 0x8B8D

#define GL_UNSIGNED_NORMALIZED 0x8C17
#define GL_TEXTURE_1D_ARRAY    0x8C18
#define GL_TEXTURE_2D_ARRAY    0x8C1A

#define GL_R11F_G11F_B10F        0x8C3A
#define GL_RGB9_E5               0x8C3D
#define GL_SRGB8                 0x8C41
#define GL_SRGB8_ALPHA8          0x8C43
#define GL_COMPRESSED_SRGB       0x8C48
#define GL_COMPRESSED_SRGB_ALPHA 0x8C49

#define GL_RASTERIZER_DISCARD 0x8C89

#define GL_STENCIL_BACK_REF         0x8CA3
#define GL_STENCIL_BACK_VALUE_MASK  0x8CA4
#define GL_STENCIL_BACK_WRITEMASK   0x8CA5
#define GL_DRAW_FRAMEBUFFER_BINDING 0x8CA6
#define GL_READ_FRAMEBUFFER         0x8CA8
#define GL_DRAW_FRAMEBUFFER         0x8CA9
#define GL_READ_FRAMEBUFFER_BINDING 0x8CAA

#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE           0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME           0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL         0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE 0x8CD3
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER         0x8CD4

#define GL_MAX_COLOR_ATTACHMENTS 0x8CDF
#define GL_COLOR_ATTACHMENT0     0x8CE0
#define GL_COLOR_ATTACHMENT1     0x8CE1
#define GL_COLOR_ATTACHMENT2     0x8CE2
#define GL_COLOR_ATTACHMENT3     0x8CE3
#define GL_COLOR_ATTACHMENT4     0x8CE4
#define GL_COLOR_ATTACHMENT5     0x8CE5
#define GL_COLOR_ATTACHMENT6     0x8CE6
#define GL_COLOR_ATTACHMENT7     0x8CE7
#define GL_COLOR_ATTACHMENT8     0x8CE8
#define GL_COLOR_ATTACHMENT9     0x8CE9
#define GL_COLOR_ATTACHMENT10    0x8CEA
#define GL_COLOR_ATTACHMENT11    0x8CEB
#define GL_COLOR_ATTACHMENT12    0x8CEC
#define GL_COLOR_ATTACHMENT13    0x8CED
#define GL_COLOR_ATTACHMENT14    0x8CEE
#define GL_COLOR_ATTACHMENT15    0x8CEF
#define GL_COLOR_ATTACHMENT16    0x8CF0
#define GL_COLOR_ATTACHMENT17    0x8CF1
#define GL_COLOR_ATTACHMENT18    0x8CF2
#define GL_COLOR_ATTACHMENT19    0x8CF3
#define GL_COLOR_ATTACHMENT20    0x8CF4
#define GL_COLOR_ATTACHMENT21    0x8CF5
#define GL_COLOR_ATTACHMENT22    0x8CF6
#define GL_COLOR_ATTACHMENT23    0x8CF7
#define GL_COLOR_ATTACHMENT24    0x8CF8
#define GL_COLOR_ATTACHMENT25    0x8CF9
#define GL_COLOR_ATTACHMENT26    0x8CFA
#define GL_COLOR_ATTACHMENT27    0x8CFB
#define GL_COLOR_ATTACHMENT28    0x8CFC
#define GL_COLOR_ATTACHMENT29    0x8CFD
#define GL_COLOR_ATTACHMENT30    0x8CFE
#define GL_COLOR_ATTACHMENT31    0x8CFF
#define GL_DEPTH_ATTACHMENT      0x8D00
#define GL_STENCIL_ATTACHMENT    0x8D20

#define GL_FRAMEBUFFER  0x8D40
#define GL_RENDERBUFFER 0x8D41

#define GL_RGB565                        0x8D62
#define GL_PRIMITIVE_RESTART_FIXED_INDEX 0x8D69
#define GL_RGBA32UI                      0x8D70
#define GL_RGB32UI                       0x8D71
#define GL_RGBA16UI                      0x8D76
#define GL_RGB16UI                       0x8D77
#define GL_RGBA8UI                       0x8D7C
#define GL_RGB8UI                        0x8D7D
#define GL_RGBA32I                       0x8D82
#define GL_RGB32I                        0x8D83
#define GL_RGBA16I                       0x8D88
#define GL_RGB16I                        0x8D89
#define GL_RGBA8I                        0x8D8E
#define GL_RGB8I                         0x8D8F
#define GL_RED_INTEGER                   0x8D94
#define GL_RGB_INTEGER                   0x8D98
#define GL_RGBA_INTEGER                  0x8D99
#define GL_BGR_INTEGER                   0x8D9A
#define GL_BGRA_INTEGER                  0x8D9B

#define GL_FRAMEBUFFER_ATTACHMENT_LAYERED 0x8DA7

#define GL_COMPRESSED_RED_RGTC1        0x8DBB
#define GL_COMPRESSED_SIGNED_RED_RGTC1 0x8DBC
#define GL_COMPRESSED_RG_RGTC2         0x8DBD
#define GL_COMPRESSED_SIGNED_RG_RGTC2  0x8DBE

#define GL_TEXTURE_SWIZZLE_R    0x8E42
#define GL_TEXTURE_SWIZZLE_G    0x8E43
#define GL_TEXTURE_SWIZZLE_B    0x8E44
#define GL_TEXTURE_SWIZZLE_A    0x8E45
#define GL_TEXTURE_SWIZZLE_RGBA 0x8E46

#define GL_COMPRESSED_RGBA_BPTC_UNORM         0x8E8C
#define GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM   0x8E8D
#define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT   0x8E8E
#define GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT 0x8E8F

#define GL_COPY_READ_BUFFER          0x8F36
#define GL_COPY_READ_BUFFER_BINDING  0x8F36
#define GL_COPY_WRITE_BUFFER         0x8F37
#define GL_COPY_WRITE_BUFFER_BINDING 0x8F37

#define GL_VERTEX_BINDING_BUFFER 0x8F4F

#define GL_R8_SNORM                0x8F94
#define GL_RG8_SNORM               0x8F95
#define GL_RGB8_SNORM              0x8F96
#define GL_RGBA8_SNORM             0x8F97
#define GL_R16_SNORM               0x8F98
#define GL_RG16_SNORM              0x8F99
#define GL_RGB16_SNORM             0x8F9A
#define GL_SIGNED_NORMALIZED       0x8F9C
#define GL_PRIMITIVE_RESTART       0x8F9D
#define GL_PRIMITIVE_RESTART_INDEX 0x8F9E

#define GL_RGB10_A2UI 0x906F

#define GL_CUBIC                0x9139
#define GL_CUBIC_MIPMAP_NEAREST 0x913A
#define GL_CUBIC_MIPMAP_LINEAR  0x913B

#define GL_MAX_FRAMEBUFFER_WIDTH  0x9315
#define GL_MAX_FRAMEBUFFER_HEIGHT 0x9316

#define GL_POINTS         0x0000
#define GL_LINES          0x0001
#define GL_LINE_LOOP      0x0002
#define GL_LINE_STRIP     0x0003
#define GL_TRIANGLES      0x0004
#define GL_TRIANGLE_STRIP 0x0005
#define GL_TRIANGLE_FAN   0x0006
#define GL_QUADS          0x0007
#define GL_QUAD_STRIP     0x0008
#define GL_POLYGON        0x0009

#define GL_COLOR_BUFFER_BIT   0x4000
#define GL_DEPTH_BUFFER_BIT   0x0100
#define GL_STENCIL_BUFFER_BIT 0x0400

#define GL_CLIENT_VERTEX_ARRAY_BIT 0x0002
#define GL_CLIENT_ALL_ATTRIB_BITS  0xFFFF

typedef uint32_t  GLbitfield;
typedef int8_t    GLbyte;
typedef uint8_t   GLboolean;
typedef char      GLchar;
typedef double    GLdouble;
typedef uint32_t  GLenum;
typedef float     GLfloat;
typedef int32_t   GLint;
typedef int64_t   GLint64;
typedef ptrdiff_t GLintptr;
typedef int64_t   GLlong;
typedef int16_t   GLshort;
typedef int32_t   GLsizei;
typedef ptrdiff_t GLsizeiptr;
typedef uint8_t   GLubyte;
typedef uint32_t  GLuint;
typedef uint64_t  GLulong;
typedef uint16_t  GLushort;
typedef void      GLvoid;

struct gl_ctx;

void gl_ctx_set(struct gl_ctx *ctx);
struct gl_ctx *gl_ctx_new(void);
void gl_ctx_free(struct gl_ctx *ctx);
void gl_ctx_resize(struct gl_ctx *ctx, GLuint width, GLuint height);
void gl_ctx_swap_buffers(struct gl_ctx *ctx);
void gl_sample(uint32_t id, const float coord[3], float color[4]);

/* blend */
void glBlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void glBlendEquation(GLenum mode);
void glBlendEquationi(GLuint buf, GLenum mode);
void glBlendEquationSeparate(GLenum mode_rgb, GLenum mode_alpha);
void glBlendEquationSeparatei(GLuint buf, GLenum mode_rgb, GLenum mode_alpha);
void glBlendFunc(GLenum s_factor, GLenum d_factor);
void glBlendFunci(GLuint buf, GLenum s_factor, GLenum d_factor);
void glBlendFuncSeparate(GLenum src_rgb, GLenum dst_rgb, GLenum src_alpha, GLenum dst_alpha);
void glBlendFuncSeparatei(GLuint buf, GLenum src_rgb, GLenum dst_rgb, GLenum src_alpha, GLenum dst_alpha);
void glLogicOp(GLenum opcode);
void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
void glColorMaski(GLuint buf, GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);

/* buffer */
void glGenBuffers(GLsizei n, GLuint *buffers);
void glCreateBuffers(GLsizei n, GLuint *buffers);
void glDeleteBuffers(GLsizei n, const GLuint *buffers);
GLboolean glIsBuffer(GLuint buffer);
void glBufferStorage(GLenum target, GLsizeiptr size, const GLvoid *data, GLbitfield flags);
void glNamedBufferStorage(GLuint buffer, GLsizeiptr size, const GLvoid *data, GLbitfield flags);
void glBindBuffer(GLenum target, GLuint buffer);
void glBufferData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
void glNamedBufferData(GLuint id, GLsizeiptr size, const GLvoid *data, GLenum usage);
void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data);
void glNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr size, const GLvoid *data);
void glCopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
void glCopyNamedBufferSubData(GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
void glClearBufferSubData(GLenum target, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const GLvoid *data);
void glClearNamedBufferSubData(GLuint buffer, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const GLvoid *data);
void glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data);
void glGetNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr size, GLvoid *data);
void glBindBufferBase(GLenum target, GLuint index, GLuint buffer);
void glBindBuffersBase(GLenum target, GLuint first, GLsizei count, const GLuint *buffers);
void glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
void glBindBuffersRange(GLenum target, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLintptr *sizes);

/* clear */
void glClear(GLbitfield mask);
void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void glClearDepth(GLdouble depth);
void glClearDepthf(GLfloat depth);
void glClearStencil(GLint stencil);

/* depth stencil */
void glDepthFunc(GLenum func);
void glDepthMask(GLboolean flag);
void glDepthRange(GLdouble near_val, GLdouble far_val);
void glDepthRangef(GLfloat near_val, GLfloat far_val);
void glStencilMask(GLuint mask);
void glStencilMaskSeparate(GLenum face, GLuint mask);
void glStencilFunc(GLenum func, GLint ref, GLuint mask);
void glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask);
void glStencilOp(GLenum sfail, GLenum dpfail, GLenum dppass);
void glStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);

/* draw */
void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
void glDrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLint basevertex);
void glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei instancecount);
void glDrawElementsInstancedBaseVertex(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei instancecount, GLint basevertex);
void glDrawElementsInstancedBaseInstance(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primcount, GLuint baseinstance);
void glDrawElementsInstancedBaseVertexBaseInstance(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance);
void glMultiDrawElements(GLenum mode, const GLsizei *count, GLenum type, const GLvoid * const *indices, GLsizei drawcount);
void glMultiDrawElementsBaseVertex(GLenum mode, const GLsizei *count, GLenum type, const GLvoid * const *indices, GLsizei drawcount, const GLint *basevertex);
void glDrawArrays(GLenum mode, GLint first, GLsizei count);
void glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
void glDrawArraysInstancedBaseInstance(GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance);
void glMultiDrawArrays(GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount);
void glPrimitiveRestartIndex(GLuint index);

/* fixed */
void glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void glNormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer);
void glInterleavedArrays(GLenum format, GLsizei stride, const GLvoid *pointer);
void glEnableClientState(GLenum cap);
void glDisableClientState(GLenum cap);
void glPushClientAttrib(GLbitfield mask);
void glPopClientAttrib(void);

/* fog */
void glFogf(GLenum pname, GLfloat param);
void glFogi(GLenum pname, GLint param);
void glFogfv(GLenum pname, GLfloat *param);
void glFogiv(GLenum pname, GLint *param);

/* framebuffer */
void glGenFramebuffers(GLsizei n, GLuint *ids);
void glCreateFramebuffers(GLsizei n, GLuint *framebuffers);
void glDeleteFramebuffers(GLsizei n, GLuint *framebuffers);
GLboolean glIsFramebuffer(GLuint framebuffer);
void glBindFramebuffer(GLenum target, GLuint framebuffer);
void glFramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level);
void glFramebufferTexture1D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
void glFramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint layer);
void glNamedFramebufferTexture(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level);
void glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
void glNamedFramebufferRenderbuffer(GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
void glClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint *value);
void glClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint *value);
void glClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat *value);
void glClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
void glClearNamedFramebufferiv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint *value);
void glClearNamedFramebufferuiv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint *value);
void glClearNamedFramebufferfv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat *value);
void glClearNamedFramebufferfi(GLuint framebuffer, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
void glDrawBuffers(GLsizei n, const GLenum *bufs);
void glNamedFramebufferDrawBuffers(GLuint framebuffer, GLsizei n, const GLenum *bufs);
void glDrawBuffer(GLenum buf);
void glNamedFramebufferDrawBuffer(GLuint framebuffer, GLenum buf);
void glReadBuffer(GLenum mode);
void glNamedFramebufferReadBuffer(GLuint framebuffer, GLenum mode);
GLenum glCheckFramebufferStatus(GLenum target);
GLenum glCheckNamedFramebufferStatus(GLuint framebuffer, GLenum target);
void glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint *params);
void glGetNamedFramebufferAttachmentParameteriv(GLuint framebuffer, GLenum attachment, GLenum pname, GLint *params);

/* get */
void glGetBooleanv(GLenum pname, GLboolean *params);
void glGetDoublev(GLenum pname, GLdouble *params);
void glGetFloatv(GLenum pname, GLfloat *params);
void glGetIntegerv(GLenum pname, GLint *params);
void glGetInteger64v(GLenum pname, GLint64 *params);
void glGetPointerv(GLenum pname, GLvoid **params);
void glGetBooleani_v(GLenum target, GLuint index, GLboolean *data);
void glGetIntegeri_v(GLenum target, GLuint index, GLint *data);
void glGetFloati_v(GLenum target, GLuint index, GLfloat *data);
void glGetDoublei_v(GLenum target, GLuint index, GLdouble *data);
void glGetInteger64i_v(GLenum target, GLuint index, GLint64 *data);
const GLubyte *glGetString(GLenum name);
const GLubyte *glGetStringi(GLenum name, GLuint index);

/* immediate */
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

/* light */
void glLightf(GLenum light, GLenum pname, GLfloat param);
void glLighti(GLenum light, GLenum pname, GLint param);
void glLightfv(GLenum light, GLenum pname, GLfloat *params);
void glLightiv(GLenum light, GLenum pname, GLint *params);
void glGetLightfv(GLenum light, GLenum pname, GLfloat *params);
void glGetLightiv(GLenum light, GLenum pname, GLint *params);

/* list */
GLuint glGenLists(GLsizei range);
void glDeleteLists(GLuint list, GLsizei range);
GLboolean glIsList(GLuint list);
void glListBase(GLuint base);
void glNewList(GLuint list, GLenum mode);
void glEndList(void);
void glCallList(GLuint list);
void glCallLists(GLsizei n, GLenum type, const GLvoid *lists);

/* material */
void glMaterialf(GLenum face, GLenum pname, GLfloat param);
void glMateriali(GLenum face, GLenum pname, GLint param);
void glMaterialfv(GLenum face, GLenum pname, GLfloat *params);
void glMaterialiv(GLenum face, GLenum pname, GLint *params);
void glGetMaterialfv(GLenum face, GLenum pname, GLfloat *params);
void glGetMaterialiv(GLenum face, GLenum pname, GLint *params);

/* matrix */
void glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near, GLdouble far);
void glLoadMatrixd(const GLdouble *m);
void glLoadMatrixf(const GLfloat *m);
void glLoadIdentity(void);
void glLoadTransposeMatrixd(const GLdouble *m);
void glLoadTransposeMatrixf(const GLfloat *m);
void glMatrixMode(GLenum mode);
void glMultMatrixd(const GLdouble *m);
void glMultMatrixf(const GLfloat *m);
void glMultTransposeMatrixd(const GLdouble *m);
void glMultTransposeMatrixf(const GLfloat *m);
void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near, GLdouble far);
void glPopMatrix(void);
void glPushMatrix(void);
void glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void glScaled(GLdouble x, GLdouble y, GLdouble z);
void glScalef(GLfloat x, GLfloat y, GLfloat z);
void glTranslated(GLdouble x, GLdouble y, GLdouble z);
void glTranslatef(GLfloat x, GLfloat y, GLfloat z);

/* misc */
void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
void glShadeModel(GLenum mode);
GLenum glGetError(void);
void glScissor(GLint x, GLint y, GLsizei width, GLsizei height);
void glFlush(void);
void glFinish(void);

/* opt */
void glEnable(GLenum cap);
void glDisable(GLenum cap);
GLboolean glIsEnabled(GLenum cap);

/* program */
GLuint glCreateProgram(void);
void glDeleteProgram(GLuint program);
void glAttachShader(GLuint program, GLuint shader);
void glLinkProgram(GLuint program);
void glUseProgram(GLuint program);
void glGetProgramiv(GLuint program, GLenum pname, GLint *params);
GLboolean glIsProgram(GLuint program);

/* rasterizer */
void glLineWidth(GLfloat width);
void glPointSize(GLfloat size);
void glFrontFace(GLenum mode);
void glCullFace(GLenum mode);
void glPolygonMode(GLenum face, GLenum mode);

/* read */
void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *data);

/* renderbuffer */
void glGenRenderbuffers(GLsizei n, GLuint *renderbuffers);
void glDeleteRenderbuffers(GLsizei n, GLuint *renderbuffers);
void glBindRenderbuffer(GLenum target, GLuint renderbuffer);
void glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
void glNamedRenderbufferStorage(GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height);

/* sampler */
void glGenSamplers(GLsizei n, GLuint *samplers);
void glCreateSamplers(GLsizei n, GLuint *samplers);
void glDeleteSamplers(GLsizei n, const GLuint *samplers);
GLboolean glIsSampler(GLuint id);
void glBindSampler(GLuint unit, GLuint sampler);
void glBindSamplers(GLuint first, GLsizei count, const GLuint *samplers);
void glSamplerParameterf(GLuint sampler, GLenum pname, GLfloat param);
void glSamplerParameteri(GLuint sampler, GLenum pname, GLint param);
void glSamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat *params);
void glSamplerParameteriv(GLuint sampler, GLenum pname, const GLint *params);
void glGetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat *params);
void glGetSamplerParameteriv(GLuint sampler, GLenum pname, GLint *params);

/* shader */
GLuint glCreateShader(GLenum type);
void glShaderSource(GLuint shader, GLsizei count, const GLchar **string, const GLuint *length);
void glCompileShader(GLuint shader);
void glDeleteShader(GLuint shader);
void glGetShaderiv(GLuint shader, GLenum pname, GLint *params);
GLboolean glIsShader(GLuint shader);

/* texture */
void glGenTextures(GLsizei n, GLuint *textures);
void glCreateTextures(GLenum target, GLsizei n, GLuint *textures);
void glDeleteTextures(GLsizei n, const GLuint *textures);
void glBindTexture(GLenum target, GLuint texture);
void glBindTextureUnit(GLuint unit, GLuint texture);
void glBindTextures(GLuint first, GLsizei count, const GLuint *textures);
void glTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *data);
void glTexStorage1D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
void glTextureStorage1D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width);
void glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
void glTextureSubImage1D(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *data);
void glTexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
void glTextureStorage2D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
void glTextureSubImage2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
void glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *data);
void glTexStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
void glTextureStorage3D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
void glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
void glTextureSubImage3D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
void glTexParameteri(GLenum target, GLenum pname, GLint param);
void glTexParameterf(GLenum target, GLenum pname, GLfloat param);
void glTexParameteriv(GLuint target, GLenum pname, const GLint *params);
void glTexParameterfv(GLuint target, GLenum pname, const GLfloat *params);
void glTextureParameteri(GLuint texture, GLenum pname, GLint param);
void glTextureParameterf(GLuint texture, GLenum pname, GLfloat param);
void glTextureParameteriv(GLuint texture, GLenum pname, const GLint *params);
void glTextureParameterfv(GLuint texture, GLenum pname, const GLfloat *params);
void glGetTexParameteriv(GLenum target, GLenum pname, GLint *params);
void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params);
void glGetTextureParameteriv(GLuint texture, GLenum pname, GLint *params);
void glGetTextureParameterfv(GLuint texture, GLenum pname, GLfloat *params);
void glActiveTexture(GLenum texture);
void glCopyImageSubData(GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth);
void glGetTextureSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, GLvoid *pixels);
void glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
void glGetnTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, GLvoid *pixels);
void glGetTextureImage(GLuint texture, GLint level, GLenum format, GLenum type, GLsizei bufSize, GLvoid *pixels);

/* vertex array */
void glGenVertexArrays(GLsizei n, GLuint *arrays);
void glCreateVertexArrays(GLsizei n, GLuint *arrays);
void glDeleteVertexArrays(GLsizei n, const GLuint *arrays);
GLboolean glIsVertexArray(GLuint array);
void glBindVertexArray(GLuint array);
void glBindVertexBuffer(GLuint binding, GLuint buffer, GLintptr offset, GLsizei stride);
void glVertexArrayVertexBuffer(GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
void glBindVertexBuffers(GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides);
void glVertexArrayVertexBuffers(GLuint vaobj, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides);
void glVertexAttribBinding(GLuint attribindex, GLuint bindingindex);
void glVertexArrayAttribBinding(GLuint vaobj, GLuint attribindex, GLuint bindingindex);
void glVertexBindingDivisor(GLuint bindingindex, GLuint divisor);
void glVertexArrayBindingDivisor(GLuint vaobj, GLuint bindingindex, GLuint divisor);
void glVertexAttribFormat(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
void glVertexAttribIFormat(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
void glVertexAttribLFormat(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
void glVertexArrayAttribFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
void glVertexArrayAttribIFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
void glVertexArrayAttribLFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
void glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void glVertexAttribLPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void glVertexAttribDivisor(GLuint index, GLuint divisor);
void glEnableVertexAttribArray(GLuint index);
void glDisableVertexAttribArray(GLuint index);
void glEnableVertexArrayAttrib(GLuint vaobj, GLuint index);
void glDisableVertexArrayAttrib(GLuint vaobj, GLuint index);

#endif
