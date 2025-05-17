/* The OpenGL Shading Language, Version 4.60.8 */

%{

#include "ast.h"

#include <stdlib.h>
#include <stdio.h>

struct glsl_parser
{
	void *yyscanner;
	struct glsl_node *root;
};

static void glsl_error(struct glsl_parser *parser, char *);
int glsl_lex(void *yylval, void *yyscanner);
int glsl_lex_init_extra(void *userdata, void **yyscanner);
int glsl_lex_destroy(void *yyscanner);
int glsl__scan_buffer(char *base, size_t size, void *yyscanner);

#define yyscanner (parser->yyscanner)

%}

%define api.pure
%define api.prefix {glsl_}
%lex-param {void *yyscanner}
%parse-param {struct glsl_parser *parser}

%token CONST
%token BOOL
%token FLOAT
%token INT
%token UINT
%token DOUBLE
%token BVEC2
%token BVEC3
%token BVEC4
%token IVEC2
%token IVEC3
%token IVEC4
%token UVEC2
%token UVEC3
%token UVEC4
%token VEC2
%token VEC3
%token VEC4
%token MAT2
%token MAT3
%token MAT4
%token MAT2X2
%token MAT2X3
%token MAT2X4
%token MAT3X2
%token MAT3X3
%token MAT3X4
%token MAT4X2
%token MAT4X3
%token MAT4X4
%token DVEC2
%token DVEC3
%token DVEC4
%token DMAT2
%token DMAT3
%token DMAT4
%token DMAT2X2
%token DMAT2X3
%token DMAT2X4
%token DMAT3X2
%token DMAT3X3
%token DMAT3X4
%token DMAT4X2
%token DMAT4X3
%token DMAT4X4
%token CENTROID
%token IN
%token OUT
%token INOUT
%token UNIFORM
%token PATCH
%token SAMPLE
%token BUFFER
%token SHARED
%token COHERENT
%token VOLATILE
%token RESTRICT
%token READONLY
%token WRITEONLY
%token NOPERSPECTIVE
%token FLAT
%token SMOOTH
%token LAYOUT
%token ATOMIC_UINT
%token SAMPLER2D
%token SAMPLER3D
%token SAMPLER2DARRAY
%token SAMPLERCUBE
%token SAMPLER2DSHADOW
%token SAMPLER2DARRAYSHADOW
%token ISAMPLER2D
%token SAMPLERCUBESHADOW
%token ISAMPLER3D
%token ISAMPLERCUBE
%token ISAMPLER2DARRAY
%token USAMPLER2D
%token USAMPLER3D
%token USAMPLERCUBE
%token USAMPLER2DARRAY
%token SAMPLER1D
%token SAMPLER1DSHADOW
%token SAMPLER1DARRAY
%token SAMPLER1DARRAYSHADOW
%token ISAMPLER1D
%token ISAMPLER1DARRAY
%token USAMPLER1D
%token USAMPLER1DARRAY
%token SAMPLER2DRECT
%token SAMPLER2DRECTSHADOW
%token ISAMPLER2DRECT
%token USAMPLER2DRECT
%token SAMPLERBUFFER
%token ISAMPLERBUFFER
%token USAMPLERBUFFER
%token SAMPLERCUBEARRAY
%token SAMPLERCUBEARRAYSHADOW
%token ISAMPLERCUBEARRAY
%token USAMPLERCUBEARRAY
%token SAMPLER2DMS
%token ISAMPLER2DMS
%token USAMPLER2DMS
%token USAMPLER2DMSARRAY
%token IMAGECUBE
%token IMAGE2D
%token IIMAGECUBE
%token IMAGE2DARRAY
%token SAMPLER2DMSARRAY
%token IIMAGE2D
%token UIMAGECUBE
%token IIMAGE2DARRAY
%token UIMAGE2D
%token IMAGEBUFFER
%token UIMAGE2DARRAY
%token IMAGE3D
%token ISAMPLER2DMSARRAY
%token IIMAGE3D
%token IIMAGEBUFFER
%token UIMAGE3D
%token UIMAGEBUFFER
%token IMAGECUBEARRAY
%token IIMAGECUBEARRAY
%token UIMAGECUBEARRAY
%token IMAGE1D
%token IIMAGE1D
%token UIMAGE1D
%token IMAGE1DARRAY
%token IIMAGE1DARRAY
%token UIMAGE1DARRAY
%token IMAGE2DRECT
%token IIMAGE2DRECT
%token UIMAGE2DRECT
%token IMAGE2DMS
%token IIMAGE2DMS
%token UIMAGE2DMS
%token IMAGE2DMSARRAY
%token IIMAGE2DMSARRAY
%token UIMAGE2DMSARRAY
%token STRUCT
%token VOID
%token WHILE
%token BREAK
%token CONTINUE
%token DO
%token ELSE
%token FOR
%token IF
%token DISCARD
%token RETURN
%token SWITCH
%token CASE
%token DEFAULT
%token SUBROUTINE
%token<str_val> IDENTIFIER
%token<float_val> FLOATCONSTANT
%token<int_val> INTCONSTANT
%token<uint_val> UINTCONSTANT
%token<bool_val> BOOLCONSTANT
%token<double_val> DOUBLECONSTANT
%token LEFT_OP
%token RIGHT_OP
%token INC_OP
%token DEC_OP
%token LE_OP
%token GE_OP
%token EQ_OP
%token NE_OP
%token AND_OP
%token OR_OP
%token XOR_OP
%token MUL_ASSIGN
%token DIV_ASSIGN
%token ADD_ASSIGN
%token MOD_ASSIGN
%token LEFT_ASSIGN
%token RIGHT_ASSIGN
%token AND_ASSIGN
%token XOR_ASSIGN
%token OR_ASSIGN
%token SUB_ASSIGN
%token LEFT_PAREN
%token RIGHT_PAREN
%token LEFT_BRACKET
%token RIGHT_BRACKET
%token LEFT_BRACE
%token RIGHT_BRACE
%token DOT
%token COMMA
%token COLON
%token EQUAL
%token SEMICOLON
%token BANG
%token DASH
%token TILDE
%token PLUS
%token STAR
%token SLASH
%token PERCENT
%token LEFT_ANGLE
%token RIGHT_ANGLE
%token VERTICAL_BAR
%token CARET
%token AMPERSAND
%token QUESTION
%token INVARIANT
%token PRECISE
%token HIGH_PRECISION
%token MEDIUM_PRECISION
%token LOW_PRECISION
%token PRECISION

