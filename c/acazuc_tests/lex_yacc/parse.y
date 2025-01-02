%{

#include "ast.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define YYDEBUG 1

extern char yytext[];
extern int yylineno;

void yyerror(char *);

%}

%token CONST BOOL FLOAT INT UINT DOUBLE
%token BVEC2 BVEC3 BVEC4 IVEC2 IVEC3 IVEC4 UVEC2 UVEC3 UVEC4 VEC2 VEC3 VEC4
%token MAT2 MAT3 MAT4 MAT2X2 MAT2X3 MAT2X4 MAT3X2 MAT3X3 MAT3X4 MAT4X2 MAT4X3 MAT4X4
%token DVEC2 DVEC3 DVEC4 DMAT2 DMAT3 DMAT4 DMAT2X2 DMAT2X3 DMAT2X4 DMAT3X2 DMAT3X3 DMAT3X4 DMAT4X2 DMAT4X3 DMAT4X4
%token CENTROID IN OUT INOUT UNIFORM PATCH SAMPLE BUFFER SHARED COHERENT VOLATILE RESTRICT READONLY WRITEONLY NOPERSPECTIVE FLAT SMOOTH LAYOUT
%token ATOMIC_UINT
%token SAMPLER2D SAMPLER3D SAMPLER2DARRAY SAMPLERCUBE SAMPLER2DSHADOW SAMPLER2DARRAYSHADOW ISAMPLER2D SAMPLERCUBESHADOW ISAMPLER3D ISAMPLERCUBE ISAMPLER2DARRAY USAMPLER2D USAMPLER3D USAMPLERCUBE USAMPLER2DARRAY
%token SAMPLER1D SAMPLER1DSHADOW SAMPLER1DARRAY SAMPLER1DARRAYSHADOW ISAMPLER1D ISAMPLER1DARRAY USAMPLER1D USAMPLER1DARRAY SAMPLER2DRECT SAMPLER2DRECTSHADOW ISAMPLER2DRECT USAMPLER2DRECT
%token SAMPLERBUFFER ISAMPLERBUFFER USAMPLERBUFFER SAMPLERCUBEARRAY SAMPLERCUBEARRAYSHADOW ISAMPLERCUBEARRAY USAMPLERCUBEARRAY SAMPLER2DMS ISAMPLER2DMS USAMPLER2DMS USAMPLER2DMSARRAY IMAGECUBE IMAGE2D IIMAGECUBE IMAGE2DARRAY SAMPLER2DMSARRAY IIMAGE2D UIMAGECUBE IIMAGE2DARRAY UIMAGE2D IMAGEBUFFER UIMAGE2DARRAY IMAGE3D ISAMPLER2DMSARRAY IIMAGE3D IIMAGEBUFFER UIMAGE3D UIMAGEBUFFER IMAGECUBEARRAY IIMAGECUBEARRAY UIMAGECUBEARRAY
%token IMAGE1D IIMAGE1D UIMAGE1D IMAGE1DARRAY IIMAGE1DARRAY UIMAGE1DARRAY IMAGE2DRECT IIMAGE2DRECT UIMAGE2DRECT IMAGE2DMS IIMAGE2DMS UIMAGE2DMS IMAGE2DMSARRAY IIMAGE2DMSARRAY UIMAGE2DMSARRAY
%token STRUCT VOID
%token WHILE BREAK CONTINUE DO ELSE FOR IF DISCARD RETURN SWITCH CASE DEFAULT SUBROUTINE
%token<str_val> IDENTIFIER
%token<float_val> FLOATCONSTANT
%token<int_val> INTCONSTANT
%token<uint_val> UINTCONSTANT
%token<bool_val> BOOLCONSTANT
%token<double_val> DOUBLECONSTANT
%token LEFT_OP RIGHT_OP INC_OP DEC_OP LE_OP GE_OP EQ_OP NE_OP AND_OP OR_OP XOR_OP MUL_ASSIGN DIV_ASSIGN ADD_ASSIGN MOD_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN XOR_ASSIGN OR_ASSIGN SUB_ASSIGN LEFT_PAREN RIGHT_PAREN LEFT_BRACKET RIGHT_BRACKET LEFT_BRACE RIGHT_BRACE DOT COMMA COLON EQUAL SEMICOLON BANG DASH TILDE PLUS STAR SLASH PERCENT LEFT_ANGLE RIGHT_ANGLE VERTICAL_BAR CARET AMPERSAND QUESTION
%token INVARIANT PRECISE HIGH_PRECISION MEDIUM_PRECISION LOW_PRECISION PRECISION

%union value {
	int32_t int_val;
	uint32_t uint_val;
	float float_val;
	double double_val;
	char *str_val;
	struct node *node;
	enum unary_op unary_op;
	enum assignment_op assignment_op;
	enum storage_qualifier storage_qualifier;
	enum type_specifier type_specifier;
	enum precision_qualifier precision_qualifier;
	enum invariant_qualifier invariant_qualifier;
	enum interpolation_qualifier interpolation_qualifier;
	enum precise_qualifier precise_qualifier;
}

