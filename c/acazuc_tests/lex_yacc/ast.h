#ifndef GLSL_AST_H
#define GLSL_AST_H

#include <sys/queue.h>

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

enum glsl_node_type
{
	GLSL_NODE_ARRAY_ACCESSOR,
	GLSL_NODE_ASSIGNMENT,
	GLSL_NODE_BINARY_OP,
	GLSL_NODE_BLOCK_DECLARATION,
	GLSL_NODE_CASE,
	GLSL_NODE_COMPARE_OP,
	GLSL_NODE_CONDITION,
	GLSL_NODE_DECLARATION,
	GLSL_NODE_DECLARATION_ENTRY,
	GLSL_NODE_FIELD_ACCESSOR,
	GLSL_NODE_FOR,
	GLSL_NODE_FULLY_SPECIFIED_TYPE,
	GLSL_NODE_FUNCTION,
	GLSL_NODE_FUNCTION_CALL,
	GLSL_NODE_IDENTIFIER,
	GLSL_NODE_IDENTIFIER_DECLARATION,
	GLSL_NODE_IMMEDIATE_BOOL,
	GLSL_NODE_IMMEDIATE_DOUBLE,
	GLSL_NODE_IMMEDIATE_FLOAT,
	GLSL_NODE_IMMEDIATE_INT,
	GLSL_NODE_IMMEDIATE_UINT,
	GLSL_NODE_INTERPOLATION_QUALIFIER,
	GLSL_NODE_INVARIANT_QUALIFIER,
	GLSL_NODE_JUMP,
	GLSL_NODE_LAYOUT_QUALIFIER,
	GLSL_NODE_LAYOUT_QUALIFIERS,
	GLSL_NODE_LIST,
	GLSL_NODE_PRECISE_QUALIFIER,
	GLSL_NODE_PRECISION_DECLARATION,
	GLSL_NODE_PRECISION_QUALIFIER,
	GLSL_NODE_PROTOTYPE,
	GLSL_NODE_PROTOTYPE_PARAMETER,
	GLSL_NODE_SHARED_QUALIFIER,
	GLSL_NODE_STORAGE_QUALIFIER,
	GLSL_NODE_STRUCT_DEFINITION,
	GLSL_NODE_STRUCT_MEMBER,
	GLSL_NODE_STRUCT_MEMBER_IDENTIFIER,
	GLSL_NODE_SUBROUTINE_QUALIFIER,
	GLSL_NODE_SWITCH,
	GLSL_NODE_TERNARY,
	GLSL_NODE_TYPE,
	GLSL_NODE_TYPE_SPECIFIER,
	GLSL_NODE_UNARY_OP,
	GLSL_NODE_WHILE,
};

enum glsl_assignment_op
{
	GLSL_ASSIGNMENT_OP_EQ,
	GLSL_ASSIGNMENT_OP_MUL,
	GLSL_ASSIGNMENT_OP_DIV,
	GLSL_ASSIGNMENT_OP_MOD,
	GLSL_ASSIGNMENT_OP_ADD,
	GLSL_ASSIGNMENT_OP_SUB,
	GLSL_ASSIGNMENT_OP_LSL,
	GLSL_ASSIGNMENT_OP_LSR,
	GLSL_ASSIGNMENT_OP_AND,
	GLSL_ASSIGNMENT_OP_XOR,
	GLSL_ASSIGNMENT_OP_OR,
};

enum glsl_binary_op
{
	GLSL_BINARY_OP_ADD,
	GLSL_BINARY_OP_SUB,
	GLSL_BINARY_OP_MUL,
	GLSL_BINARY_OP_DIV,
	GLSL_BINARY_OP_MOD,
	GLSL_BINARY_OP_LSL,
	GLSL_BINARY_OP_LSR,
	GLSL_BINARY_OP_AND,
	GLSL_BINARY_OP_OR,
	GLSL_BINARY_OP_XOR,
	GLSL_BINARY_OP_LAND,
	GLSL_BINARY_OP_LOR,
	GLSL_BINARY_OP_LXOR,
};

