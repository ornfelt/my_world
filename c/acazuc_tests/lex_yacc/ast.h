#ifndef AST_H
#define AST_H

#include <stddef.h>

enum node_type
{
	NODE_ASSIGNMENT,
	NODE_BINARY_OP,
	NODE_UNARY_OP,
	NODE_COMPARE_OP,
	NODE_TERNARY,
	NODE_STRUCT_DEFINITION,
	NODE_STRUCT_MEMBER,
	NODE_STRUCT_MEMBER_IDENTIFIER,
	NODE_TYPE_QUALIFIER,
	NODE_LAYOUT_QUALIFIER,
	NODE_FUNCTION,
	NODE_FULLY_SPECIFIED_TYPE,
	NODE_PROTOTYPE,
	NODE_PROTOTYPE_PARAMETER,
	NODE_FUNCTION_CALL_ARGUMENT,
	NODE_FUNCTION_CALL,
	NODE_TYPE_SPECIFIER,
};

enum binary_op
{
	BINARY_OP_ADD,
	BINARY_OP_SUB,
	BINARY_OP_MUL,
	BINARY_OP_DIV,
	BINARY_OP_MOD,
	BINARY_OP_LSL,
	BINARY_OP_LSR,
	BINARY_OP_AND,
	BINARY_OP_OR,
	BINARY_OP_XOR,
	BINARY_OP_LAND,
	BINARY_OP_LOR,
	BINARY_OP_LXOR,
};

enum iteration_type
{
	ITERATION_WHILE,
	ITERATION_DO,
	ITERATION_FOR,
};

enum unary_op
{
	UNARY_OP_POS,
	UNARY_OP_NEG,
	UNARY_OP_NOT,
	UNARY_OP_INV,
};

enum compare_op
{
	COMPARE_OP_LT,
	COMPARE_OP_LE,
	COMPARE_OP_EQ,
	COMPARE_OP_NE,
	COMPARE_OP_GE,
	COMPARE_OP_GT,
};

enum assignment_op
{
	ASSIGNMENT_OP_EQ,
	ASSIGNMENT_OP_MUL,
	ASSIGNMENT_OP_DIV,
	ASSIGNMENT_OP_MOD,
	ASSIGNMENT_OP_ADD,
	ASSIGNMENT_OP_SUB,
	ASSIGNMENT_OP_LSL,
	ASSIGNMENT_OP_LSR,
	ASSIGNMENT_OP_AND,
	ASSIGNMENT_OP_XOR,
	ASSIGNMENT_OP_OR,
};

enum storage_qualifier
{
	STORAGE_CONST,
	STORAGE_IN,
	STORAGE_OUT,
	STORAGE_INOUT,
	STORAGE_CENTROID,
	STORAGE_PATCH,
	STORAGE_SAMPLE,
	STORAGE_UNIFORM,
	STORAGE_BUFFER,
	STORAGE_SHARED,
	STORAGE_COHERENT,
	STORAGE_VOLATILE,
	STORAGE_RESTRICT,
	STORAGE_READONLY,
	STORAGE_WRITEONLY,
	STORAGE_SUBROUTINE,
};

