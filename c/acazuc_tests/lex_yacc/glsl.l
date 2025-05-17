/* The OpenGL Shading Language, Version 4.60.8 */

%{

#include <stdlib.h>
#include <string.h>

#include "ast.h"

struct glsl_parser;

#include "y.glsl.h"

#define YYSTYPE GLSL_STYPE

static void multiline_comment(yyscan_t yyscanner);
static int parse_int(YYSTYPE *lval, yyscan_t yyscanner, const char *text, int base);
static int parse_uint(YYSTYPE *lval, yyscan_t yyscanner, const char *text, int base);
static int parse_float(YYSTYPE *lval, yyscan_t yyscanner, const char *text);
static int parse_double(YYSTYPE *lval, yyscan_t yyscanner, const char *text);

#define YY_EXTRA_TYPE struct user_type *

%}

%option reentrant
%option bison-bridge
%option prefix="glsl_"
%option nounput
%option noyywrap
%option never-interactive

%%

[ \t]
[\n] /* XXX backslash newline = no token separation */

\/\/.*\n
"/*" multiline_comment(yyscanner);
^#.*\n

"const"  return CONST;
"bool"   return BOOL;
"float"  return FLOAT;
"int"    return INT;
"uint"   return UINT;
"double" return DOUBLE;

"bvec2" return BVEC2;
"bvec3" return BVEC3;
"bvec4" return BVEC4;
"ivec2" return IVEC2;
"ivec3" return IVEC3;
"ivec4" return IVEC4;
"uvec2" return UVEC2;
"uvec3" return UVEC3;
"uvec4" return UVEC4;
"vec2"  return VEC2;
"vec3"  return VEC3;
"vec4"  return VEC4;

"mat2"   return MAT2;
"mat3"   return MAT3;
"mat4"   return MAT4;
"mat2x2" return MAT2X2;
"mat2x3" return MAT2X3;
"mat2x4" return MAT2X4;
"mat3x2" return MAT3X2;
"mat3x3" return MAT3X3;
"mat3x4" return MAT3X4;
"mat4x2" return MAT4X2;
"mat4x3" return MAT4X3;
"mat4x4" return MAT4X4;

"dvec2"   return DVEC2;
"dvec3"   return DVEC3;
"dvec4"   return DVEC4;
"dmat2"   return DMAT2;
"dmat3"   return DMAT3;
"dmat4"   return DMAT4;
"dmat2x2" return DMAT2X2;
"dmat2x3" return DMAT2X3;
"dmat2x4" return DMAT2X4;
"dmat3x2" return DMAT3X2;
"dmat3x3" return DMAT3X3;
"dmat3x4" return DMAT3X4;
"dmat4x2" return DMAT4X2;
"dmat4x3" return DMAT4X3;
"dmat4x4" return DMAT4X4;

"centroid"      return CENTROID;
"in"            return IN;
"out"           return OUT;
"inout"         return INOUT;
"uniform"       return UNIFORM;
"patch"         return PATCH;
"sample"        return SAMPLE;
"buffer"        return BUFFER;
"shared"        return SHARED;
"coherent"      return COHERENT;
"volatile"      return VOLATILE;
"restrict"      return RESTRICT;
"readonly"      return READONLY;
"writeonly"     return WRITEONLY;
"noperspective" return NOPERSPECTIVE;
"flat"          return FLAT;
"smooth"        return SMOOTH;
"layout"        return LAYOUT;

"atomic_uint" return ATOMIC_UINT;

"sampler1D"            return SAMPLER1D;
"sampler1DArray"       return SAMPLER1DARRAY;
"sampler1DShadow"      return SAMPLER1DSHADOW;
"sampler1DArrayShadow" return SAMPLER1DARRAYSHADOW;

"isampler1D"      return ISAMPLER1D;
"isampler1DArray" return ISAMPLER1DARRAY;
"usampler1D"      return USAMPLER1D;
"usampler1DArray" return USAMPLER1DARRAY;

"sampler2D"            return SAMPLER2D;
"sampler2DArray"       return SAMPLER2DARRAY;
"sampler2DShadow"      return SAMPLER2DSHADOW;
"sampler2DArrayShadow" return SAMPLER2DARRAYSHADOW;

"isampler2D"      return ISAMPLER2D;
"isampler2DArray" return ISAMPLER2DARRAY;
"usampler2D"      return USAMPLER2D;
"usampler2DArray" return USAMPLER2DARRAY;

"sampler2DRect"       return SAMPLER2DRECT;
"sampler2DRectShadow" return SAMPLER2DRECTSHADOW;
"isampler2DRect"      return ISAMPLER2DRECT;
"usampler2DRect"      return USAMPLER2DRECT;