%union value
{
	int32_t int_val;
	uint32_t uint_val;
	float float_val;
	double double_val;
	char *str_val;
	bool bool_val;
	struct glsl_node *node;
	enum glsl_unary_op unary_op;
	enum glsl_assignment_op assignment_op;
	enum glsl_storage_qualifier storage_qualifier;
	enum glsl_type type;
	enum glsl_precision_qualifier precision_qualifier;
	enum glsl_interpolation_qualifier interpolation_qualifier;
}

%type<node> primary_expression
%type<node> postfix_expression
%type<node> unary_expression
%type<node> multiplicative_expression
%type<node> additive_expression
%type<node> shift_expression
%type<node> relational_expression
%type<node> equality_expression
%type<node> and_expression
%type<node> exclusive_or_expression
%type<node> inclusive_or_expression
%type<node> logical_and_expression
%type<node> logical_xor_expression
%type<node> logical_or_expression
%type<node> conditional_expression
%type<node> assignment_expression
%type<node> expression
%type<node> constant_expression
%type<node> declaration
%type<node> struct_specifier
%type<node> translation_unit
%type<node> external_declaration
%type<node> function_definition
%type<node> function_prototype
%type<node> function_parameters
%type<node> single_type_qualifier
%type<node> type_qualifier
%type<node> layout_qualifier
%type<node> layout_qualifier_id_list
%type<node> layout_qualifier_id
%type<node> fully_specified_type
%type<node> compound_statement_no_new_scope
%type<node> parameter_declaration
%type<node> struct_declarator_list
%type<node> struct_declaration
%type<node> struct_declaration_list
%type<node> struct_declarator
%type<node> function_call
%type<node> function_call_parameters
%type<node> function_identifier
%type<node> type_specifier
%type<node> statement_list
%type<node> statement
%type<node> initializer_list
%type<node> initializer
%type<node> compound_statement
%type<node> simple_statement
%type<node> declaration_statement
%type<node> expression_statement
%type<node> selection_statement
%type<node> switch_statement
%type<node> switch_statement_list
%type<node> case_label
%type<node> iteration_statement
%type<node> jump_statement
%type<node> for_init_statement
%type<node> statement_no_new_scope
%type<node> conditionopt
%type<node> condition
%type<node> init_declarator_list
%type<node> declarator_list
%type<node> declarator_entry
%type<node> array_specifier
%type<node> identifier_list
%type<node> identifier
%type<node> storage_qualifier
%type<node> invariant_qualifier
%type<unary_op> unary_operator
%type<assignment_op> assignment_operator
%type<storage_qualifier> native_storage_qualifier
%type<precision_qualifier> precision_qualifier
%type<interpolation_qualifier> interpolation_qualifier
%type<type> native_type_specifier

%start translation_unit

%%

identifier
	: IDENTIFIER { $$ = glsl_emit_identifier($1); }
	;

primary_expression
	: identifier { $$ = $1; }
	| INTCONSTANT { $$ = glsl_emit_immediate_int($1); }
	| UINTCONSTANT { $$ = glsl_emit_immediate_uint($1); }
	| FLOATCONSTANT { $$ = glsl_emit_immediate_float($1); }
	| BOOLCONSTANT { $$ = glsl_emit_immediate_bool($1); }
	| DOUBLECONSTANT { $$ = glsl_emit_immediate_double($1); }
	| LEFT_PAREN expression RIGHT_PAREN { $$ = $2; }
	;

postfix_expression
	: primary_expression { $$ = $1; }
	| postfix_expression LEFT_BRACKET expression RIGHT_BRACKET { $$ = glsl_emit_array_accessor($1, $3); }
	| function_call { $$ = $1; }
	| postfix_expression DOT identifier { $$ = glsl_emit_field_accessor($1, $3); }
	| postfix_expression INC_OP { $$ = glsl_emit_unary_op(GLSL_UNARY_OP_POST_INC, $1); }
	| postfix_expression DEC_OP { $$ = glsl_emit_unary_op(GLSL_UNARY_OP_POST_DEC, $1); }
	;

function_call
	: function_identifier LEFT_PAREN function_call_parameters RIGHT_PAREN { $$ = glsl_emit_function_call($1, $3); }
	;

function_call_parameters
	: assignment_expression { $$ = glsl_emit_list(); glsl_list_add($$, $1); }
	| function_call_parameters COMMA assignment_expression { $$ = $1; glsl_list_add($$, $3); }
	| VOID { $$ = NULL; }
	| /* empty */ { $$ = NULL; }
	;

function_identifier
	: type_specifier { $$ = $1; }
	| postfix_expression { $$ = $1; }
	;