enum glsl_compare_op
{
	GLSL_COMPARE_OP_LT,
	GLSL_COMPARE_OP_LE,
	GLSL_COMPARE_OP_EQ,
	GLSL_COMPARE_OP_NE,
	GLSL_COMPARE_OP_GE,
	GLSL_COMPARE_OP_GT,
};

enum glsl_interpolation_qualifier
{
	GLSL_INTERPOLATION_SMOOTH,
	GLSL_INTERPOLATION_FLAT,
	GLSL_INTERPOLATION_NOPERSPECTIVE,
};

enum glsl_jump_type
{
	GLSL_JUMP_CONTINUE,
	GLSL_JUMP_BREAK,
	GLSL_JUMP_RETURN,
	GLSL_JUMP_DISCARD,
};

enum glsl_precision_qualifier
{
	GLSL_PRECISION_LOW,
	GLSL_PRECISION_MEDIUM,
	GLSL_PRECISION_HIGH,
};

enum glsl_storage_qualifier
{
	GLSL_STORAGE_CONST,
	GLSL_STORAGE_IN,
	GLSL_STORAGE_OUT,
	GLSL_STORAGE_INOUT,
	GLSL_STORAGE_CENTROID,
	GLSL_STORAGE_PATCH,
	GLSL_STORAGE_SAMPLE,
	GLSL_STORAGE_UNIFORM,
	GLSL_STORAGE_BUFFER,
	GLSL_STORAGE_SHARED,
	GLSL_STORAGE_COHERENT,
	GLSL_STORAGE_VOLATILE,
	GLSL_STORAGE_RESTRICT,
	GLSL_STORAGE_READONLY,
	GLSL_STORAGE_WRITEONLY,
	GLSL_STORAGE_SUBROUTINE,
};