"sampler2DMS"  return SAMPLER2DMS;
"isampler2DMS" return ISAMPLER2DMS;
"usampler2DMS" return USAMPLER2DMS;

"sampler2DMSArray"  return SAMPLER2DMSARRAY;
"isampler2DMSArray" return ISAMPLER2DMSARRAY;
"usampler2DMSArray" return USAMPLER2DMSARRAY;

"sampler3D"  return SAMPLER3D;
"isampler3D" return ISAMPLER3D;
"usampler3D" return USAMPLER3D;

"samplerCube"       return SAMPLERCUBE;
"samplerCubeShadow" return SAMPLERCUBESHADOW;
"isamplerCube"      return ISAMPLERCUBE;
"usamplerCube"      return USAMPLERCUBE;

"samplerCubeArray"       return SAMPLERCUBEARRAY;
"samplerCubeArrayShadow" return SAMPLERCUBEARRAYSHADOW;

"isamplerCubeArray" return ISAMPLERCUBEARRAY;
"usamplerCubeArray" return USAMPLERCUBEARRAY;

"samplerBuffer"  return SAMPLERBUFFER;
"isamplerBuffer" return ISAMPLERBUFFER;
"usamplerBuffer" return USAMPLERBUFFER;

"image1D"  return IMAGE1D;
"iimage1D" return IIMAGE1D;
"uimage1D" return UIMAGE1D;

"image1DArray"  return IMAGE1DARRAY;
"iimage1DArray" return IIMAGE1DARRAY;
"uimage1DArray" return UIMAGE1DARRAY;

"image2D"  return IMAGE2D;
"iimage2D" return IIMAGE2D;
"uimage2D" return UIMAGE2D;

"image2DArray"  return IMAGE2DARRAY;
"iimage2DArray" return IIMAGE2DARRAY;
"uimage2DArray" return UIMAGE2DARRAY;

"image2DRect"  return IMAGE2DRECT;
"iimage2DRect" return IIMAGE2DRECT;
"uimage2DRect" return UIMAGE2DRECT;

"image2DMS"  return IMAGE2DMS;
"iimage2DMS" return IIMAGE2DMS;
"uimage2DMS" return UIMAGE2DMS;

"image2DMSArray"  return IMAGE2DMSARRAY;
"iimage2DMSArray" return IIMAGE2DMSARRAY;
"uimage2DMSArray" return UIMAGE2DMSARRAY;

"image3D"  return IMAGE3D;
"iimage3D" return IIMAGE3D;
"uimage3D" return UIMAGE3D;

"imageCube"  return IMAGECUBE;
"iimageCube" return IIMAGECUBE;
"uimageCube" return UIMAGECUBE;

"imageCubeArray"  return IMAGECUBEARRAY;
"iimageCubeArray" return IIMAGECUBEARRAY;
"uimageCubeArray" return UIMAGECUBEARRAY;

"imageBuffer"  return IMAGEBUFFER;
"iimageBuffer" return IIMAGEBUFFER;
"uimageBuffer" return UIMAGEBUFFER;

"struct" return STRUCT;
"void"   return VOID;

"while"      return WHILE;
"break"      return BREAK;
"continue"   return CONTINUE;
"do"         return DO;
"else"       return ELSE;
"for"        return FOR;
"if"         return IF;
"discard"    return DISCARD;
"return"     return RETURN;
"switch"     return SWITCH;
"case"       return CASE;
"default"    return DEFAULT;
"subroutine" return SUBROUTINE;

"common"        yyterminate();
"partition"     yyterminate();
"active"        yyterminate();
"asm"           yyterminate();
"class"         yyterminate();
"union"         yyterminate();
"enum"          yyterminate();
"typedef"       yyterminate();
"template"      yyterminate();
"this"          yyterminate();
"resource"      yyterminate();
"goto"          yyterminate();
"inline"        yyterminate();
"noinline"      yyterminate();
"public"        yyterminate();
"static"        yyterminate();
"extern"        yyterminate();
"external"      yyterminate();
"interface"     yyterminate();
"long"          yyterminate();
"short"         yyterminate();
"half"          yyterminate();
"fixed"         yyterminate();
"unsigned"      yyterminate();
"superp"        yyterminate();
"input"         yyterminate();
"output"        yyterminate();
"hvec2"         yyterminate();
"hvec3"         yyterminate();
"hvec4"         yyterminate();
"fvec2"         yyterminate();
"fvec3"         yyterminate();
"fvec4"         yyterminate();
"filter"        yyterminate();
"sizeof"        yyterminate();
"cast"          yyterminate();
"namespace"     yyterminate();
"using"         yyterminate();
"sampler3DRect" yyterminate();