unary_expression
	: postfix_expression { $$ = $1; }
	| unary_operator unary_expression { $$ = glsl_emit_unary_op($1, $2); }
	;

unary_operator
	: PLUS { $$ = GLSL_UNARY_OP_POS; }
	| DASH { $$ = GLSL_UNARY_OP_NEG; }
	| BANG { $$ = GLSL_UNARY_OP_NOT; }
	| TILDE { $$ = GLSL_UNARY_OP_INV; }
	| INC_OP { $$ = GLSL_UNARY_OP_PRE_INC; }
	| DEC_OP { $$ = GLSL_UNARY_OP_PRE_DEC; }
	;

multiplicative_expression
	: unary_expression { $$ = $1; }
	| multiplicative_expression STAR unary_expression { $$ = glsl_emit_binary_op(GLSL_BINARY_OP_MUL, $1, $3); }
	| multiplicative_expression SLASH unary_expression { $$ = glsl_emit_binary_op(GLSL_BINARY_OP_DIV, $1, $3); }
	| multiplicative_expression PERCENT unary_expression { $$ = glsl_emit_binary_op(GLSL_BINARY_OP_MOD, $1, $3); }
	;

additive_expression
	: multiplicative_expression { $$ = $1; }
	| additive_expression PLUS multiplicative_expression { $$ = glsl_emit_binary_op(GLSL_BINARY_OP_ADD, $1, $3); }
	| additive_expression DASH multiplicative_expression { $$ = glsl_emit_binary_op(GLSL_BINARY_OP_SUB, $1, $3); }
	;

shift_expression
	: additive_expression { $$ = $1; }
	| shift_expression LEFT_OP additive_expression { $$ = glsl_emit_binary_op(GLSL_BINARY_OP_LSL, $1, $3); }
	| shift_expression RIGHT_OP additive_expression { $$ = glsl_emit_binary_op(GLSL_BINARY_OP_LSR, $1, $3); }
	;

relational_expression
	: shift_expression { $$ = $1; }
	| relational_expression LEFT_ANGLE shift_expression { $$ = glsl_emit_compare_op(GLSL_COMPARE_OP_LT, $1, $3); }
	| relational_expression RIGHT_ANGLE shift_expression { $$ = glsl_emit_compare_op(GLSL_COMPARE_OP_GT, $1, $3); }
	| relational_expression LE_OP shift_expression { $$ = glsl_emit_compare_op(GLSL_COMPARE_OP_LE, $1, $3); }
	| relational_expression GE_OP shift_expression { $$ = glsl_emit_compare_op(GLSL_COMPARE_OP_GE, $1, $3); }
	;

equality_expression
	: relational_expression { $$ = $1; }
	| equality_expression EQ_OP relational_expression { $$ = glsl_emit_compare_op(GLSL_COMPARE_OP_EQ, $1, $3); }
	| equality_expression NE_OP relational_expression { $$ = glsl_emit_compare_op(GLSL_COMPARE_OP_NE, $1, $3); }
	;

and_expression
	: equality_expression { $$ = $1; }
	| and_expression AMPERSAND equality_expression { $$ = glsl_emit_binary_op(GLSL_BINARY_OP_AND, $1, $3); }
	;

exclusive_or_expression
	: and_expression { $$ = $1; }
	| exclusive_or_expression CARET and_expression { $$ = glsl_emit_binary_op(GLSL_BINARY_OP_XOR, $1, $3); }
	;

inclusive_or_expression
	: exclusive_or_expression { $$ = $1; }
	| inclusive_or_expression VERTICAL_BAR exclusive_or_expression { $$ = glsl_emit_binary_op(GLSL_BINARY_OP_OR, $1, $3); }
	;

logical_and_expression
	: inclusive_or_expression { $$ = $1; }
	| logical_and_expression AND_OP inclusive_or_expression { $$ = glsl_emit_binary_op(GLSL_BINARY_OP_LAND, $1, $3); }
	;

logical_xor_expression
	: logical_and_expression { $$ = $1; }
	| logical_xor_expression XOR_OP logical_and_expression { $$ = glsl_emit_binary_op(GLSL_BINARY_OP_LXOR, $1, $3); }
	;

logical_or_expression
	: logical_xor_expression { $$ = $1; }
	| logical_or_expression OR_OP logical_xor_expression { $$ = glsl_emit_binary_op(GLSL_BINARY_OP_LOR, $1, $3); }
	;

conditional_expression
	: logical_or_expression { $$ = $1; }
	| logical_or_expression QUESTION expression COLON assignment_expression { $$ = glsl_emit_ternary($1, $3, $5); }
	;

assignment_expression
	: conditional_expression { $$ = $1; }
	| unary_expression assignment_operator assignment_expression { $$ = glsl_emit_assignment($2, $1, $3); }
	;

assignment_operator
	: EQUAL { $$ = GLSL_ASSIGNMENT_OP_EQ; }
	| MUL_ASSIGN { $$ = GLSL_ASSIGNMENT_OP_MUL; }
	| DIV_ASSIGN { $$ = GLSL_ASSIGNMENT_OP_DIV; }
	| MOD_ASSIGN { $$ = GLSL_ASSIGNMENT_OP_MOD; }
	| ADD_ASSIGN { $$ = GLSL_ASSIGNMENT_OP_ADD; }
	| SUB_ASSIGN { $$ = GLSL_ASSIGNMENT_OP_SUB; }
	| LEFT_ASSIGN { $$ = GLSL_ASSIGNMENT_OP_LSL; }
	| RIGHT_ASSIGN { $$ = GLSL_ASSIGNMENT_OP_LSR; }
	| AND_ASSIGN { $$ = GLSL_ASSIGNMENT_OP_AND; }
	| XOR_ASSIGN { $$ = GLSL_ASSIGNMENT_OP_XOR; }
	| OR_ASSIGN { $$ = GLSL_ASSIGNMENT_OP_OR; }
	;