%type<node> primary_expression postfix_expression unary_expression multiplicative_expression additive_expression shift_expression relational_expression equality_expression and_expression exclusive_or_expression inclusive_or_expression logical_and_expression logical_xor_expression logical_or_expression conditional_expression assignment_expression expression constant_expression declaration struct_specifier translation_unit external_declaration function_definition function_prototype function_declarator function_header function_header_with_parameters single_type_qualifier type_qualifier layout_qualifier layout_qualifier_id_list layout_qualifier_id fully_specified_type compound_statement_no_new_scope parameter_declaration parameter_declarator struct_declarator_list struct_declaration struct_declaration_list struct_declarator function_call function_call_or_method function_call_generic function_call_header_no_parameters function_call_header_with_parameters function_call_header function_identifier type_specifier_nonarray type_specifier parameter_type_specifier
%type<str_val> TYPE_NAME;
%type<unary_op> unary_operator;
%type<assignment_op> assignment_operator;
%type<storage_qualifier> storage_qualifier;
%type<precision_qualifier> precision_qualifier;
%type<invariant_qualifier> invariant_qualifier;
%type<interpolation_qualifier> interpolation_qualifier;
%type<precise_qualifier> precise_qualifier;

%start translation_unit

%%
TYPE_NAME
	: IDENTIFIER {}
	;

variable_identifier
	: IDENTIFIER {}
	;

primary_expression
	: variable_identifier {}
	| INTCONSTANT {}
	| UINTCONSTANT {}
	| FLOATCONSTANT {}
	| BOOLCONSTANT {}
	| DOUBLECONSTANT {}
	| LEFT_PAREN expression RIGHT_PAREN {}
	;

postfix_expression
	: primary_expression {}
	| postfix_expression LEFT_BRACKET integer_expression RIGHT_BRACKET {}
	| function_call {}
	| postfix_expression DOT IDENTIFIER {}
	| postfix_expression INC_OP {}
	| postfix_expression DEC_OP {}
	;

integer_expression
	: expression {}
	;

function_call
	: function_call_or_method { $$ = $1; }
	;

function_call_or_method
	: function_call_generic { $$ = $1; }
	;

function_call_generic
	: function_call_header_with_parameters RIGHT_PAREN { $$ = $1; }
	| function_call_header_no_parameters RIGHT_PAREN { $$ = $1; }
	;

function_call_header_no_parameters
	: function_call_header VOID { $$ = emit_function_call_node($1, NULL); }
	| function_call_header { $$ = emit_function_call_node($1, NULL); }
	;

function_call_header_with_parameters
	: function_call_header assignment_expression { $$ = emit_function_call_node($1, $2); }
	| function_call_header_with_parameters COMMA assignment_expression { $$ = $1; push_function_call_argument($1, $3); }
	;

function_call_header
	: function_identifier LEFT_PAREN { $$ = $1; }
	;

function_identifier
	: type_specifier { $$ = $1; }
	| postfix_expression { $$ = $1; }
	;

unary_expression
	: postfix_expression { $$ = $1; }
	| INC_OP unary_expression {}
	| DEC_OP unary_expression {}
	| unary_operator unary_expression { $$ = emit_unary_op_node($1, $2); }
	;

unary_operator
	: PLUS { $$ = UNARY_OP_POS; }
	| DASH { $$ = UNARY_OP_NEG; }
	| BANG { $$ = UNARY_OP_NOT; }
	| TILDE { $$ = UNARY_OP_INV; }

multiplicative_expression
	: unary_expression { $$ = $1; }
	| multiplicative_expression STAR unary_expression { $$ = emit_binary_op_node(BINARY_OP_MUL, $1, $3); }
	| multiplicative_expression SLASH unary_expression { $$ = emit_binary_op_node(BINARY_OP_DIV, $1, $3); }
	| multiplicative_expression PERCENT unary_expression { $$ = emit_binary_op_node(BINARY_OP_MOD, $1, $3); }
	;

additive_expression
	: multiplicative_expression { $$ = $1; }
	| additive_expression PLUS multiplicative_expression { $$ = emit_binary_op_node(BINARY_OP_ADD, $1, $3); }
	| additive_expression DASH multiplicative_expression { $$ = emit_binary_op_node(BINARY_OP_SUB, $1, $3); }
	;

shift_expression
	: additive_expression { $$ = $1; }
	| shift_expression LEFT_OP additive_expression { $$ = emit_binary_op_node(BINARY_OP_LSL, $1, $3); }
	| shift_expression RIGHT_OP additive_expression { $$ = emit_binary_op_node(BINARY_OP_LSR, $1, $3); }
	;