enum glsl_type
{
	GLSL_TYPE_VOID,
	GLSL_TYPE_FLOAT,
	GLSL_TYPE_DOUBLE,
	GLSL_TYPE_INT,
	GLSL_TYPE_UINT,
	GLSL_TYPE_BOOL,
	GLSL_TYPE_VEC2,
	GLSL_TYPE_VEC3,
	GLSL_TYPE_VEC4,
	GLSL_TYPE_DVEC2,
	GLSL_TYPE_DVEC3,
	GLSL_TYPE_DVEC4,
	GLSL_TYPE_BVEC2,
	GLSL_TYPE_BVEC3,
	GLSL_TYPE_BVEC4,
	GLSL_TYPE_IVEC2,
	GLSL_TYPE_IVEC3,
	GLSL_TYPE_IVEC4,
	GLSL_TYPE_UVEC2,
	GLSL_TYPE_UVEC3,
	GLSL_TYPE_UVEC4,
	GLSL_TYPE_MAT2X2,
	GLSL_TYPE_MAT2X3,
	GLSL_TYPE_MAT2X4,
	GLSL_TYPE_MAT3X2,
	GLSL_TYPE_MAT3X3,
	GLSL_TYPE_MAT3X4,
	GLSL_TYPE_MAT4X2,
	GLSL_TYPE_MAT4X3,
	GLSL_TYPE_MAT4X4,
	GLSL_TYPE_DMAT2X2,
	GLSL_TYPE_DMAT2X3,
	GLSL_TYPE_DMAT2X4,
	GLSL_TYPE_DMAT3X2,
	GLSL_TYPE_DMAT3X3,
	GLSL_TYPE_DMAT3X4,
	GLSL_TYPE_DMAT4X2,
	GLSL_TYPE_DMAT4X3,
	GLSL_TYPE_DMAT4X4,
	GLSL_TYPE_ATOMIC_UINT,
	GLSL_TYPE_SAMPLER2D,
	GLSL_TYPE_SAMPLER3D,
	GLSL_TYPE_SAMPLERCUBE,
	GLSL_TYPE_SAMPLER2DSHADOW,
	GLSL_TYPE_SAMPLERCUBESHADOW,
	GLSL_TYPE_SAMPLER2DARRAY,
	GLSL_TYPE_SAMPLER2DARRAYSHADOW,
	GLSL_TYPE_SAMPLERCUBEARRAY,
	GLSL_TYPE_SAMPLERCUBEARRAYSHADOW,
	GLSL_TYPE_ISAMPLER2D,
	GLSL_TYPE_ISAMPLER3D,
	GLSL_TYPE_ISAMPLERCUBE,
	GLSL_TYPE_ISAMPLER2DARRAY,
	GLSL_TYPE_ISAMPLERCUBEARRAY,
	GLSL_TYPE_USAMPLER2D,
	GLSL_TYPE_USAMPLER3D,
	GLSL_TYPE_USAMPLERCUBE,
	GLSL_TYPE_USAMPLER2DARRAY,
	GLSL_TYPE_USAMPLERCUBEARRAY,
	GLSL_TYPE_SAMPLER1D,
	GLSL_TYPE_SAMPLER1DSHADOW,
	GLSL_TYPE_SAMPLER1DARRAY,
	GLSL_TYPE_SAMPLER1DARRAYSHADOW,
	GLSL_TYPE_ISAMPLER1D,
	GLSL_TYPE_ISAMPLER1DARRAY,
	GLSL_TYPE_USAMPLER1D,
	GLSL_TYPE_USAMPLER1DARRAY,
	GLSL_TYPE_SAMPLER2DRECT,
	GLSL_TYPE_SAMPLER2DRECTSHADOW,
	GLSL_TYPE_ISAMPLER2DRECT,
	GLSL_TYPE_USAMPLER2DRECT,
	GLSL_TYPE_SAMPLERBUFFER,
	GLSL_TYPE_ISAMPLERBUFFER,
	GLSL_TYPE_USAMPLERBUFFER,
	GLSL_TYPE_SAMPLER2DMS,
	GLSL_TYPE_ISAMPLER2DMS,
	GLSL_TYPE_USAMPLER2DMS,
	GLSL_TYPE_SAMPLER2DMSARRAY,
	GLSL_TYPE_ISAMPLER2DMSARRAY,
	GLSL_TYPE_USAMPLER2DMSARRAY,
	GLSL_TYPE_IMAGE2D,
	GLSL_TYPE_IIMAGE2D,
	GLSL_TYPE_UIMAGE2D,
	GLSL_TYPE_IMAGE3D,
	GLSL_TYPE_IIMAGE3D,
	GLSL_TYPE_UIMAGE3D,
	GLSL_TYPE_IMAGECUBE,
	GLSL_TYPE_IIMAGECUBE,
	GLSL_TYPE_UIMAGECUBE,
	GLSL_TYPE_IMAGEBUFFER,
	GLSL_TYPE_IIMAGEBUFFER,
	GLSL_TYPE_UIMAGEBUFFER,
	GLSL_TYPE_IMAGE1D,
	GLSL_TYPE_IIMAGE1D,
	GLSL_TYPE_UIMAGE1D,
	GLSL_TYPE_IMAGE1DARRAY,
	GLSL_TYPE_IIMAGE1DARRAY,
	GLSL_TYPE_UIMAGE1DARRAY,
	GLSL_TYPE_IMAGE2DRECT,
	GLSL_TYPE_IIMAGE2DRECT,
	GLSL_TYPE_UIMAGE2DRECT,
	GLSL_TYPE_IMAGE2DARRAY,
	GLSL_TYPE_IIMAGE2DARRAY,
	GLSL_TYPE_UIMAGE2DARRAY,
	GLSL_TYPE_IMAGECUBEARRAY,
	GLSL_TYPE_IIMAGECUBEARRAY,
	GLSL_TYPE_UIMAGECUBEARRAY,
	GLSL_TYPE_IMAGE2DMS,
	GLSL_TYPE_IIMAGE2DMS,
	GLSL_TYPE_UIMAGE2DMS,
	GLSL_TYPE_IMAGE2DMSARRAY,
	GLSL_TYPE_IIMAGE2DMSARRAY,
	GLSL_TYPE_UIMAGE2DMSARRAY,
};

enum glsl_type_specifier_type
{
	GLSL_TYPE_SPECIFIER_NATIVE,
	GLSL_TYPE_SPECIFIER_STRUCT,
	GLSL_TYPE_SPECIFIER_NAMED,
};