enum type_specifier
{
	TYPE_VOID,
	TYPE_FLOAT,
	TYPE_DOUBLE,
	TYPE_INT,
	TYPE_UINT,
	TYPE_BOOL,
	TYPE_VEC2,
	TYPE_VEC3,
	TYPE_VEC4,
	TYPE_DVEC2,
	TYPE_DVEC3,
	TYPE_DVEC4,
	TYPE_BVEC2,
	TYPE_BVEC3,
	TYPE_BVEC4,
	TYPE_IVEC2,
	TYPE_IVEC3,
	TYPE_IVEC4,
	TYPE_UVEC2,
	TYPE_UVEC3,
	TYPE_UVEC4,
	TYPE_MAT2X2,
	TYPE_MAT2X3,
	TYPE_MAT2X4,
	TYPE_MAT3X2,
	TYPE_MAT3X3,
	TYPE_MAT3X4,
	TYPE_MAT4X2,
	TYPE_MAT4X3,
	TYPE_MAT4X4,
	TYPE_DMAT2X2,
	TYPE_DMAT2X3,
	TYPE_DMAT2X4,
	TYPE_DMAT3X2,
	TYPE_DMAT3X3,
	TYPE_DMAT3X4,
	TYPE_DMAT4X2,
	TYPE_DMAT4X3,
	TYPE_DMAT4X4,
	TYPE_ATOMIC_UINT,
	TYPE_SAMPLER2D,
	TYPE_SAMPLER3D,
	TYPE_SAMPLERCUBE,
	TYPE_SAMPLER2DSHADOW,
	TYPE_SAMPLERCUBESHADOW,
	TYPE_SAMPLER2DARRAY,
	TYPE_SAMPLER2DARRAYSHADOW,
	TYPE_SAMPLERCUBEARRAY,
	TYPE_SAMPLERCUBEARRAYSHADOW,
	TYPE_ISAMPLER2D,
	TYPE_ISAMPLER3D,
	TYPE_ISAMPLERCUBE,
	TYPE_ISAMPLER2DARRAY,
	TYPE_ISAMPLERCUBEARRAY,
	TYPE_USAMPLER2D,
	TYPE_USAMPLER3D,
	TYPE_USAMPLERCUBE,
	TYPE_USAMPLER2DARRAY,
	TYPE_USAMPLERCUBEARRAY,
	TYPE_SAMPLER1D,
	TYPE_SAMPLER1DSHADOW,
	TYPE_SAMPLER1DARRAY,
	TYPE_SAMPLER1DARRAYSHADOW,
	TYPE_ISAMPLER1D,
	TYPE_ISAMPLER1DARRAY,
	TYPE_USAMPLER1D,
	TYPE_USAMPLER1DARRAY,
	TYPE_SAMPLER2DRECT,
	TYPE_SAMPLER2DRECTSHADOW,
	TYPE_ISAMPLER2DRECT,
	TYPE_USAMPLER2DRECT,
	TYPE_SAMPLERBUFFER,
	TYPE_ISAMPLERBUFFER,
	TYPE_USAMPLERBUFFER,
	TYPE_SAMPLER2DMS,
	TYPE_ISAMPLER2DMS,
	TYPE_USAMPLER2DMS,
	TYPE_SAMPLER2DMSARRAY,
	TYPE_ISAMPLER2DMSARRAY,
	TYPE_USAMPLER2DMSARRAY,
	TYPE_IMAGE2D,
	TYPE_IIMAGE2D,
	TYPE_UIMAGE2D,
	TYPE_IMAGE3D,
	TYPE_IIMAGE3D,
	TYPE_UIMAGE3D,
	TYPE_IMAGECUBE,
	TYPE_IIMAGECUBE,
	TYPE_UIMAGECUBE,
	TYPE_IMAGEBUFFER,
	TYPE_IIMAGEBUFFER,
	TYPE_UIMAGEBUFFER,
	TYPE_IMAGE1D,
	TYPE_IIMAGE1D,
	TYPE_UIMAGE1D,
	TYPE_IMAGE1DARRAY,
	TYPE_IIMAGE1DARRAY,
	TYPE_UIMAGE1DARRAY,
	TYPE_IMAGE2DRECT,
	TYPE_IIMAGE2DRECT,
	TYPE_UIMAGE2DRECT,
	TYPE_IMAGE2DARRAY,
	TYPE_IIMAGE2DARRAY,
	TYPE_UIMAGE2DARRAY,
	TYPE_IMAGECUBEARRAY,
	TYPE_IIMAGECUBEARRAY,
	TYPE_UIMAGECUBEARRAY,
	TYPE_IMAGE2DMS,
	TYPE_IIMAGE2DMS,
	TYPE_UIMAGE2DMS,
	TYPE_IMAGE2DMSARRAY,
	TYPE_IIMAGE2DMSARRAY,
	TYPE_UIMAGE2DMSARRAY,
};

enum type_specifier_type
{
	TYPE_SPECIFIER_NATIVE,
	TYPE_SPECIFIER_STRUCT,
	TYPE_SPECIFIER_NAMED,
};

enum precision_qualifier
{
	PRECISION_LOW,
	PRECISION_MEDIUM,
	PRECISION_HIGH,
};

enum interpolation_qualifier
{
	INTERPOLATION_SMOOTH,
	INTERPOLATION_FLAT,
	INTERPOLATION_NOPERSPECTIVE,
};

enum invariant_qualifier
{
	INVARIANT_INVARIANT,
};

enum precise_qualifier
{
	PRECISE_PRECISE,
};

enum qualifier_type
{
	QUALIFIER_STORAGE,
	QUALIFIER_LAYOUT,
	QUALIFIER_PRECISION,
	QUALIFIER_INTERPOLATION,
	QUALIFIER_INVARIANT,
	QUALIFIER_PRECISE,
};

struct node
{
	enum node_type type;
};

struct identifier_node
{
	struct node node;
	char *identifier;
};

struct binary_op_node
{
	struct node node;
	enum binary_op op;
	struct node *left;
	struct node *right;
};

struct unary_op_node
{
	struct node node;
	enum unary_op op;
	struct node *expr;
};

struct compare_op_node
{
	struct node node;
	enum compare_op op;
	struct node *left;
	struct node *right;
};

struct assignment_node
{
	struct node node;
	enum assignment_op op;
	struct node *dst;
	struct node *expr;
};