relational_expression
	: shift_expression { $$ = $1; }
	| relational_expression LEFT_ANGLE shift_expression { $$ = emit_compare_op_node(COMPARE_OP_LT, $1, $3); }
	| relational_expression RIGHT_ANGLE shift_expression { $$ = emit_compare_op_node(COMPARE_OP_GT, $1, $3); }
	| relational_expression LE_OP shift_expression { $$ = emit_compare_op_node(COMPARE_OP_LE, $1, $3); }
	| relational_expression GE_OP shift_expression { $$ = emit_compare_op_node(COMPARE_OP_GE, $1, $3); }
	;

equality_expression
	: relational_expression { $$ = $1; }
	| equality_expression EQ_OP relational_expression { $$ = emit_compare_op_node(COMPARE_OP_EQ, $1, $3); }
	| equality_expression NE_OP relational_expression { $$ = emit_compare_op_node(COMPARE_OP_NE, $1, $3); }
	;

and_expression
	: equality_expression { $$ = $1; }
	| and_expression AMPERSAND equality_expression { $$ = emit_binary_op_node(BINARY_OP_AND, $1, $3); }
	;

exclusive_or_expression
	: and_expression { $$ = $1; }
	| exclusive_or_expression CARET and_expression { $$ = emit_binary_op_node(BINARY_OP_XOR, $1, $3); }
	;

inclusive_or_expression
	: exclusive_or_expression { $$ = $1; }
	| inclusive_or_expression VERTICAL_BAR exclusive_or_expression { $$ = emit_binary_op_node(BINARY_OP_OR, $1, $3); }
	;

logical_and_expression
	: inclusive_or_expression { $$ = $1; }
	| logical_and_expression AND_OP inclusive_or_expression { $$ = emit_binary_op_node(BINARY_OP_LAND, $1, $3); }
	;

logical_xor_expression
	: logical_and_expression { $$ = $1; }
	| logical_xor_expression XOR_OP logical_and_expression { $$ = emit_binary_op_node(BINARY_OP_LXOR, $1, $3); }
	;

logical_or_expression
	: logical_xor_expression { $$ = $1; }
	| logical_or_expression OR_OP logical_xor_expression { $$ = emit_binary_op_node(BINARY_OP_LOR, $1, $3); }
	;

conditional_expression
	: logical_or_expression { $$ = $1; }
	| logical_or_expression QUESTION expression COLON assignment_expression { $$ = emit_ternary_node($1, $3, $5); }
	;

assignment_expression
	: conditional_expression { $$ = $1; }
	| unary_expression assignment_operator assignment_expression { $$ = emit_assignment_node($2, $1, $3); }
	;

assignment_operator
	: EQUAL { $$ = ASSIGNMENT_OP_EQ; }
	| MUL_ASSIGN { $$ = ASSIGNMENT_OP_MUL; }
	| DIV_ASSIGN { $$ = ASSIGNMENT_OP_DIV; }
	| MOD_ASSIGN { $$ = ASSIGNMENT_OP_MOD; }
	| ADD_ASSIGN { $$ = ASSIGNMENT_OP_ADD; }
	| SUB_ASSIGN { $$ = ASSIGNMENT_OP_SUB; }
	| LEFT_ASSIGN { $$ = ASSIGNMENT_OP_LSL; }
	| RIGHT_ASSIGN { $$ = ASSIGNMENT_OP_LSR; }
	| AND_ASSIGN { $$ = ASSIGNMENT_OP_AND; }
	| XOR_ASSIGN { $$ = ASSIGNMENT_OP_XOR; }
	| OR_ASSIGN { $$ = ASSIGNMENT_OP_OR; }
	;

expression
	: assignment_expression { $$ = $1; }
	| expression COMMA assignment_expression { fprintf(stderr, "unhandled\n"); exit(1); }
	;

constant_expression
	: conditional_expression {}
	;

declaration
	: function_prototype SEMICOLON { $$ = $1;}
	| init_declarator_list SEMICOLON {}
	| PRECISION precision_qualifier type_specifier SEMICOLON {}
	| type_qualifier IDENTIFIER LEFT_BRACE struct_declaration_list RIGHT_BRACE SEMICOLON {}
	| type_qualifier IDENTIFIER LEFT_BRACE struct_declaration_list RIGHT_BRACE IDENTIFIER {}
	| SEMICOLON {}
	| type_qualifier IDENTIFIER LEFT_BRACE struct_declaration_list RIGHT_BRACE IDENTIFIER {}
	| array_specifier SEMICOLON {}
	| type_qualifier SEMICOLON {}
	| type_qualifier IDENTIFIER SEMICOLON {}
	| type_qualifier IDENTIFIER identifier_list SEMICOLON {}
	;

identifier_list
	: COMMA IDENTIFIER {}
	| identifier_list COMMA IDENTIFIER {}
	;

function_prototype
	: function_declarator RIGHT_PAREN { $$ = $1; }
	;

function_declarator
	: function_header { $$ = $1; }
	| function_header_with_parameters { $$ = $1; }
	;