expression
	: assignment_expression { $$ = glsl_emit_list(); glsl_list_add($$, $1); }
	| expression COMMA assignment_expression { $$ = $1; glsl_list_add($$, $3); }
	;

constant_expression
	: conditional_expression { $$ = $1; }
	;

declaration
	: function_prototype SEMICOLON { $$ = $1;}
	| init_declarator_list SEMICOLON { $$ = $1; }
	| PRECISION precision_qualifier type_specifier SEMICOLON { $$ = glsl_emit_precision_declaration($2, $3); }
	| type_qualifier identifier LEFT_BRACE struct_declaration_list RIGHT_BRACE SEMICOLON { $$ = glsl_emit_block_declaration($1, $2, $4, NULL, NULL); }
	| type_qualifier identifier LEFT_BRACE struct_declaration_list RIGHT_BRACE identifier SEMICOLON { $$ = glsl_emit_block_declaration($1, $2, $4, $6, NULL); }
	| type_qualifier identifier LEFT_BRACE struct_declaration_list RIGHT_BRACE identifier array_specifier SEMICOLON { $$ = glsl_emit_block_declaration($1, $2, $4, $6, $7); }
	| type_qualifier SEMICOLON { $$ = glsl_emit_identifier_declaration($1, NULL); }
	| type_qualifier identifier_list SEMICOLON { $$ = glsl_emit_identifier_declaration($1, $2); }
	;

identifier_list
	: identifier { $$ = glsl_emit_list(); glsl_list_add($$, $1); }
	| identifier_list COMMA identifier { $$ = $1; glsl_list_add($$, $3); }
	;

function_prototype
	: fully_specified_type identifier LEFT_PAREN RIGHT_PAREN { $$ = glsl_emit_prototype($1, $2, NULL); }
	| fully_specified_type identifier LEFT_PAREN function_parameters RIGHT_PAREN { $$ = glsl_emit_prototype($1, $2, $4); }
	;

function_parameters
	: parameter_declaration { $$ = glsl_emit_list(); glsl_list_add($$, $1); }
	| function_parameters COMMA parameter_declaration { $$ = $1; glsl_list_add($$, $3); }
	;

parameter_declaration
	: type_qualifier type_specifier identifier { $$ = glsl_emit_prototype_parameter($1, $2, $3, NULL); }
	| type_qualifier type_specifier identifier array_specifier { $$ = glsl_emit_prototype_parameter($1, $2, $3, $4); }
	| type_specifier identifier { $$ = glsl_emit_prototype_parameter(NULL, $1, $2, NULL); }
	| type_specifier identifier array_specifier { $$ = glsl_emit_prototype_parameter(NULL, $1, $2, $3); }
	| type_qualifier type_specifier { $$ = glsl_emit_prototype_parameter($1, $2, NULL, NULL); }
	| type_specifier { $$ = glsl_emit_prototype_parameter(NULL, $1, NULL, NULL); }
	;

init_declarator_list
	: fully_specified_type { $$ = glsl_emit_declaration($1, NULL); }
	| fully_specified_type declarator_list { $$ = glsl_emit_declaration($1, $2); }
	;

declarator_list
	: declarator_entry { $$ = glsl_emit_list(); glsl_list_add($$, $1); }
	| declarator_list COMMA declarator_entry { $$ = $1; glsl_list_add($$, $3); }
	;

declarator_entry
	: identifier { $$ = glsl_emit_declaration_entry($1, NULL, NULL); }
	| identifier array_specifier EQUAL initializer { $$ = glsl_emit_declaration_entry($1, $2, $4); }
	| identifier array_specifier { $$ = glsl_emit_declaration_entry($1, $2, NULL); }
	| identifier EQUAL initializer { $$ = glsl_emit_declaration_entry($1, NULL, $3); }
	;

fully_specified_type
	: type_specifier { $$ = glsl_emit_fully_specified_type($1, NULL); }
	| type_qualifier type_specifier { $$ = glsl_emit_fully_specified_type($2, $1); }
	;

invariant_qualifier
	: INVARIANT { $$ = glsl_emit_invariant_qualifier(); }
	;

interpolation_qualifier
	: SMOOTH { $$ = GLSL_INTERPOLATION_SMOOTH; }
	| FLAT { $$ = GLSL_INTERPOLATION_FLAT; }
	| NOPERSPECTIVE { $$ = GLSL_INTERPOLATION_NOPERSPECTIVE; }
	;

layout_qualifier
	: LAYOUT LEFT_PAREN layout_qualifier_id_list RIGHT_PAREN { $$ = $3; }
	;

layout_qualifier_id_list
	: layout_qualifier_id { $$ = glsl_emit_list(); glsl_list_add($$, $1); }
	| layout_qualifier_id_list COMMA layout_qualifier_id { $$ = $1; glsl_list_add($$, $3); }
	;

layout_qualifier_id
	: identifier { $$ = glsl_emit_layout_qualifier($1, NULL); }
	| identifier EQUAL constant_expression { $$ = glsl_emit_layout_qualifier($1, $3); }
	| SHARED { $$ = glsl_emit_shared_qualifier(); }
	;

type_qualifier
	: single_type_qualifier { $$ = glsl_emit_list(); glsl_list_add($$, $1); }
	| type_qualifier single_type_qualifier { $$ = $1; glsl_list_add($$, $2); }
	;