enum glsl_unary_op
{
	GLSL_UNARY_OP_POS,
	GLSL_UNARY_OP_NEG,
	GLSL_UNARY_OP_NOT,
	GLSL_UNARY_OP_INV,
	GLSL_UNARY_OP_PRE_INC,
	GLSL_UNARY_OP_PRE_DEC,
	GLSL_UNARY_OP_POST_INC,
	GLSL_UNARY_OP_POST_DEC,
};

struct glsl_node
{
	enum glsl_node_type type;
	TAILQ_ENTRY(glsl_node) chain;
};

struct glsl_array_accessor_node
{
	struct glsl_node node;
	struct glsl_node *expression;
	struct glsl_node *index;
};

struct glsl_assignment_node
{
	struct glsl_node node;
	enum glsl_assignment_op op;
	struct glsl_node *dst;
	struct glsl_node *expr;
};

struct glsl_binary_op_node
{
	struct glsl_node node;
	enum glsl_binary_op op;
	struct glsl_node *left;
	struct glsl_node *right;
};

struct glsl_block_declaration_node
{
	struct glsl_node node;
	struct glsl_node *type_qualifier;
	struct glsl_node *identifier;
	struct glsl_node *declaration_list;
	struct glsl_node *name;
	struct glsl_node *array_specifier;
};

struct glsl_case_node
{
	struct glsl_node node;
	struct glsl_node *expression;
};

struct glsl_compare_op_node
{
	struct glsl_node node;
	enum glsl_compare_op op;
	struct glsl_node *left;
	struct glsl_node *right;
};

struct glsl_condition_node
{
	struct glsl_node node;
	struct glsl_node *condition;
	struct glsl_node *true_statement;
	struct glsl_node *false_statement;
};

struct glsl_declaration_node
{
	struct glsl_node node;
	struct glsl_node *type;
	struct glsl_node *entries;
};

struct glsl_declaration_entry_node
{
	struct glsl_node node;
	struct glsl_node *identifier;
	struct glsl_node *array_specifier;
	struct glsl_node *initializer;
};

struct glsl_field_accessor_node
{
	struct glsl_node node;
	struct glsl_node *expression;
	struct glsl_node *field;
};

struct glsl_for_node
{
	struct glsl_node node;
	struct glsl_node *initialize;
	struct glsl_node *condition;
	struct glsl_node *iteration;
	struct glsl_node *statement;
};

struct glsl_fully_specified_type_node
{
	struct glsl_node node;
	struct glsl_node *type_specifier;
	struct glsl_node *qualifiers;
};

struct glsl_function_node
{
	struct glsl_node node;
	struct glsl_node *prototype;
	struct glsl_node *content;
};

struct glsl_function_call_node
{
	struct glsl_node node;
	struct glsl_node *identifier;
	struct glsl_node *arguments;
};

struct glsl_identifier_node
{
	struct glsl_node node;
	char *identifier;
};

struct glsl_identifier_declaration_node
{
	struct glsl_node node;
	struct glsl_node *type_qualifier;
	struct glsl_node *identifiers;
};

struct glsl_immediate_bool_node
{
	struct glsl_node node;
	bool value;
};

struct glsl_immediate_double_node
{
	struct glsl_node node;
	double value;
};

struct glsl_immediate_float_node
{
	struct glsl_node node;
	float value;
};

struct glsl_immediate_int_node
{
	struct glsl_node node;
	int32_t value;
};

struct glsl_immediate_uint_node
{
	struct glsl_node node;
	uint32_t value;
};

struct glsl_interpolation_qualifier_node
{
	struct glsl_node node;
	enum glsl_interpolation_qualifier qualifier;
};

struct glsl_invariant_qualifier_node
{
	struct glsl_node node;
};

struct glsl_jump_node
{
	struct glsl_node node;
	enum glsl_jump_type type;
	struct glsl_node *expression;
};

struct glsl_layout_qualifier_node
{
	struct glsl_node node;
	struct glsl_node *identifier;
	struct glsl_node *value;
};