struct cond_node
{
	struct node node;
	struct node *cond;
	struct node *true_node;
	struct node *false_node;
};

struct case_node
{
	struct node node;
	struct node *expr;
	struct node *block;
};

struct switch_node
{
	struct node node;
	size_t cases_nb;
	struct node **cases;
};

struct iteration_node
{
	struct node node;
	enum iteration_type type;
};

struct ternary_node
{
	struct node node;
	struct node *cond;
	struct node *true_expr;
	struct node *false_expr;
};

struct struct_member_identifier_node
{
	struct node node;
	char *identifier;
	struct struct_member_identifier_node *next;
};

struct struct_member_node
{
	struct node node;
	struct node *type_qualifiers;
	struct node *type_specifier;
	struct node *identifiers;
	struct struct_member_node *next;
};

struct struct_definition_node
{
	struct node node;
	char *identifier;
	struct node *members;
};

struct prototype_parameter_node
{
	struct node node;
	struct node *type_qualifiers;
	struct node *type_specifier;
	char *identifier;
	struct prototype_parameter_node *next;
};

struct prototype_node
{
	struct node node;
	struct node *return_type;
	char *identifier;
	struct prototype_parameter_node *parameters;
};

struct function_node
{
	struct node node;
	struct node *prototype;
	struct node *content;
};

struct layout_qualifier_node
{
	struct node node;
	char *identifier;
	struct node *value;
	struct layout_qualifier_node *next;
};

struct type_qualifier_node
{
	struct node node;
	enum qualifier_type type;
	union
	{
		enum storage_qualifier storage_qualifier;
		struct node *layout_qualifier;
		enum precision_qualifier precision_qualifier;
		enum interpolation_qualifier interpolation_qualifier;
		enum invariant_qualifier invariant_qualifier;
		enum precise_qualifier precise_qualifier;
	};
	struct type_qualifier_node *next;
};

struct fully_specified_type_node
{
	struct node node;
	struct node *type_specifier;
	struct node *qualifiers;
};

struct function_call_argument_node
{
	struct node node;
	struct node *expr;
	struct function_call_argument_node *next;
};

struct function_call_node
{
	struct node node;
	struct node *identifier;
	struct node *arguments;
};

struct type_specifier_node
{
	struct node node;
	enum type_specifier_type type;
	union
	{
		enum type_specifier native;
		struct node *st;
		char *identifier;
	};
};

struct node *emit_unary_op_node(enum unary_op op, struct node *expr);
struct node *emit_binary_op_node(enum binary_op op, struct node *left, struct node *right);
struct node *emit_compare_op_node(enum compare_op, struct node *left, struct node *right);
struct node *emit_assignment_node(enum assignment_op op, struct node *dst, struct node *expr);
struct node *emit_ternary_node(struct node *cond, struct node *true_expr, struct node *false_expr);
struct node *emit_struct_member_identifier_node(const char *identifier);
void push_struct_member_identifier(struct node *node, struct node *add);
struct node *emit_struct_member_node(struct node *type_qualifiers, struct node *type_specifier, struct node *identifiers);
void push_struct_member(struct node *node, struct node *add);
struct node *emit_struct_definition_node(const char *identifier, struct node *members);
struct node *emit_function_node(struct node *prototype, struct node *content);
struct node *emit_layout_qualifier_node(const char *identifier, struct node *value);
void push_layout_qualifier(struct node *node, struct node *add);
struct node *emit_storage_qualifier_node(enum storage_qualifier qualifier);
struct node *emit_layout_qualifier_specifier_node(struct node *qualifier);
struct node *emit_precision_qualifier_node(enum precision_qualifier qualifier);
struct node *emit_interpolation_qualifier_node(enum interpolation_qualifier qualifier);
struct node *emit_invariant_qualifier_node(enum invariant_qualifier qualifier);
struct node *emit_precise_qualifier_node(enum precise_qualifier qualifier);
void push_type_qualifier(struct node *node, struct node *add);
struct node *emit_fully_specified_type_node(struct node *type_specifier, struct node *qualifiers);
struct node *emit_prototype_node(struct node *return_type, const char *identifier);
void push_prototype_parameter(struct node *node, struct node *parameter);
struct node *emit_prototype_parameter_node(struct node *type_qualifiers, struct node *type_specifier, const char *identifier);
struct node *emit_function_call_argument(struct node *expr);
void push_function_call_argument(struct node *node, struct node *add);
struct node *emit_function_call_node(struct node *identifier, struct node *arguments);
struct node *emit_type_specifier_native_node(enum type_specifier type_specifier);
struct node *emit_type_specifier_struct_node(struct node *st);
struct node *emit_type_specifier_named_node(const char *identifier);

#endif