single_type_qualifier
	: storage_qualifier { $$ = $1; }
	| layout_qualifier { $$ = glsl_emit_layout_qualifiers($1); }
	| precision_qualifier { $$ = glsl_emit_precision_qualifier($1); }
	| interpolation_qualifier { $$ = glsl_emit_interpolation_qualifier($1); }
	| invariant_qualifier { $$ = $1; }
	| PRECISE { $$ = glsl_emit_precise_qualifier(); }
	;

native_storage_qualifier
	: CONST { $$ = GLSL_STORAGE_CONST; }
	| IN { $$ = GLSL_STORAGE_IN; }
	| OUT { $$ = GLSL_STORAGE_OUT; }
	| INOUT { $$ = GLSL_STORAGE_INOUT; }
	| CENTROID { $$ = GLSL_STORAGE_CENTROID; }
	| PATCH { $$ = GLSL_STORAGE_PATCH; }
	| SAMPLE { $$ = GLSL_STORAGE_SAMPLE; }
	| UNIFORM { $$ = GLSL_STORAGE_UNIFORM; }
	| BUFFER { $$ = GLSL_STORAGE_BUFFER; }
	| SHARED { $$ = GLSL_STORAGE_SHARED; }
	| COHERENT { $$ = GLSL_STORAGE_COHERENT; }
	| VOLATILE { $$ = GLSL_STORAGE_VOLATILE; }
	| RESTRICT { $$ = GLSL_STORAGE_RESTRICT; }
	| READONLY { $$ = GLSL_STORAGE_READONLY; }
	| WRITEONLY { $$ = GLSL_STORAGE_WRITEONLY; }
	| SUBROUTINE { $$ = GLSL_STORAGE_SUBROUTINE; }
	;

storage_qualifier
	: native_storage_qualifier { $$ = glsl_emit_storage_qualifier($1); }
	| SUBROUTINE LEFT_PAREN identifier_list RIGHT_PAREN { $$ = glsl_emit_subroutine_qualifier($3); }
	;

type_specifier
	: native_type_specifier { $$ = glsl_emit_type_specifier(glsl_emit_type($1), NULL); }
	| native_type_specifier array_specifier { $$ = glsl_emit_type_specifier(glsl_emit_type($1), $2); }
	| struct_specifier { $$ = glsl_emit_type_specifier($1, NULL); }
	| struct_specifier array_specifier { $$ = glsl_emit_type_specifier($1, $2); }
	| identifier { $$ = glsl_emit_type_specifier($1, NULL); }
	| identifier array_specifier { $$ = glsl_emit_type_specifier($1, $2); }
	;

array_specifier
	: LEFT_BRACKET RIGHT_BRACKET { $$ = glsl_emit_list(); glsl_list_add($$, NULL); }
	| LEFT_BRACKET conditional_expression RIGHT_BRACKET { $$ = glsl_emit_list(); glsl_list_add($$, $2); }
	| array_specifier LEFT_BRACKET RIGHT_BRACKET { $$ = $1; glsl_list_add($$, NULL); }
	| array_specifier LEFT_BRACKET conditional_expression RIGHT_BRACKET { $$ = $1; glsl_list_add($$, $3); }
	;