struct glsl_layout_qualifiers_node
{
	struct glsl_node node;
	struct glsl_node *qualifiers;
};

struct glsl_list_node
{
	struct glsl_node node;
	TAILQ_HEAD(, glsl_node) nodes;
};

struct glsl_precise_qualifier_node
{
	struct glsl_node node;
};

struct glsl_precision_declaration_node
{
	struct glsl_node node;
	enum glsl_precision_qualifier precision_qualifier;
	struct glsl_node *type_specifier;
};

struct glsl_precision_qualifier_node
{
	struct glsl_node node;
	enum glsl_precision_qualifier qualifier;
};

struct glsl_prototype_node
{
	struct glsl_node node;
	struct glsl_node *return_type;
	struct glsl_node *identifier;
	struct glsl_node *parameters;
};

struct glsl_prototype_parameter_node
{
	struct glsl_node node;
	struct glsl_node *type_qualifiers;
	struct glsl_node *type_specifier;
	struct glsl_node *identifier;
	struct glsl_node *array_specifier;
};

struct glsl_shared_qualifier_node
{
	struct glsl_node node;
};

struct glsl_storage_qualifier_node
{
	struct glsl_node node;
	enum glsl_storage_qualifier qualifier;
};

struct glsl_struct_definition_node
{
	struct glsl_node node;
	struct glsl_node *identifier;
	struct glsl_node *members;
};

struct glsl_struct_member_node
{
	struct glsl_node node;
	struct glsl_node *type_qualifiers;
	struct glsl_node *type_specifier;
	struct glsl_node *identifiers;
};

struct glsl_struct_member_identifier_node
{
	struct glsl_node node;
	struct glsl_node *identifier;
	struct glsl_node *array_specifier;
};

struct glsl_subroutine_qualifier_node
{
	struct glsl_node node;
	struct glsl_node *qualifiers;
};

struct glsl_switch_node
{
	struct glsl_node node;
	struct glsl_node *expression;
	struct glsl_node *statements;
};

struct glsl_ternary_node
{
	struct glsl_node node;
	struct glsl_node *cond;
	struct glsl_node *true_expr;
	struct glsl_node *false_expr;
};

struct glsl_type_node
{
	struct glsl_node node;
	enum glsl_type type;
};

struct glsl_type_specifier_node
{
	struct glsl_node node;
	struct glsl_node *type;
	struct glsl_node *array_specifier;
};

struct glsl_unary_op_node
{
	struct glsl_node node;
	enum glsl_unary_op op;
	struct glsl_node *expr;
};

struct glsl_while_node
{
	struct glsl_node node;
	bool do_while;
	struct glsl_node *condition;
	struct glsl_node *statement;
};

struct glsl_node *glsl_emit_array_accessor(struct glsl_node *expression,
                                           struct glsl_node *index);
struct glsl_node *glsl_emit_assignment(enum glsl_assignment_op op,
                                       struct glsl_node *dst,
                                       struct glsl_node *expr);
struct glsl_node *glsl_emit_binary_op(enum glsl_binary_op op,
                                      struct glsl_node *left,
                                      struct glsl_node *right);
struct glsl_node *glsl_emit_block_declaration(struct glsl_node *type_qualifier,
                                              struct glsl_node *identifier,
                                              struct glsl_node *declaration_list,
                                              struct glsl_node *name,
                                              struct glsl_node *array_specifier);
struct glsl_node *glsl_emit_case(struct glsl_node *expression);
struct glsl_node *glsl_emit_compare_op(enum glsl_compare_op,
                                       struct glsl_node *left,
                                       struct glsl_node *right);
struct glsl_node *glsl_emit_condition(struct glsl_node *condition,
                                      struct glsl_node *true_statement,
                                      struct glsl_node *false_statement);
struct glsl_node *glsl_emit_declaration(struct glsl_node *type,
                                        struct glsl_node *entries);
struct glsl_node *glsl_emit_declaration_entry(struct glsl_node *identifier,
                                              struct glsl_node *array_specifier,
                                              struct glsl_node *initializer);