function_header_with_parameters
	: function_header parameter_declaration { $$ = $1; push_prototype_parameter($1, $2); }
	| function_header_with_parameters COMMA parameter_declaration { $$ = $1; push_prototype_parameter($1, $3); }
	;

function_header
	: fully_specified_type IDENTIFIER LEFT_PAREN { $$ = emit_prototype_node($1, $2); }
	;

parameter_declarator
	: type_specifier IDENTIFIER { $$ = emit_prototype_parameter_node(NULL, $1, $2); }
	| type_specifier IDENTIFIER array_specifier { $$ = emit_prototype_parameter_node(NULL, $1, $2); /* XXX */ }
	;

parameter_declaration
	: type_qualifier parameter_declarator { $$ = $2; ((struct prototype_parameter_node*)($$))->type_qualifiers = $1; }
	| parameter_declarator { $$ = $1; }
	| type_qualifier parameter_type_specifier { $$ = emit_prototype_parameter_node($1, $2, NULL); ((struct prototype_parameter_node*)($$))->type_qualifiers = $1; }
	| parameter_type_specifier { $$ = emit_prototype_parameter_node(NULL, $1, NULL); }
	;

parameter_type_specifier
	: type_specifier { $$ = $1; }
	;

init_declarator_list
	: single_declaration {}
	| init_declarator_list COMMA IDENTIFIER {}
	| init_declarator_list COMMA IDENTIFIER array_specifier {}
	| init_declarator_list COMMA IDENTIFIER array_specifier EQUAL initializer {}
	| init_declarator_list COMMA IDENTIFIER EQUAL initializer {}
	;

single_declaration
	: fully_specified_type {}
	| fully_specified_type IDENTIFIER {}
	| fully_specified_type IDENTIFIER array_specifier {}
	| fully_specified_type IDENTIFIER array_specifier EQUAL initializer {}
	| fully_specified_type IDENTIFIER EQUAL initializer {}
	;

fully_specified_type
	: type_specifier { $$ = emit_fully_specified_type_node($1, NULL); }
	| type_qualifier type_specifier { $$ = emit_fully_specified_type_node($2, $1); }
	;

invariant_qualifier
	: INVARIANT { $$ = INVARIANT_INVARIANT; }
	;

interpolation_qualifier
	: SMOOTH { $$ = INTERPOLATION_SMOOTH; }
	| FLAT { $$ = INTERPOLATION_FLAT; }
	| NOPERSPECTIVE { $$ = INTERPOLATION_NOPERSPECTIVE; }
	;

layout_qualifier
	: LAYOUT LEFT_PAREN layout_qualifier_id_list RIGHT_PAREN { $$ = $3; }
	;

layout_qualifier_id_list
	: layout_qualifier_id { $$ = $1; }
	| layout_qualifier_id_list COMMA layout_qualifier_id { $$ = $1; push_layout_qualifier($1, $3); }
	;

layout_qualifier_id
	: IDENTIFIER { $$ = emit_layout_qualifier_node($1, NULL); }
	| IDENTIFIER EQUAL constant_expression { $$ = emit_layout_qualifier_node($1, $3); }
	| SHARED { /* XXX */ }
	;

precise_qualifier
	: PRECISE { $$ = PRECISE_PRECISE; }
	;

type_qualifier
	: single_type_qualifier { $$ = $1; }
	| type_qualifier single_type_qualifier { $$ = $1; push_type_qualifier($1, $2); }
	;

single_type_qualifier
	: storage_qualifier { $$ = emit_storage_qualifier_node($1); }
	| layout_qualifier { $$ = emit_layout_qualifier_specifier_node($1); }
	| precision_qualifier { $$ = emit_precision_qualifier_node($1); }
	| interpolation_qualifier { $$ = emit_interpolation_qualifier_node($1); }
	| invariant_qualifier { $$ = emit_invariant_qualifier_node($1); }
	| precise_qualifier { $$ = emit_precise_qualifier_node($1); }
	;

storage_qualifier
	: CONST { $$ = STORAGE_CONST; }
	| IN { $$ = STORAGE_IN; }
	| OUT { $$ = STORAGE_OUT; }
	| INOUT { $$ = STORAGE_INOUT; }
	| CENTROID { $$ = STORAGE_CENTROID; }
	| PATCH { $$ = STORAGE_PATCH; }
	| SAMPLE { $$ = STORAGE_SAMPLE; }
	| UNIFORM { $$ = STORAGE_UNIFORM; }
	| BUFFER { $$ = STORAGE_BUFFER; }
	| SHARED { $$ = STORAGE_SHARED; }
	| COHERENT { $$ = STORAGE_COHERENT; }
	| VOLATILE { $$ = STORAGE_VOLATILE; }
	| RESTRICT { $$ = STORAGE_RESTRICT; }
	| READONLY { $$ = STORAGE_READONLY; }
	| WRITEONLY { $$ = STORAGE_WRITEONLY; }
	| SUBROUTINE { $$ = STORAGE_SUBROUTINE; }
	| SUBROUTINE LEFT_PAREN type_name_list RIGHT_PAREN {}
	;