native_type_specifier
	: VOID { $$ = GLSL_TYPE_VOID; }
	| FLOAT { $$ = GLSL_TYPE_FLOAT; }
	| DOUBLE { $$ = GLSL_TYPE_DOUBLE; }
	| INT { $$ = GLSL_TYPE_INT; }
	| UINT { $$ = GLSL_TYPE_UINT; }
	| BOOL { $$ = GLSL_TYPE_BOOL; }
	| VEC2 { $$ = GLSL_TYPE_VEC2; }
	| VEC3 { $$ = GLSL_TYPE_VEC3; }
	| VEC4 { $$ = GLSL_TYPE_VEC4; }
	| DVEC2 { $$ = GLSL_TYPE_DVEC2; }
	| DVEC3 { $$ = GLSL_TYPE_DVEC3; }
	| DVEC4 { $$ = GLSL_TYPE_DVEC4; }
	| BVEC2 { $$ = GLSL_TYPE_BVEC2; }
	| BVEC3 { $$ = GLSL_TYPE_BVEC3; }
	| BVEC4 { $$ = GLSL_TYPE_BVEC4; }
	| IVEC2 { $$ = GLSL_TYPE_IVEC2; }
	| IVEC3 { $$ = GLSL_TYPE_IVEC3; }
	| IVEC4 { $$ = GLSL_TYPE_IVEC4; }
	| UVEC2 { $$ = GLSL_TYPE_UVEC2; }
	| UVEC3 { $$ = GLSL_TYPE_UVEC3; }
	| UVEC4 { $$ = GLSL_TYPE_UVEC4; }
	| MAT2 { $$ = GLSL_TYPE_MAT2X2; }
	| MAT3 { $$ = GLSL_TYPE_MAT3X3; }
	| MAT4 { $$ = GLSL_TYPE_MAT4X4; }
	| MAT2X2 { $$ = GLSL_TYPE_MAT2X2; }
	| MAT2X3 { $$ = GLSL_TYPE_MAT2X3; }
	| MAT2X4 { $$ = GLSL_TYPE_MAT2X4; }
	| MAT3X2 { $$ = GLSL_TYPE_MAT3X2; }
	| MAT3X3 { $$ = GLSL_TYPE_MAT3X3; }
	| MAT3X4 { $$ = GLSL_TYPE_MAT3X4; }
	| MAT4X2 { $$ = GLSL_TYPE_MAT4X2; }
	| MAT4X3 { $$ = GLSL_TYPE_MAT4X3; }
	| MAT4X4 { $$ = GLSL_TYPE_MAT4X4; }
	| DMAT2 { $$ = GLSL_TYPE_DMAT2X2; }
	| DMAT3 { $$ = GLSL_TYPE_DMAT3X3; }
	| DMAT4 { $$ = GLSL_TYPE_DMAT4X4; }
	| DMAT2X2 { $$ = GLSL_TYPE_DMAT2X2; }
	| DMAT2X3 { $$ = GLSL_TYPE_DMAT2X3; }
	| DMAT2X4 { $$ = GLSL_TYPE_DMAT2X4; }
	| DMAT3X2 { $$ = GLSL_TYPE_DMAT3X2; }
	| DMAT3X3 { $$ = GLSL_TYPE_DMAT3X3; }
	| DMAT3X4 { $$ = GLSL_TYPE_DMAT3X4; }
	| DMAT4X2 { $$ = GLSL_TYPE_DMAT4X2; }
	| DMAT4X3 { $$ = GLSL_TYPE_DMAT4X3; }
	| DMAT4X4 { $$ = GLSL_TYPE_DMAT4X4; }
	| ATOMIC_UINT { $$ = GLSL_TYPE_ATOMIC_UINT; }
	| SAMPLER2D { $$ = GLSL_TYPE_SAMPLER2D; }
	| SAMPLER3D { $$ = GLSL_TYPE_SAMPLER3D; }
	| SAMPLERCUBE { $$ = GLSL_TYPE_SAMPLERCUBE; }
	| SAMPLER2DSHADOW { $$ = GLSL_TYPE_SAMPLER2DSHADOW; }
	| SAMPLERCUBESHADOW { $$ = GLSL_TYPE_SAMPLERCUBESHADOW; }
	| SAMPLER2DARRAY { $$ = GLSL_TYPE_SAMPLER2DARRAY; }
	| SAMPLER2DARRAYSHADOW { $$ = GLSL_TYPE_SAMPLER2DARRAYSHADOW; }
	| SAMPLERCUBEARRAY { $$ = GLSL_TYPE_SAMPLERCUBEARRAY; }
	| SAMPLERCUBEARRAYSHADOW { $$ = GLSL_TYPE_SAMPLERCUBEARRAYSHADOW; }
	| ISAMPLER2D { $$ = GLSL_TYPE_ISAMPLER2D; }
	| ISAMPLER3D { $$ = GLSL_TYPE_ISAMPLER3D; }
	| ISAMPLERCUBE { $$ = GLSL_TYPE_ISAMPLERCUBE; }
	| ISAMPLER2DARRAY { $$ = GLSL_TYPE_ISAMPLER2DARRAY; }
	| ISAMPLERCUBEARRAY { $$ = GLSL_TYPE_ISAMPLERCUBEARRAY; }
	| USAMPLER2D { $$ = GLSL_TYPE_USAMPLER2D; }
	| USAMPLER3D { $$ = GLSL_TYPE_USAMPLER3D; }
	| USAMPLERCUBE { $$ = GLSL_TYPE_USAMPLERCUBE; }
	| USAMPLER2DARRAY { $$ = GLSL_TYPE_USAMPLER2DARRAY; }
	| USAMPLERCUBEARRAY { $$ = GLSL_TYPE_USAMPLERCUBEARRAY; }
	| SAMPLER1D { $$ = GLSL_TYPE_SAMPLER1D; }
	| SAMPLER1DSHADOW { $$ = GLSL_TYPE_SAMPLER1DSHADOW; }
	| SAMPLER1DARRAY { $$ = GLSL_TYPE_SAMPLER1DARRAY; }
	| SAMPLER1DARRAYSHADOW { $$ = GLSL_TYPE_SAMPLER1DARRAYSHADOW; }
	| ISAMPLER1D { $$ = GLSL_TYPE_ISAMPLER1D; }
	| ISAMPLER1DARRAY { $$ = GLSL_TYPE_ISAMPLER1DARRAY; }
	| USAMPLER1D { $$ = GLSL_TYPE_USAMPLER1D; }
	| USAMPLER1DARRAY { $$ = GLSL_TYPE_USAMPLER1DARRAY; }
	| SAMPLER2DRECT { $$ = GLSL_TYPE_SAMPLER2DRECT; }
	| SAMPLER2DRECTSHADOW { $$ = GLSL_TYPE_SAMPLER2DRECTSHADOW; }
	| ISAMPLER2DRECT { $$ = GLSL_TYPE_ISAMPLER2DRECT; }
	| USAMPLER2DRECT { $$ = GLSL_TYPE_USAMPLER2DRECT; }
	| SAMPLERBUFFER { $$ = GLSL_TYPE_SAMPLERBUFFER; }
	| ISAMPLERBUFFER { $$ = GLSL_TYPE_ISAMPLERBUFFER; }
	| USAMPLERBUFFER { $$ = GLSL_TYPE_USAMPLERBUFFER; }
	| SAMPLER2DMS { $$ = GLSL_TYPE_SAMPLER2DMS; }
	| ISAMPLER2DMS { $$ = GLSL_TYPE_ISAMPLER2DMS; }
	| USAMPLER2DMS { $$ = GLSL_TYPE_USAMPLER2DMS; }
	| SAMPLER2DMSARRAY { $$ = GLSL_TYPE_SAMPLER2DMSARRAY; }
	| ISAMPLER2DMSARRAY { $$ = GLSL_TYPE_ISAMPLER2DMSARRAY; }
	| USAMPLER2DMSARRAY { $$ = GLSL_TYPE_USAMPLER2DMSARRAY; }
	| IMAGE2D { $$ = GLSL_TYPE_IMAGE2D; }
	| IIMAGE2D { $$ = GLSL_TYPE_IIMAGE2D; }
	| UIMAGE2D { $$ = GLSL_TYPE_UIMAGE2D; }
	| IMAGE3D { $$ = GLSL_TYPE_IMAGE3D; }
	| IIMAGE3D { $$ = GLSL_TYPE_IIMAGE3D; }
	| UIMAGE3D { $$ = GLSL_TYPE_UIMAGE3D; }
	| IMAGECUBE { $$ = GLSL_TYPE_IMAGECUBE; }
	| IIMAGECUBE { $$ = GLSL_TYPE_IIMAGECUBE; }
	| UIMAGECUBE { $$ = GLSL_TYPE_UIMAGECUBE; }
	| IMAGEBUFFER { $$ = GLSL_TYPE_IMAGEBUFFER; }
	| IIMAGEBUFFER { $$ = GLSL_TYPE_IIMAGEBUFFER; }
	| UIMAGEBUFFER { $$ = GLSL_TYPE_UIMAGEBUFFER; }
	| IMAGE1D { $$ = GLSL_TYPE_IMAGE1D; }
	| IIMAGE1D { $$ = GLSL_TYPE_IIMAGE1D; }
	| UIMAGE1D { $$ = GLSL_TYPE_UIMAGE1D; }
	| IMAGE1DARRAY { $$ = GLSL_TYPE_IMAGE1DARRAY; }
	| IIMAGE1DARRAY { $$ = GLSL_TYPE_IIMAGE1DARRAY; }
	| UIMAGE1DARRAY { $$ = GLSL_TYPE_UIMAGE1DARRAY; }
	| IMAGE2DRECT { $$ = GLSL_TYPE_IMAGE2DRECT; }
	| IIMAGE2DRECT { $$ = GLSL_TYPE_IIMAGE2DRECT; }
	| UIMAGE2DRECT { $$ = GLSL_TYPE_UIMAGE2DRECT; }
	| IMAGE2DARRAY { $$ = GLSL_TYPE_IMAGE2DARRAY; }
	| IIMAGE2DARRAY { $$ = GLSL_TYPE_IIMAGE2DARRAY; }
	| UIMAGE2DARRAY { $$ = GLSL_TYPE_UIMAGE2DARRAY; }
	| IMAGECUBEARRAY { $$ = GLSL_TYPE_IMAGECUBEARRAY; }
	| IIMAGECUBEARRAY { $$ = GLSL_TYPE_IIMAGECUBEARRAY; }
	| UIMAGECUBEARRAY { $$ = GLSL_TYPE_UIMAGECUBEARRAY; }
	| IMAGE2DMS { $$ = GLSL_TYPE_IMAGE2DMS; }
	| IIMAGE2DMS { $$ = GLSL_TYPE_IIMAGE2DMS; }
	| UIMAGE2DMS { $$ = GLSL_TYPE_UIMAGE2DMS; }
	| IMAGE2DMSARRAY { $$ = GLSL_TYPE_IMAGE2DMSARRAY; }
	| IIMAGE2DMSARRAY { $$ = GLSL_TYPE_IIMAGE2DMSARRAY; }
	| UIMAGE2DMSARRAY { $$ = GLSL_TYPE_UIMAGE2DMSARRAY; }