struct glsl_node *glsl_emit_field_accessor(struct glsl_node *expression,
                                           struct glsl_node *field);
struct glsl_node *glsl_emit_for(struct glsl_node *initialize,
                                struct glsl_node *condition,
                                struct glsl_node *iteration,
                                struct glsl_node *statement);
struct glsl_node *glsl_emit_fully_specified_type(struct glsl_node *type_specifier,
                                                 struct glsl_node *qualifiers);
struct glsl_node *glsl_emit_function(struct glsl_node *prototype,
                                     struct glsl_node *content);
struct glsl_node *glsl_emit_function_call(struct glsl_node *identifier,
                                          struct glsl_node *arguments);
struct glsl_node *glsl_emit_identifier(const char *identifier);
struct glsl_node *glsl_emit_identifier_declaration(struct glsl_node *type_qualifier,
                                                   struct glsl_node *identifiers);
struct glsl_node *glsl_emit_immediate_bool(bool value);
struct glsl_node *glsl_emit_immediate_double(double value);
struct glsl_node *glsl_emit_immediate_float(float value);
struct glsl_node *glsl_emit_immediate_int(int32_t value);
struct glsl_node *glsl_emit_immediate_uint(uint32_t value);
struct glsl_node *glsl_emit_interpolation_qualifier(enum glsl_interpolation_qualifier qualifier);
struct glsl_node *glsl_emit_invariant_qualifier(void);
struct glsl_node *glsl_emit_jump(enum glsl_jump_type type,
                                 struct glsl_node *expression);
struct glsl_node *glsl_emit_layout_qualifier(struct glsl_node *identifier,
                                             struct glsl_node *value);
struct glsl_node *glsl_emit_layout_qualifiers(struct glsl_node *qualifiers);
struct glsl_node *glsl_emit_list(void);
void glsl_list_add(struct glsl_node *list,
                   struct glsl_node *node);
struct glsl_node *glsl_emit_precise_qualifier(void);
struct glsl_node *glsl_emit_precision_declaration(enum glsl_precision_qualifier precision_qualifier,
                                                  struct glsl_node *type_specifier);
struct glsl_node *glsl_emit_precision_qualifier(enum glsl_precision_qualifier qualifier);
struct glsl_node *glsl_emit_prototype(struct glsl_node *return_type,
                                      struct glsl_node *identifier,
                                      struct glsl_node *parameters);
struct glsl_node *glsl_emit_prototype_parameter(struct glsl_node *type_qualifiers,
                                                struct glsl_node *type_specifier,
                                                struct glsl_node *identifier,
                                                struct glsl_node *array_specifier);
struct glsl_node *glsl_emit_shared_qualifier(void);
struct glsl_node *glsl_emit_storage_qualifier(enum glsl_storage_qualifier qualifier);
struct glsl_node *glsl_emit_struct_definition(struct glsl_node *identifier,
                                             struct glsl_node *members);
struct glsl_node *glsl_emit_struct_member(struct glsl_node *type_qualifiers,
                                          struct glsl_node *type_specifier,
                                          struct glsl_node *identifiers);
struct glsl_node *glsl_emit_struct_member_identifier(struct glsl_node *identifier,
                                                     struct glsl_node *array_specifier);
struct glsl_node *glsl_emit_subroutine_qualifier(struct glsl_node *identifiers);
struct glsl_node *glsl_emit_switch(struct glsl_node *expression,
                                   struct glsl_node *statements);
struct glsl_node *glsl_emit_ternary(struct glsl_node *cond,
                                    struct glsl_node *true_expr,
                                    struct glsl_node *false_expr);
struct glsl_node *glsl_emit_type(enum glsl_type type);
struct glsl_node *glsl_emit_type_specifier(struct glsl_node *type,
                                           struct glsl_node *array_specifier);
struct glsl_node *glsl_emit_unary_op(enum glsl_unary_op op,
                                     struct glsl_node *expr);
struct glsl_node *glsl_emit_while(bool do_while,
                                  struct glsl_node *condition,
                                  struct glsl_node *statement);

void glsl_print(struct glsl_node *node);

#endif