type_name_list
	: TYPE_NAME {}
	| type_name_list COMMA TYPE_NAME {}
	;

type_specifier
	: type_specifier_nonarray { $$ = $1; }
	| type_specifier_nonarray array_specifier { fprintf(stderr, "unhandled arrays\n"); YYABORT; /* XXX */ }
	;

array_specifier
	: LEFT_BRACKET RIGHT_BRACKET {}
	| LEFT_BRACKET conditional_expression RIGHT_BRACKET {}
	| array_specifier LEFT_BRACKET RIGHT_BRACKET {}
	| array_specifier LEFT_BRACKET conditional_expression RIGHT_BRACKET {}
	;

type_specifier_nonarray
	: VOID { $$ = emit_type_specifier_native_node(TYPE_VOID); }
	| FLOAT { $$ = emit_type_specifier_native_node(TYPE_FLOAT); }
	| DOUBLE { $$ = emit_type_specifier_native_node(TYPE_DOUBLE); }
	| INT { $$ = emit_type_specifier_native_node(TYPE_INT); }
	| UINT { $$ = emit_type_specifier_native_node(TYPE_UINT); }
	| BOOL { $$ = emit_type_specifier_native_node(TYPE_BOOL); }
	| VEC2 { $$ = emit_type_specifier_native_node(TYPE_VEC2); }
	| VEC3 { $$ = emit_type_specifier_native_node(TYPE_VEC3); }
	| VEC4 { $$ = emit_type_specifier_native_node(TYPE_VEC4); }
	| DVEC2 { $$ = emit_type_specifier_native_node(TYPE_DVEC2); }
	| DVEC3 { $$ = emit_type_specifier_native_node(TYPE_DVEC3); }
	| DVEC4 { $$ = emit_type_specifier_native_node(TYPE_DVEC4); }
	| BVEC2 { $$ = emit_type_specifier_native_node(TYPE_BVEC2); }
	| BVEC3 { $$ = emit_type_specifier_native_node(TYPE_BVEC3); }
	| BVEC4 { $$ = emit_type_specifier_native_node(TYPE_BVEC4); }
	| IVEC2 { $$ = emit_type_specifier_native_node(TYPE_IVEC2); }
	| IVEC3 { $$ = emit_type_specifier_native_node(TYPE_IVEC3); }
	| IVEC4 { $$ = emit_type_specifier_native_node(TYPE_IVEC4); }
	| UVEC2 { $$ = emit_type_specifier_native_node(TYPE_UVEC2); }
	| UVEC3 { $$ = emit_type_specifier_native_node(TYPE_UVEC3); }
	| UVEC4 { $$ = emit_type_specifier_native_node(TYPE_UVEC4); }
	| MAT2 { $$ = emit_type_specifier_native_node(TYPE_MAT2X2); }
	| MAT3 { $$ = emit_type_specifier_native_node(TYPE_MAT3X3); }
	| MAT4 { $$ = emit_type_specifier_native_node(TYPE_MAT4X4); }
	| MAT2X2 { $$ = emit_type_specifier_native_node(TYPE_MAT2X2); }
	| MAT2X3 { $$ = emit_type_specifier_native_node(TYPE_MAT2X3); }
	| MAT2X4 { $$ = emit_type_specifier_native_node(TYPE_MAT2X4); }
	| MAT3X2 { $$ = emit_type_specifier_native_node(TYPE_MAT3X2); }
	| MAT3X3 { $$ = emit_type_specifier_native_node(TYPE_MAT3X3); }
	| MAT3X4 { $$ = emit_type_specifier_native_node(TYPE_MAT3X4); }
	| MAT4X2 { $$ = emit_type_specifier_native_node(TYPE_MAT4X2); }
	| MAT4X3 { $$ = emit_type_specifier_native_node(TYPE_MAT4X3); }
	| MAT4X4 { $$ = emit_type_specifier_native_node(TYPE_MAT4X4); }
	| DMAT2 { $$ = emit_type_specifier_native_node(TYPE_DMAT2X2); }
	| DMAT3 { $$ = emit_type_specifier_native_node(TYPE_DMAT3X3); }
	| DMAT4 { $$ = emit_type_specifier_native_node(TYPE_DMAT4X4); }
	| DMAT2X2 { $$ = emit_type_specifier_native_node(TYPE_DMAT2X2); }
	| DMAT2X3 { $$ = emit_type_specifier_native_node(TYPE_DMAT2X3); }
	| DMAT2X4 { $$ = emit_type_specifier_native_node(TYPE_DMAT2X4); }
	| DMAT3X2 { $$ = emit_type_specifier_native_node(TYPE_DMAT3X2); }
	| DMAT3X3 { $$ = emit_type_specifier_native_node(TYPE_DMAT3X3); }
	| DMAT3X4 { $$ = emit_type_specifier_native_node(TYPE_DMAT3X4); }
	| DMAT4X2 { $$ = emit_type_specifier_native_node(TYPE_DMAT4X2); }
	| DMAT4X3 { $$ = emit_type_specifier_native_node(TYPE_DMAT4X3); }
	| DMAT4X4 { $$ = emit_type_specifier_native_node(TYPE_DMAT4X4); }
	| ATOMIC_UINT { $$ = emit_type_specifier_native_node(TYPE_ATOMIC_UINT); }
	| SAMPLER2D { $$ = emit_type_specifier_native_node(TYPE_SAMPLER2D); }
	| SAMPLER3D { $$ = emit_type_specifier_native_node(TYPE_SAMPLER3D); }
	| SAMPLERCUBE { $$ = emit_type_specifier_native_node(TYPE_SAMPLERCUBE); }
	| SAMPLER2DSHADOW { $$ = emit_type_specifier_native_node(TYPE_SAMPLER2DSHADOW); }
	| SAMPLERCUBESHADOW { $$ = emit_type_specifier_native_node(TYPE_SAMPLERCUBESHADOW); }
	| SAMPLER2DARRAY { $$ = emit_type_specifier_native_node(TYPE_SAMPLER2DARRAY); }
	| SAMPLER2DARRAYSHADOW { $$ = emit_type_specifier_native_node(TYPE_SAMPLER2DARRAYSHADOW); }
	| SAMPLERCUBEARRAY { $$ = emit_type_specifier_native_node(TYPE_SAMPLERCUBEARRAY); }
	| SAMPLERCUBEARRAYSHADOW { $$ = emit_type_specifier_native_node(TYPE_SAMPLERCUBEARRAYSHADOW); }
	| ISAMPLER2D { $$ = emit_type_specifier_native_node(TYPE_ISAMPLER2D); }
	| ISAMPLER3D { $$ = emit_type_specifier_native_node(TYPE_ISAMPLER3D); }
	| ISAMPLERCUBE { $$ = emit_type_specifier_native_node(TYPE_ISAMPLERCUBE); }
	| ISAMPLER2DARRAY { $$ = emit_type_specifier_native_node(TYPE_ISAMPLER2DARRAY); }
	| ISAMPLERCUBEARRAY { $$ = emit_type_specifier_native_node(TYPE_ISAMPLERCUBEARRAY); }
	| USAMPLER2D { $$ = emit_type_specifier_native_node(TYPE_USAMPLER2D); }
	| USAMPLER3D { $$ = emit_type_specifier_native_node(TYPE_USAMPLER3D); }
	| USAMPLERCUBE { $$ = emit_type_specifier_native_node(TYPE_USAMPLERCUBE); }
	| USAMPLER2DARRAY { $$ = emit_type_specifier_native_node(TYPE_USAMPLER2DARRAY); }
	| USAMPLERCUBEARRAY { $$ = emit_type_specifier_native_node(TYPE_USAMPLERCUBEARRAY); }
	| SAMPLER1D { $$ = emit_type_specifier_native_node(TYPE_SAMPLER1D); }
	| SAMPLER1DSHADOW { $$ = emit_type_specifier_native_node(TYPE_SAMPLER1DSHADOW); }
	| SAMPLER1DARRAY { $$ = emit_type_specifier_native_node(TYPE_SAMPLER1DARRAY); }
	| SAMPLER1DARRAYSHADOW { $$ = emit_type_specifier_native_node(TYPE_SAMPLER1DARRAYSHADOW); }
	| ISAMPLER1D { $$ = emit_type_specifier_native_node(TYPE_ISAMPLER1D); }
	| ISAMPLER1DARRAY { $$ = emit_type_specifier_native_node(TYPE_ISAMPLER1DARRAY); }
	| USAMPLER1D { $$ = emit_type_specifier_native_node(TYPE_USAMPLER1D); }
	| USAMPLER1DARRAY { $$ = emit_type_specifier_native_node(TYPE_USAMPLER1DARRAY); }
	| SAMPLER2DRECT { $$ = emit_type_specifier_native_node(TYPE_SAMPLER2DRECT); }
	| SAMPLER2DRECTSHADOW { $$ = emit_type_specifier_native_node(TYPE_SAMPLER2DRECTSHADOW); }
	| ISAMPLER2DRECT { $$ = emit_type_specifier_native_node(TYPE_ISAMPLER2DRECT); }
	| USAMPLER2DRECT { $$ = emit_type_specifier_native_node(TYPE_USAMPLER2DRECT); }
	| SAMPLERBUFFER { $$ = emit_type_specifier_native_node(TYPE_SAMPLERBUFFER); }
	| ISAMPLERBUFFER { $$ = emit_type_specifier_native_node(TYPE_ISAMPLERBUFFER); }
	| USAMPLERBUFFER { $$ = emit_type_specifier_native_node(TYPE_USAMPLERBUFFER); }
	| SAMPLER2DMS { $$ = emit_type_specifier_native_node(TYPE_SAMPLER2DMS); }
	| ISAMPLER2DMS { $$ = emit_type_specifier_native_node(TYPE_ISAMPLER2DMS); }
	| USAMPLER2DMS { $$ = emit_type_specifier_native_node(TYPE_USAMPLER2DMS); }
	| SAMPLER2DMSARRAY { $$ = emit_type_specifier_native_node(TYPE_SAMPLER2DMSARRAY); }
	| ISAMPLER2DMSARRAY { $$ = emit_type_specifier_native_node(TYPE_ISAMPLER2DMSARRAY); }
	| USAMPLER2DMSARRAY { $$ = emit_type_specifier_native_node(TYPE_USAMPLER2DMSARRAY); }
	| IMAGE2D { $$ = emit_type_specifier_native_node(TYPE_IMAGE2D); }
	| IIMAGE2D { $$ = emit_type_specifier_native_node(TYPE_IIMAGE2D); }
	| UIMAGE2D { $$ = emit_type_specifier_native_node(TYPE_UIMAGE2D); }
	| IMAGE3D { $$ = emit_type_specifier_native_node(TYPE_IMAGE3D); }
	| IIMAGE3D { $$ = emit_type_specifier_native_node(TYPE_IIMAGE3D); }
	| UIMAGE3D { $$ = emit_type_specifier_native_node(TYPE_UIMAGE3D); }
	| IMAGECUBE { $$ = emit_type_specifier_native_node(TYPE_IMAGECUBE); }
	| IIMAGECUBE { $$ = emit_type_specifier_native_node(TYPE_IIMAGECUBE); }
	| UIMAGECUBE { $$ = emit_type_specifier_native_node(TYPE_UIMAGECUBE); }
	| IMAGEBUFFER { $$ = emit_type_specifier_native_node(TYPE_IMAGEBUFFER); }
	| IIMAGEBUFFER { $$ = emit_type_specifier_native_node(TYPE_IIMAGEBUFFER); }
	| UIMAGEBUFFER { $$ = emit_type_specifier_native_node(TYPE_UIMAGEBUFFER); }
	| IMAGE1D { $$ = emit_type_specifier_native_node(TYPE_IMAGE1D); }
	| IIMAGE1D { $$ = emit_type_specifier_native_node(TYPE_IIMAGE1D); }
	| UIMAGE1D { $$ = emit_type_specifier_native_node(TYPE_UIMAGE1D); }
	| IMAGE1DARRAY { $$ = emit_type_specifier_native_node(TYPE_IMAGE1DARRAY); }
	| IIMAGE1DARRAY { $$ = emit_type_specifier_native_node(TYPE_IIMAGE1DARRAY); }
	| UIMAGE1DARRAY { $$ = emit_type_specifier_native_node(TYPE_UIMAGE1DARRAY); }
	| IMAGE2DRECT { $$ = emit_type_specifier_native_node(TYPE_IMAGE2DRECT); }
	| IIMAGE2DRECT { $$ = emit_type_specifier_native_node(TYPE_IIMAGE2DRECT); }
	| UIMAGE2DRECT { $$ = emit_type_specifier_native_node(TYPE_UIMAGE2DRECT); }
	| IMAGE2DARRAY { $$ = emit_type_specifier_native_node(TYPE_IMAGE2DARRAY); }
	| IIMAGE2DARRAY { $$ = emit_type_specifier_native_node(TYPE_IIMAGE2DARRAY); }
	| UIMAGE2DARRAY { $$ = emit_type_specifier_native_node(TYPE_UIMAGE2DARRAY); }
	| IMAGECUBEARRAY { $$ = emit_type_specifier_native_node(TYPE_IMAGECUBEARRAY); }
	| IIMAGECUBEARRAY { $$ = emit_type_specifier_native_node(TYPE_IIMAGECUBEARRAY); }
	| UIMAGECUBEARRAY { $$ = emit_type_specifier_native_node(TYPE_UIMAGECUBEARRAY); }
	| IMAGE2DMS { $$ = emit_type_specifier_native_node(TYPE_IMAGE2DMS); }
	| IIMAGE2DMS { $$ = emit_type_specifier_native_node(TYPE_IIMAGE2DMS); }
	| UIMAGE2DMS { $$ = emit_type_specifier_native_node(TYPE_UIMAGE2DMS); }
	| IMAGE2DMSARRAY { $$ = emit_type_specifier_native_node(TYPE_IMAGE2DMSARRAY); }
	| IIMAGE2DMSARRAY { $$ = emit_type_specifier_native_node(TYPE_IIMAGE2DMSARRAY); }
	| UIMAGE2DMSARRAY { $$ = emit_type_specifier_native_node(TYPE_UIMAGE2DMSARRAY); }
	| struct_specifier { $$ = emit_type_specifier_struct_node($1); }
	| TYPE_NAME { $$ = emit_type_specifier_named_node($1); }
	;