"<<"  return LEFT_OP;
">>"  return RIGHT_OP;
"++"  return INC_OP;
"--"  return DEC_OP;
"<="  return LE_OP;
">="  return GE_OP;
"=="  return EQ_OP;
"!="  return NE_OP;
"&&"  return AND_OP;
"||"  return OR_OP;
"^^"  return XOR_OP;
"*="  return MUL_ASSIGN;
"/="  return DIV_ASSIGN;
"+="  return ADD_ASSIGN;
"%="  return MOD_ASSIGN;
"<<=" return LEFT_ASSIGN;
">>=" return RIGHT_ASSIGN;
"&="  return AND_ASSIGN;
"^="  return XOR_ASSIGN;
"|="  return OR_ASSIGN;
"-="  return SUB_ASSIGN;
"("   return LEFT_PAREN;
")"   return RIGHT_PAREN;
"["   return LEFT_BRACKET;
"]"   return RIGHT_BRACKET;
"{"   return LEFT_BRACE;
"}"   return RIGHT_BRACE;
"."   return DOT;
","   return COMMA;
":"   return COLON;
"="   return EQUAL;
";"   return SEMICOLON;
"!"   return BANG;
"-"   return DASH;
"~"   return TILDE;
"+"   return PLUS;
"*"   return STAR;
"/"   return SLASH;
"%"   return PERCENT;
"<"   return LEFT_ANGLE;
">"   return RIGHT_ANGLE;
"|"   return VERTICAL_BAR;
"^"   return CARET;
"&"   return AMPERSAND;
"?"   return QUESTION;

"invariant"        return INVARIANT;
"precise"          return PRECISE;
"high_precision"   return HIGH_PRECISION;
"medium_precision" return MEDIUM_PRECISION;
"low_precision"    return LOW_PRECISION;
"precision"        return PRECISION;

"true" {
	yylval->bool_val = true;
	return BOOLCONSTANT;
}

"false" {
	yylval->bool_val = false;
	return BOOLCONSTANT;
}

0[[:digit:]]*u return parse_uint(yylval, yyscanner, yytext, 8);
0[[:digit:]]*  return parse_int(yylval, yyscanner, yytext, 8);

0[xX][[:xdigit:]]+u return parse_uint(yylval, yyscanner, yytext, 16);
0[xX][[:xdigit:]]+  return parse_int(yylval, yyscanner, yytext, 16);

[[:digit:]]*"."[[:digit:]]*[fF]?   return parse_float(yylval, yyscanner, yytext);
[[:digit:]]*"."[[:digit:]]*(lf|LF) return parse_double(yylval, yyscanner, yytext);

[[:digit:]]+[uU] return parse_uint(yylval, yyscanner, yytext, 10);
[[:digit:]]+     return parse_int(yylval, yyscanner, yytext, 10);

[_[:alpha:]][_[:alnum:]]* {
	yylval->str_val = strdup(yytext);
	if (!yylval->str_val)
		YY_FATAL_ERROR("string duplication failed");
	return IDENTIFIER;
}

%%

static void
multiline_comment(yyscan_t yyscanner)
{
	while (1)
	{
		char c = input(yyscanner);
		if (c != '*')
			continue;
		c = input(yyscanner);
		if (c != '/')
			continue;
		break;
	}
}

static int
parse_int(YYSTYPE *lval, yyscan_t yyscanner, const char *text, int base)
{
	char *endptr;
	errno = 0;
	lval->uint_val = strtol(text, &endptr, base);
	if (errno || *endptr)
		YY_FATAL_ERROR("invalid integer immediate");
	return UINTCONSTANT;
}

static int
parse_uint(YYSTYPE *lval, yyscan_t yyscanner, const char *text, int base)
{
	char *endptr;
	errno = 0;
	lval->uint_val = strtoul(text, &endptr, base);
	if (errno || *endptr)
		YY_FATAL_ERROR("invalid unsigned integer immediate");
	return UINTCONSTANT;
}

static int
parse_float(YYSTYPE *lval, yyscan_t yyscanner, const char *text)
{
	char *endptr;
	errno = 0;
	lval->float_val = strtof(text, &endptr);
	if (errno || *endptr)
		YY_FATAL_ERROR("invalid floating point immediate");
	return FLOATCONSTANT;
}

static int
parse_double(YYSTYPE *lval, yyscan_t yyscanner, const char *text)
{
	char *endptr;
	errno = 0;
	lval->double_val = strtod(text, &endptr);
	if (errno || *endptr)
		YY_FATAL_ERROR("invalid double floating point immediate");
	return DOUBLECONSTANT;
}