precision_qualifier
	: HIGH_PRECISION { $$ = GLSL_PRECISION_HIGH; }
	| MEDIUM_PRECISION { $$ = GLSL_PRECISION_MEDIUM; }
	| LOW_PRECISION { $$ = GLSL_PRECISION_LOW; }
	;

struct_specifier
	: STRUCT identifier LEFT_BRACE struct_declaration_list RIGHT_BRACE { $$ = glsl_emit_struct_definition($2, $4); }
	| STRUCT LEFT_BRACE struct_declaration_list RIGHT_BRACE { $$ = glsl_emit_struct_definition(NULL, $3); }
	;

struct_declaration_list
	: struct_declaration { $$ = glsl_emit_list(); glsl_list_add($$, $1); }
	| struct_declaration_list struct_declaration { $$ = $1; glsl_list_add($$, $2); }
	;

struct_declaration
	: type_specifier struct_declarator_list SEMICOLON { $$ = glsl_emit_struct_member(NULL, $1, $2); }
	| type_qualifier type_specifier struct_declarator_list SEMICOLON { $$ = glsl_emit_struct_member($1, $2, $3); }
	;

struct_declarator_list
	: struct_declarator { $$ = glsl_emit_list(); glsl_list_add($$, $1); }
	| struct_declarator_list COMMA struct_declarator { $$ = $1; glsl_list_add($$, $3); }
	;

struct_declarator
	: identifier { $$ = glsl_emit_struct_member_identifier($1, NULL); }
	| identifier array_specifier { $$ = glsl_emit_struct_member_identifier($1, $2); }
	;

initializer
	: assignment_expression { $$ = $1; }
	| LEFT_BRACE initializer_list RIGHT_BRACE { $$ = $2; }
	| LEFT_BRACE initializer_list COMMA RIGHT_BRACE { $$ = $2; }
	;

initializer_list
	: initializer { $$ = glsl_emit_list(); glsl_list_add($$, $1); }
	| initializer_list COMMA initializer { $$ = $1; glsl_list_add($$, $3); }
	;

declaration_statement
	: declaration { $$ = $1; }
	;

statement
	: compound_statement { $$ = $1; }
	| simple_statement { $$ = $1; }
	;