precision_qualifier
	: HIGH_PRECISION { $$ = PRECISION_HIGH; }
	| MEDIUM_PRECISION { $$ = PRECISION_MEDIUM; }
	| LOW_PRECISION { $$ = PRECISION_LOW; }
	;

struct_specifier
	: STRUCT IDENTIFIER LEFT_BRACE struct_declaration_list RIGHT_BRACE { $$ = emit_struct_definition_node($2, $4); }
	| STRUCT LEFT_BRACE struct_declaration_list RIGHT_BRACE { $$ = emit_struct_definition_node(NULL, $3); }
	;

struct_declaration_list
	: struct_declaration { $$ = $1; }
	| struct_declaration_list struct_declaration { $$ = $1; push_struct_member($$, $2); }
	;

struct_declaration
	: type_specifier struct_declarator_list SEMICOLON { $$ = emit_struct_member_node(NULL, $1, $2); }
	| type_qualifier type_specifier struct_declarator_list SEMICOLON { $$ = emit_struct_member_node($1, $2, $3); }
	;

struct_declarator_list
	: struct_declarator { $$ = $1; }
	| struct_declarator_list COMMA struct_declarator { $$ = $1; push_struct_member_identifier($$, $3); }
	;

struct_declarator
	: IDENTIFIER { $$ = emit_struct_member_identifier_node($1); }
	| IDENTIFIER array_specifier { $$ = emit_struct_member_identifier_node($1); /* XXX */}
	;

