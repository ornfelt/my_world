/* The OpenGL Shading Language, Version 4.60.8 */

%{

#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "y.tab.h"

#define EMIT(token) fprintf(stderr, "%s ", #token); return token

#define RESERVED(token) fprintf(stderr, "reserved identifier: %s\n", #token); yyterminate()

void line_comment();
void multiline_comment();

%}

%%

[ \t]
[\n] fprintf(stderr, "\n");

"//" line_comment();
"/*" multiline_comment();
"#" line_comment(); /* XXX */

"const"  EMIT(CONST);
"bool"   EMIT(BOOL);
"float"  EMIT(FLOAT);
"int"    EMIT(INT);
"uint"   EMIT(UINT);
"double" EMIT(DOUBLE);

"bvec2" EMIT(BVEC2);
"bvec3" EMIT(BVEC3);
"bvec4" EMIT(BVEC4);
"ivec2" EMIT(IVEC2);
"ivec3" EMIT(IVEC3);
"ivec4" EMIT(IVEC4);
"uvec2" EMIT(UVEC2);
"uvec3" EMIT(UVEC3);
"uvec4" EMIT(UVEC4);
"vec2"  EMIT(VEC2);
"vec3"  EMIT(VEC3);
"vec4"  EMIT(VEC4);

"mat2"   EMIT(MAT2);
"mat3"   EMIT(MAT3);
"mat4"   EMIT(MAT4);
"mat2x2" EMIT(MAT2X2);
"mat2x3" EMIT(MAT2X3);
"mat2x4" EMIT(MAT2X4);
"mat3x2" EMIT(MAT3X2);
"mat3x3" EMIT(MAT3X3);
"mat3x4" EMIT(MAT3X4);
"mat4x2" EMIT(MAT4X2);
"mat4x3" EMIT(MAT4X3);
"mat4x4" EMIT(MAT4X4);

"dvec2"   EMIT(DVEC2);
"dvec3"   EMIT(DVEC3);
"dvec4"   EMIT(DVEC4);
"dmat2"   EMIT(DMAT2);
"dmat3"   EMIT(DMAT3);
"dmat4"   EMIT(DMAT4);
"dmat2x2" EMIT(DMAT2X2);
"dmat2x3" EMIT(DMAT2X3);
"dmat2x4" EMIT(DMAT2X4);
"dmat3x2" EMIT(DMAT3X2);
"dmat3x3" EMIT(DMAT3X3);
"dmat3x4" EMIT(DMAT3X4);
"dmat4x2" EMIT(DMAT4X2);
"dmat4x3" EMIT(DMAT4X3);
"dmat4x4" EMIT(DMAT4X4);

"centroid"      EMIT(CENTROID);
"in"            EMIT(IN);
"out"           EMIT(OUT);
"inout"         EMIT(INOUT);
"uniform"       EMIT(UNIFORM);
"patch"         EMIT(PATCH);
"sample"        EMIT(SAMPLE);
"buffer"        EMIT(BUFFER);
"shared"        EMIT(SHARED);
"coherent"      EMIT(COHERENT);
"volatile"      EMIT(VOLATILE);
"restrict"      EMIT(RESTRICT);
"readonly"      EMIT(READONLY);
"writeonly"     EMIT(WRITEONLY);
"noperspective" EMIT(NOPERSPECTIVE);
"flat"          EMIT(FLAT);
"smooth"        EMIT(SMOOTH);
"layout"        EMIT(LAYOUT);

"atomic_uint" EMIT(ATOMIC_UINT);

"sampler2D"            EMIT(SAMPLER2D);
"sampler3D"            EMIT(SAMPLER3D);
"sampler2DArray"       EMIT(SAMPLER2DARRAY);
"samplerCube"          EMIT(SAMPLERCUBE);
"sampler2DShadow"      EMIT(SAMPLER2DSHADOW);
"sampler2DArrayShadow" EMIT(SAMPLER2DARRAYSHADOW);
"isampler2D"           EMIT(ISAMPLER2D);
"samplerCubeShadow"    EMIT(SAMPLERCUBESHADOW);
"isampler3D"           EMIT(ISAMPLER3D);
"isamplerCube"         EMIT(ISAMPLERCUBE);
"isampler2DArray"      EMIT(ISAMPLER2DARRAY);
"usampler2D"           EMIT(USAMPLER2D);
"usampler3D"           EMIT(USAMPLER3D);
"usamplerCube"         EMIT(USAMPLERCUBE);
"usampler2DArray"      EMIT(USAMPLER2DARRAY);

"sampler1D"            EMIT(SAMPLER1D);
"sampler1DShadow"      EMIT(SAMPLER1DSHADOW);