simple_statement
	: declaration_statement { $$ = $1; }
	| expression_statement { $$ = $1; }
	| selection_statement { $$ = $1; }
	| switch_statement { $$ = $1; }
	| case_label { $$ = $1; }
	| iteration_statement { $$ = $1; }
	| jump_statement { $$ = $1; }
	;

compound_statement
	: LEFT_BRACE RIGHT_BRACE { $$ = NULL; }
	| LEFT_BRACE statement_list RIGHT_BRACE { $$ = $2; }
	;

statement_no_new_scope
	: compound_statement_no_new_scope { $$ = $1; }
	| simple_statement { $$ = $1; }
	;

compound_statement_no_new_scope
	: LEFT_BRACE RIGHT_BRACE { $$ = NULL; }
	| LEFT_BRACE statement_list RIGHT_BRACE { $$ = $2; }
	;

statement_list
	: statement { $$ = glsl_emit_list(); glsl_list_add($$, $1); }
	| statement_list statement { $$ = $1; glsl_list_add($$, $2); }
	;

expression_statement
	: SEMICOLON { $$ = NULL; }
	| expression SEMICOLON { $$ = $1; }
	;

selection_statement
	: IF LEFT_PAREN expression RIGHT_PAREN statement { $$ = glsl_emit_condition($3, $5, NULL); }
	| IF LEFT_PAREN expression RIGHT_PAREN statement ELSE statement { $$ = glsl_emit_condition($3, $5, $7); }
	;

condition
	: expression { $$ = $1; }
	| fully_specified_type identifier EQUAL initializer { struct glsl_node *entry = glsl_emit_declaration_entry($2, NULL, $4); struct glsl_node *list = glsl_emit_list(); glsl_list_add(list, entry); $$ = glsl_emit_declaration($1, list); }
	;

switch_statement
	: SWITCH LEFT_PAREN expression RIGHT_PAREN LEFT_BRACE switch_statement_list RIGHT_BRACE { $$ = glsl_emit_switch($3, $6); }
	;

switch_statement_list
	: { $$ = NULL; }
	| statement_list { $$ = $1; }
	;

case_label
	: CASE expression COLON { $$ = glsl_emit_case($2); }
	| DEFAULT COLON { $$ = glsl_emit_case(NULL); }
	;

iteration_statement
	: WHILE LEFT_PAREN condition RIGHT_PAREN statement_no_new_scope { $$ = glsl_emit_while(false, $3, $5); }
	| DO statement WHILE LEFT_PAREN expression RIGHT_PAREN SEMICOLON { $$ = glsl_emit_while(true, $5, $2); }
	| FOR LEFT_PAREN for_init_statement conditionopt SEMICOLON RIGHT_PAREN statement_no_new_scope { $$ = glsl_emit_for($3, $4, NULL, $7); }
	| FOR LEFT_PAREN for_init_statement conditionopt SEMICOLON expression RIGHT_PAREN statement_no_new_scope { $$ = glsl_emit_for($3, $4, $6, $8); }
	;

for_init_statement
	: expression_statement { $$ = $1; }
	| declaration_statement { $$ = $1; }
	;

conditionopt
	: condition { $$ = $1; }
	| /* empty */ { $$ = NULL; }
	;

jump_statement
	: CONTINUE SEMICOLON { $$ = glsl_emit_jump(GLSL_JUMP_CONTINUE, NULL); }
	| BREAK SEMICOLON { $$ = glsl_emit_jump(GLSL_JUMP_BREAK, NULL); }
	| RETURN SEMICOLON { $$ = glsl_emit_jump(GLSL_JUMP_RETURN, NULL); }
	| RETURN expression SEMICOLON { $$ = glsl_emit_jump(GLSL_JUMP_RETURN, $2); }
	| DISCARD SEMICOLON { $$ = glsl_emit_jump(GLSL_JUMP_DISCARD, NULL); }
	;

translation_unit
	: external_declaration { $$ = glsl_emit_list(); parser->root = $$; if ($1) glsl_list_add($$, $1); }
	| translation_unit external_declaration { $$ = $1; if ($2) glsl_list_add($$, $2); }
	;

external_declaration
	: function_definition { $$ = $1; }
	| declaration { $$ = $1; }
	| SEMICOLON { $$ = NULL; }
	;

function_definition
	: function_prototype compound_statement_no_new_scope { $$ = glsl_emit_function($1, $2); }
	;

%%

#undef yyscanner

static void
glsl_error(struct glsl_parser *parser, char *ps)
{
	(void)parser;
	fprintf(stderr, "%s\n", ps);
}

static char *
read_buffer(size_t *size)
{
	char *data = NULL;
	ssize_t rd;

	*size = 0;
	while (1)
	{
		char *tmp = realloc(data, *size + 4096);
		if (!tmp)
		{
			free(data);
			return NULL;
		}
		data = tmp;
		rd = fread(&data[*size], 1, 4096 - 2, stdin);
		if (rd <= 0)
			break;
		*size += rd;
	}
	data[*size] = 0;
	data[*size + 1] = 0;
	*size += 2;
	return data;
}

int
main()
{
	struct glsl_parser parser;
	char *data;
	size_t size;

	parser.root = NULL;
	data = read_buffer(&size);
	if (!data)
		return 1;
	if (glsl_lex_init_extra(NULL, &parser.yyscanner))
		return 1;
	if (!glsl__scan_buffer(data, size, parser.yyscanner))
		return 1;
	yyparse(&parser);
	glsl_lex_destroy(parser.yyscanner);
	if (parser.root)
		glsl_print(parser.root);
	return 0;
}