initializer
	: assignment_expression {}
	| LEFT_BRACE initializer_list RIGHT_BRACE {}
	| LEFT_BRACE initializer_list COMMA RIGHT_BRACE {}
	;

initializer_list
	: initializer {}
	| initializer_list COMMA initializer {}
	;

declaration_statement
	: declaration {}
	;

statement
	: compound_statement {}
	| simple_statement {}
	;

simple_statement
	: declaration_statement {}
	| expression_statement {}
	| selection_statement {}
	| switch_statement {}
	| case_label {}
	| iteration_statement {}
	| jump_statement {}
	;

compound_statement
	: LEFT_BRACE RIGHT_BRACE {}
	| LEFT_BRACE statement_list RIGHT_BRACE {}
	;

statement_no_new_scope
	: compound_statement_no_new_scope {}
	| simple_statement {}
	;

compound_statement_no_new_scope
	: LEFT_BRACE RIGHT_BRACE {}
	| LEFT_BRACE statement_list RIGHT_BRACE {}
	;

statement_list
	: statement {}
	| statement_list statement {}
	;

expression_statement
	: SEMICOLON {}
	| expression SEMICOLON {}
	;

selection_statement
	: IF LEFT_PAREN expression RIGHT_PAREN selection_rest_statement {}
	;