"struct" EMIT(STRUCT);
"void"   EMIT(VOID);

"while"      EMIT(WHILE);
"break"      EMIT(BREAK);
"continue"   EMIT(CONTINUE);
"do"         EMIT(DO);
"else"       EMIT(ELSE);
"for"        EMIT(FOR);
"if"         EMIT(IF);
"discard"    EMIT(DISCARD);
"return"     EMIT(RETURN);
"switch"     EMIT(SWITCH);
"case"       EMIT(CASE);
"default"    EMIT(DEFAULT);
"subroutine" EMIT(SUBROUTINE);

"<<"  EMIT(LEFT_OP);
">>"  EMIT(RIGHT_OP);
"++"  EMIT(INC_OP);
"--"  EMIT(DEC_OP);
"<="  EMIT(LE_OP);
">="  EMIT(GE_OP);
"=="  EMIT(EQ_OP);
"!="  EMIT(NE_OP);
"&&"  EMIT(AND_OP);
"||"  EMIT(OR_OP);
"^^"  EMIT(XOR_OP);
"*="  EMIT(MUL_ASSIGN);
"/="  EMIT(DIV_ASSIGN);
"+="  EMIT(ADD_ASSIGN);
"%="  EMIT(MOD_ASSIGN);
"<<=" EMIT(LEFT_ASSIGN);
">>=" EMIT(RIGHT_ASSIGN);
"&="  EMIT(AND_ASSIGN);
"^="  EMIT(XOR_ASSIGN);
"|="  EMIT(OR_ASSIGN);
"-="  EMIT(SUB_ASSIGN);
"("   EMIT(LEFT_PAREN);
")"   EMIT(RIGHT_PAREN);
"["   EMIT(LEFT_BRACKET);
"]"   EMIT(RIGHT_BRACKET);
"{"   EMIT(LEFT_BRACE);
"}"   EMIT(RIGHT_BRACE);
"."   EMIT(DOT);
","   EMIT(COMMA);
":"   EMIT(COLON);
"="   EMIT(EQUAL);
";"   EMIT(SEMICOLON);
"!"   EMIT(BANG);
"-"   EMIT(DASH);
"~"   EMIT(TILDE);
"+"   EMIT(PLUS);
"*"   EMIT(STAR);
"/"   EMIT(SLASH);
"%"   EMIT(PERCENT);
"<"   EMIT(LEFT_ANGLE);
">"   EMIT(RIGHT_ANGLE);
"|"   EMIT(VERTICAL_BAR);
"^"   EMIT(CARET);
"&"   EMIT(AMPERSAND);
"?"   EMIT(QUESTION);

"invariant"        EMIT(INVARIANT);
"precise"          EMIT(PRECISE);
"high_precision"   EMIT(HIGH_PRECISION);
"medium_precision" EMIT(MEDIUM_PRECISION);
"low_precision"    EMIT(LOW_PRECISION);
"precision"        EMIT(PRECISION);

0[[:digit:]]* {
	char *endptr;
	errno = 0;
	yylval.int_val = strtol(yytext, &endptr, 8);
	if (errno || *endptr)
	{
		fprintf(stderr, "invalid integer\n");
		yyterminate();
	}
	EMIT(INTCONSTANT);
	}

0[xX][[:digit:]]+ {
	char *endptr;
	errno = 0;
	yylval.int_val = strtol(yytext, &endptr, 16);
	if (errno || *endptr)
	{
		fprintf(stderr, "invalid integer\n");
		yyterminate();
	}
	EMIT(INTCONSTANT);
	}

[[:digit:]]+"."[[:digit:]]+[f]? {
	char *endptr;
	errno = 0;
	yylval.float_val = strtof(yytext, &endptr);
	if (errno || *endptr)
	{
		fprintf(stderr, "invalid floating point\n");
		yyterminate();
	}
	EMIT(FLOATCONSTANT);
	}

[[:digit:]]+ {
	char *endptr;
	errno = 0;
	yylval.int_val = strtol(yytext, &endptr, 10);
	if (errno || *endptr)
	{
		fprintf(stderr, "invalid integer\n");
		yyterminate();
	}
	EMIT(INTCONSTANT);
	}

[_[:alpha:]][_[:alnum:]]* {
	yylval.str_val = strdup(yytext);
	EMIT(IDENTIFIER);
	}

%%

void line_comment()
{
	while (1)
	{
		char c = input();
		if (c == '\n')
			break;
	}
}

void multiline_comment()
{
	while (1)
	{
		char c = input();
		if (c != '*')
			continue;
		c = input();
		if (c != '/')
			continue;
		break;
	}
}

int yywrap()
{
	return 1;
}