selection_rest_statement
	: statement ELSE statement {}
	| statement {}
	;

condition 
	: expression {}
	| fully_specified_type IDENTIFIER EQUAL initializer {}
	;

switch_statement
	: SWITCH LEFT_PAREN expression RIGHT_PAREN LEFT_BRACE switch_statement_list {}
	| RIGHT_BRACE {}
	;

switch_statement_list
	: /* nothing */
	| statement_list {}
	;

case_label
	: CASE expression COLON {}
	| DEFAULT COLON {}
	;

iteration_statement
	: WHILE LEFT_PAREN condition RIGHT_PAREN statement_no_new_scope {}
	| DO statement WHILE LEFT_PAREN expression RIGHT_PAREN SEMICOLON {}
	| FOR LEFT_PAREN for_init_statement for_rest_statement RIGHT_PAREN statement_no_new_scope {}
	;

for_init_statement
	: expression_statement {}
	| declaration_statement {}
	;

conditionopt
	: condition {}
	| /* empty */
	;

for_rest_statement
	: conditionopt SEMICOLON {}
	| conditionopt SEMICOLON expression {}
	;

jump_statement
	: CONTINUE SEMICOLON {}
	| BREAK SEMICOLON {}
	| RETURN SEMICOLON {}
	| RETURN expression SEMICOLON {}
	| DISCARD SEMICOLON {}
	;

translation_unit
	: external_declaration { $$ = $1; }
	| translation_unit external_declaration { $$ = $1; }
	;

external_declaration
	: function_definition { $$ = $1; }
	| declaration { $$ = $1; }
	| SEMICOLON {}
	;

function_definition
	: function_prototype compound_statement_no_new_scope { $$ = emit_function_node($1, $2); }
	;

%%

#include <stdio.h>

void yyerror(char *ps)
{
	fprintf(stderr, "\n%d\n", yylineno);
}

extern int yydebug;

int main()
{
	yydebug = 1;
	//while (1) yylex();
	yyparse();
	return 0;
}
