#include "ast.h"

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void *
emit_node(enum glsl_node_type type, size_t size)
{
	struct glsl_node *node;

	node = calloc(size, 1);
	if (!node)
		return NULL;
	node->type = type;
	return node;
}

struct glsl_node *
glsl_emit_array_accessor(struct glsl_node *expression,
                         struct glsl_node *index)
{
	struct glsl_array_accessor_node *node;

	node = emit_node(GLSL_NODE_ARRAY_ACCESSOR, sizeof(*node));
	if (!node)
		return NULL;
	node->expression = expression;
	node->index = index;
	return &node->node;
}

struct glsl_node *
glsl_emit_assignment(enum glsl_assignment_op op,
                     struct glsl_node *dst,
                     struct glsl_node *expr)
{
	struct glsl_assignment_node *node;

	node = emit_node(GLSL_NODE_ASSIGNMENT, sizeof(*node));
	if (!node)
		return NULL;
	node->op = op;
	node->dst = dst;
	node->expr = expr;
	return &node->node;
}

struct glsl_node *
glsl_emit_binary_op(enum glsl_binary_op op,
                    struct glsl_node *left,
                    struct glsl_node *right)
{
	struct glsl_binary_op_node *node;

	node = emit_node(GLSL_NODE_BINARY_OP, sizeof(*node));
	if (!node)
		return NULL;
	node->op = op;
	node->left = left;
	node->right = right;
	return &node->node;
}

struct glsl_node *
glsl_emit_block_declaration(struct glsl_node *type_qualifier,
                            struct glsl_node *identifier,
                            struct glsl_node *declaration_list,
                            struct glsl_node *name,
                            struct glsl_node *array_specifier)
{
	struct glsl_block_declaration_node *node;

	node = emit_node(GLSL_NODE_BLOCK_DECLARATION, sizeof(*node));
	if (!node)
		return NULL;
	node->type_qualifier = type_qualifier;
	node->identifier = identifier;
	node->declaration_list = declaration_list;
	node->name = name;
	node->array_specifier = array_specifier;
	return &node->node;
}

struct glsl_node *
glsl_emit_case(struct glsl_node *expression)
{
	struct glsl_case_node *node;

	node = emit_node(GLSL_NODE_CASE, sizeof(*node));
	if (!node)
		return NULL;
	node->expression = expression;
	return &node->node;
}

struct glsl_node *
glsl_emit_compare_op(enum glsl_compare_op op,
                     struct glsl_node *left,
                     struct glsl_node *right)
{
	struct glsl_compare_op_node *node;

	node = emit_node(GLSL_NODE_COMPARE_OP, sizeof(*node));
	if (!node)
		return NULL;
	node->op = op;
	node->left = left;
	node->right = right;
	return &node->node;
}

struct glsl_node *
glsl_emit_condition(struct glsl_node *condition,
                    struct glsl_node *true_statement,
                    struct glsl_node *false_statement)
{
	struct glsl_condition_node *node;

	node = emit_node(GLSL_NODE_CONDITION, sizeof(*node));
	if (!node)
		return NULL;
	node->condition = condition;
	node->true_statement = true_statement;
	node->false_statement = false_statement;
	return &node->node;
}

struct glsl_node *
glsl_emit_declaration(struct glsl_node *type,
                      struct glsl_node *entries)
{
	struct glsl_declaration_node *node;

	node = emit_node(GLSL_NODE_DECLARATION, sizeof(*node));
	if (!node)
		return NULL;
	node->type = type;
	node->entries = entries;
	return &node->node;
}

struct glsl_node *
glsl_emit_declaration_entry(struct glsl_node *identifier,
                            struct glsl_node *array_specifier,
                            struct glsl_node *initializer)
{
	struct glsl_declaration_entry_node *node;

	node = emit_node(GLSL_NODE_DECLARATION_ENTRY, sizeof(*node));
	if (!node)
		return NULL;
	node->identifier = identifier;
	node->array_specifier = array_specifier;
	node->initializer = initializer;
	return &node->node;
}

struct glsl_node *
glsl_emit_field_accessor(struct glsl_node *expression,
                         struct glsl_node *field)
{
	struct glsl_field_accessor_node *node;

	node = emit_node(GLSL_NODE_FIELD_ACCESSOR, sizeof(*node));
	if (!node)
		return NULL;
	node->expression = expression;
	node->field = field;
	return &node->node;
}

struct glsl_node *
glsl_emit_for(struct glsl_node *initialize,
              struct glsl_node *condition,
              struct glsl_node *iteration,
              struct glsl_node *statement)
{
	struct glsl_for_node *node;

	node = emit_node(GLSL_NODE_FOR, sizeof(*node));
	if (!node)
		return NULL;
	node->initialize = initialize;
	node->condition = condition;
	node->iteration = iteration;
	node->statement = statement;
	return &node->node;
}

struct glsl_node *
glsl_emit_fully_specified_type(struct glsl_node *type_specifier,
                               struct glsl_node *qualifiers)
{
	struct glsl_fully_specified_type_node *node;

	node = emit_node(GLSL_NODE_FULLY_SPECIFIED_TYPE, sizeof(*node));
	if (!node)
		return NULL;
	node->type_specifier = type_specifier;
	node->qualifiers = qualifiers;
	return &node->node;
}

struct glsl_node *
glsl_emit_function(struct glsl_node *prototype,
                   struct glsl_node *content)
{
	struct glsl_function_node *node;

	node = emit_node(GLSL_NODE_FUNCTION, sizeof(*node));
	if (!node)
		return NULL;
	node->prototype = prototype;
	node->content = content;
	return &node->node;
}

struct glsl_node *
glsl_emit_function_call(struct glsl_node *identifier,
                        struct glsl_node *arguments)
{
	struct glsl_function_call_node *node;

	node = emit_node(GLSL_NODE_FUNCTION_CALL, sizeof(*node));
	if (!node)
		return NULL;
	node->identifier = identifier;
	node->arguments = arguments;
	return &node->node;
}

struct glsl_node *
glsl_emit_identifier(const char *identifier)
{
	struct glsl_identifier_node *node;

	node = emit_node(GLSL_NODE_IDENTIFIER, sizeof(*node));
	if (!node)
		return NULL;
	node->identifier = strdup(identifier);
	if (!node->identifier)
	{
		free(node);
		return NULL;
	}
	return &node->node;
}

struct glsl_node *
glsl_emit_identifier_declaration(struct glsl_node *type_qualifier,
                                 struct glsl_node *identifiers)
{
	struct glsl_identifier_declaration_node *node;

	node = emit_node(GLSL_NODE_IDENTIFIER_DECLARATION, sizeof(*node));
	if (!node)
		return NULL;
	node->type_qualifier = type_qualifier;
	node->identifiers = identifiers;
	return &node->node;
}

struct glsl_node *
glsl_emit_immediate_bool(bool value)
{
	struct glsl_immediate_bool_node *node;

	node = emit_node(GLSL_NODE_IMMEDIATE_BOOL, sizeof(*node));
	if (!node)
		return NULL;
	node->value = value;
	return &node->node;
}

struct glsl_node *
glsl_emit_immediate_double(double value)
{
	struct glsl_immediate_double_node *node;

	node = emit_node(GLSL_NODE_IMMEDIATE_DOUBLE, sizeof(*node));
	if (!node)
		return NULL;
	node->value = value;
	return &node->node;
}

struct glsl_node *
glsl_emit_immediate_float(float value)
{
	struct glsl_immediate_float_node *node;

	node = emit_node(GLSL_NODE_IMMEDIATE_FLOAT, sizeof(*node));
	if (!node)
		return NULL;
	node->value = value;
	return &node->node;
}

struct glsl_node *
glsl_emit_immediate_int(int32_t value)
{
	struct glsl_immediate_int_node *node;

	node = emit_node(GLSL_NODE_IMMEDIATE_INT, sizeof(*node));
	if (!node)
		return NULL;
	node->value = value;
	return &node->node;
}

struct glsl_node *
glsl_emit_immediate_uint(uint32_t value)
{
	struct glsl_immediate_uint_node *node;

	node = emit_node(GLSL_NODE_IMMEDIATE_UINT, sizeof(*node));
	if (!node)
		return NULL;
	node->value = value;
	return &node->node;
}

struct glsl_node *
glsl_emit_interpolation_qualifier(enum glsl_interpolation_qualifier qualifier)
{
	struct glsl_interpolation_qualifier_node *node;

	node = emit_node(GLSL_NODE_INTERPOLATION_QUALIFIER, sizeof(*node));
	if (!node)
		return NULL;
	node->qualifier = qualifier;
	return &node->node;
}

struct glsl_node *
glsl_emit_invariant_qualifier(void)
{
	struct glsl_invariant_qualifier_node *node;

	node = emit_node(GLSL_NODE_INVARIANT_QUALIFIER, sizeof(*node));
	if (!node)
		return NULL;
	return &node->node;
}

struct glsl_node *
glsl_emit_jump(enum glsl_jump_type type,
               struct glsl_node *expression)
{
	struct glsl_jump_node *node;

	node = emit_node(GLSL_NODE_JUMP, sizeof(*node));
	if (!node)
		return NULL;
	node->type = type;
	node->expression = expression;
	return &node->node;
}

struct glsl_node *
glsl_emit_layout_qualifier(struct glsl_node *identifier,
                           struct glsl_node *value)
{
	struct glsl_layout_qualifier_node *node;

	node = emit_node(GLSL_NODE_LAYOUT_QUALIFIER, sizeof(*node));
	if (!node)
		return NULL;
	node->identifier = identifier;
	node->value = value;
	return &node->node;
}

struct glsl_node *
glsl_emit_layout_qualifiers(struct glsl_node *qualifiers)
{
	struct glsl_layout_qualifiers_node *node;

	node = emit_node(GLSL_NODE_LAYOUT_QUALIFIERS, sizeof(*node));
	if (!node)
		return NULL;
	node->qualifiers = qualifiers;
	return &node->node;
}

struct glsl_node *
glsl_emit_list(void)
{
	struct glsl_list_node *node;

	node = emit_node(GLSL_NODE_LIST, sizeof(*node));
	if (!node)
		return NULL;
	TAILQ_INIT(&node->nodes);
	return &node->node;
}

void
glsl_list_add(struct glsl_node *list, struct glsl_node *node)
{
	if (!node)
		return; /* XXX */
	TAILQ_INSERT_TAIL(&((struct glsl_list_node*)list)->nodes, node, chain);
}

struct glsl_node *
glsl_emit_precise_qualifier(void)
{
	struct glsl_precise_qualifier_node *node;

	node = emit_node(GLSL_NODE_PRECISE_QUALIFIER, sizeof(*node));
	if (!node)
		return NULL;
	return &node->node;
}

struct glsl_node *
glsl_emit_precision_declaration(enum glsl_precision_qualifier precision_qualifier,
                                struct glsl_node *type_specifier)
{
	struct glsl_precision_declaration_node *node;

	node = emit_node(GLSL_NODE_PRECISION_DECLARATION, sizeof(*node));
	if (!node)
		return NULL;
	node->precision_qualifier = precision_qualifier;
	node->type_specifier = type_specifier;
	return &node->node;
}

struct glsl_node *
glsl_emit_precision_qualifier(enum glsl_precision_qualifier qualifier)
{
	struct glsl_precision_qualifier_node *node;

	node = emit_node(GLSL_NODE_PRECISION_QUALIFIER, sizeof(*node));
	if (!node)
		return NULL;
	node->qualifier = qualifier;
	return &node->node;
}

struct glsl_node *
glsl_emit_prototype(struct glsl_node *return_type,
                    struct glsl_node *identifier,
                    struct glsl_node *parameters)
{
	struct glsl_prototype_node *node;

	node = emit_node(GLSL_NODE_PROTOTYPE, sizeof(*node));
	if (!node)
		return NULL;
	node->return_type = return_type;
	node->parameters = parameters;
	node->identifier = identifier;
	return &node->node;
}

struct glsl_node *
glsl_emit_prototype_parameter(struct glsl_node *type_qualifiers,
                              struct glsl_node *type_specifier,
                              struct glsl_node *identifier,
                              struct glsl_node *array_specifier)
{
	struct glsl_prototype_parameter_node *node;

	node = emit_node(GLSL_NODE_PROTOTYPE_PARAMETER, sizeof(*node));
	if (!node)
		return NULL;
	node->type_qualifiers = type_qualifiers;
	node->type_specifier = type_specifier;
	node->array_specifier = array_specifier;
	node->identifier = identifier;
	return &node->node;
}

struct glsl_node *
glsl_emit_shared_qualifier(void)
{
	struct glsl_shared_qualifier_node *node;

	node = emit_node(GLSL_NODE_SHARED_QUALIFIER, sizeof(*node));
	if (!node)
		return NULL;
	return &node->node;
}

struct glsl_node *
glsl_emit_storage_qualifier(enum glsl_storage_qualifier qualifier)
{
	struct glsl_storage_qualifier_node *node;

	node = emit_node(GLSL_NODE_STORAGE_QUALIFIER, sizeof(*node));
	if (!node)
		return NULL;
	node->qualifier = qualifier;
	return &node->node;
}

struct glsl_node *
glsl_emit_struct_definition(struct glsl_node *identifier,
                            struct glsl_node *members)
{
	struct glsl_struct_definition_node *node;

	node = emit_node(GLSL_NODE_STRUCT_DEFINITION, sizeof(*node));
	if (!node)
		return NULL;
	node->identifier = identifier;
	node->members = members;
	return &node->node;
}

struct glsl_node *
glsl_emit_struct_member(struct glsl_node *type_qualifiers,
                        struct glsl_node *type_specifier,
                        struct glsl_node *identifiers)
{
	struct glsl_struct_member_node *node;

	node = emit_node(GLSL_NODE_STRUCT_MEMBER, sizeof(*node));
	if (!node)
		return NULL;
	node->type_qualifiers = type_qualifiers;
	node->type_specifier = type_specifier;
	node->identifiers = identifiers;
	return &node->node;
}

struct glsl_node *
glsl_emit_struct_member_identifier(struct glsl_node *identifier,
                                   struct glsl_node *array_specifier)
{
	struct glsl_struct_member_identifier_node *node;

	node = emit_node(GLSL_NODE_STRUCT_MEMBER_IDENTIFIER, sizeof(*node));
	if (!node)
		return NULL;
	node->identifier = identifier;
	node->array_specifier = array_specifier;
	return &node->node;
}

struct glsl_node *
glsl_emit_subroutine_qualifier(struct glsl_node *identifiers)
{
	struct glsl_subroutine_qualifier_node *node;

	node = emit_node(GLSL_NODE_SUBROUTINE_QUALIFIER, sizeof(*node));
	if (!node)
		return NULL;
	node->qualifiers = identifiers;
	return &node->node;
}

struct glsl_node *
glsl_emit_switch(struct glsl_node *expression,
                 struct glsl_node *statements)
{
	struct glsl_switch_node *node;

	node = emit_node(GLSL_NODE_SWITCH, sizeof(*node));
	if (!node)
		return NULL;
	node->expression = expression;
	node->statements = statements;
	return &node->node;
}

struct glsl_node *
glsl_emit_ternary(struct glsl_node *cond,
                  struct glsl_node *true_expr,
                  struct glsl_node *false_expr)
{
	struct glsl_ternary_node *node;

	node = emit_node(GLSL_NODE_TERNARY, sizeof(*node));
	if (!node)
		return NULL;
	node->cond = cond;
	node->true_expr = true_expr;
	node->false_expr = false_expr;
	return &node->node;
}

struct glsl_node *
glsl_emit_type(enum glsl_type type)
{
	struct glsl_type_node *node;

	node = emit_node(GLSL_NODE_TYPE, sizeof(*node));
	if (!node)
		return NULL;
	node->type = type;
	return &node->node;
}

struct glsl_node *
glsl_emit_type_specifier(struct glsl_node *type,
                         struct glsl_node *array_specifier)
{
	struct glsl_type_specifier_node *node;

	node = emit_node(GLSL_NODE_TYPE_SPECIFIER, sizeof(*node));
	if (!node)
		return NULL;
	node->type = type;
	node->array_specifier = array_specifier;
	return &node->node;
}

struct glsl_node *
glsl_emit_unary_op(enum glsl_unary_op op,
                   struct glsl_node *expr)
{
	struct glsl_unary_op_node *node;

	node = emit_node(GLSL_NODE_UNARY_OP, sizeof(*node));
	if (!node)
		return NULL;
	node->op = op;
	node->expr = expr;
	return &node->node;
}

struct glsl_node *
glsl_emit_while(bool do_while,
                struct glsl_node *condition,
                struct glsl_node *statement)
{
	struct glsl_while_node *node;

	node = emit_node(GLSL_NODE_WHILE, sizeof(*node));
	if (!node)
		return NULL;
	node->do_while = do_while;
	node->condition = condition;
	node->statement = statement;
	return &node->node;
}

static void
print_indent(size_t indent)
{
	printf("%*s", (int)indent * 2, "");
}

static const char *
node_type_strings[] =
{
	[GLSL_NODE_ARRAY_ACCESSOR]           = "ARRAY_ACCESSOR",
	[GLSL_NODE_ASSIGNMENT]               = "ASSIGNMENT",
	[GLSL_NODE_BINARY_OP]                = "BINARY_OP",
	[GLSL_NODE_BLOCK_DECLARATION]        = "BLOCK_DECLARATION",
	[GLSL_NODE_CASE]                     = "CASE",
	[GLSL_NODE_COMPARE_OP]               = "COMPARE_OP",
	[GLSL_NODE_CONDITION]                = "CONDITION",
	[GLSL_NODE_DECLARATION]              = "DECLARATION",
	[GLSL_NODE_DECLARATION_ENTRY]        = "DECLARATION_ENTRY",
	[GLSL_NODE_FIELD_ACCESSOR]           = "FIELD_ACCESSOR",
	[GLSL_NODE_FOR]                      = "FOR",
	[GLSL_NODE_FULLY_SPECIFIED_TYPE]     = "FULLY_SPECIFIED_TYPE",
	[GLSL_NODE_FUNCTION]                 = "FUNCTION",
	[GLSL_NODE_FUNCTION_CALL]            = "FUNCTION_CALL",
	[GLSL_NODE_IDENTIFIER]               = "IDENTIFIER",
	[GLSL_NODE_IDENTIFIER_DECLARATION]   = "IDENTIFIER_DECLARATION",
	[GLSL_NODE_IMMEDIATE_BOOL]           = "IMMEDIATE_BOOL",
	[GLSL_NODE_IMMEDIATE_DOUBLE]         = "IMMEDIATE_DOUBLE",
	[GLSL_NODE_IMMEDIATE_FLOAT]          = "IMMEDIATE_FLOAT",
	[GLSL_NODE_IMMEDIATE_INT]            = "IMMEDIATE_INT",
	[GLSL_NODE_IMMEDIATE_UINT]           = "IMMEDIATE_UINT",
	[GLSL_NODE_INTERPOLATION_QUALIFIER]  = "INTERPOLATION_QUALIFIER",
	[GLSL_NODE_INVARIANT_QUALIFIER]      = "INVARIANT_QUALIFIER",
	[GLSL_NODE_JUMP]                     = "JUMP",
	[GLSL_NODE_LAYOUT_QUALIFIER]         = "LAYOUT_QUALIFIER",
	[GLSL_NODE_LAYOUT_QUALIFIERS]        = "LAYOUT_QUALIFIERS",
	[GLSL_NODE_LIST]                     = "LIST",
	[GLSL_NODE_PRECISE_QUALIFIER]        = "PRECISE_QUALIFIER",
	[GLSL_NODE_PRECISION_DECLARATION]    = "PRECISION_DECLARATION",
	[GLSL_NODE_PRECISION_QUALIFIER]      = "PRECISION_QUALIFIER",
	[GLSL_NODE_PROTOTYPE]                = "PROTOTYPE",
	[GLSL_NODE_PROTOTYPE_PARAMETER]      = "PROTOTYPE_PARAMETER",
	[GLSL_NODE_SHARED_QUALIFIER]         = "SHARED_QUALIFIER",
	[GLSL_NODE_STORAGE_QUALIFIER]        = "STORAGE_QUALIFIER",
	[GLSL_NODE_STRUCT_DEFINITION]        = "STRUCT_DEFINITION",
	[GLSL_NODE_STRUCT_MEMBER]            = "STRUCT_MEMBER",
	[GLSL_NODE_STRUCT_MEMBER_IDENTIFIER] = "STRUCT_MEMBER_IDENTIFIER",
	[GLSL_NODE_SUBROUTINE_QUALIFIER]     = "SUBROUTINE_QUALIFIER",
	[GLSL_NODE_SWITCH]                   = "SWITCH",
	[GLSL_NODE_TERNARY]                  = "TERNARY",
	[GLSL_NODE_TYPE]                     = "TYPE",
	[GLSL_NODE_TYPE_SPECIFIER]           = "TYPE_SPECIFIER",
	[GLSL_NODE_UNARY_OP]                 = "UNARY_OP",
	[GLSL_NODE_WHILE]                    = "WHILE",
};

static const char *
assignment_strings[] =
{
	[GLSL_ASSIGNMENT_OP_EQ]  = "EQ",
	[GLSL_ASSIGNMENT_OP_MUL] = "MUL",
	[GLSL_ASSIGNMENT_OP_DIV] = "DIV",
	[GLSL_ASSIGNMENT_OP_MOD] = "MOD",
	[GLSL_ASSIGNMENT_OP_ADD] = "ADD",
	[GLSL_ASSIGNMENT_OP_SUB] = "SUB",
	[GLSL_ASSIGNMENT_OP_LSL] = "LSL",
	[GLSL_ASSIGNMENT_OP_LSR] = "LSR",
	[GLSL_ASSIGNMENT_OP_AND] = "AND",
	[GLSL_ASSIGNMENT_OP_XOR] = "XOR",
	[GLSL_ASSIGNMENT_OP_OR]  = "OR",
};

static const char *
binary_op_strings[] =
{
	[GLSL_BINARY_OP_ADD]  = "ADD",
	[GLSL_BINARY_OP_SUB]  = "SUB",
	[GLSL_BINARY_OP_MUL]  = "MUL",
	[GLSL_BINARY_OP_DIV]  = "DIV",
	[GLSL_BINARY_OP_MOD]  = "MOD",
	[GLSL_BINARY_OP_LSL]  = "LSL",
	[GLSL_BINARY_OP_LSR]  = "LSR",
	[GLSL_BINARY_OP_AND]  = "AND",
	[GLSL_BINARY_OP_OR]   = "OR",
	[GLSL_BINARY_OP_XOR]  = "XOR",
	[GLSL_BINARY_OP_LAND] = "LAND",
	[GLSL_BINARY_OP_LOR]  = "LOR",
	[GLSL_BINARY_OP_LXOR] = "LXOR",
};

static const char *
compare_op_strings[] =
{
	[GLSL_COMPARE_OP_LT] = "LT",
	[GLSL_COMPARE_OP_LE] = "LE",
	[GLSL_COMPARE_OP_EQ] = "EQ",
	[GLSL_COMPARE_OP_NE] = "NE",
	[GLSL_COMPARE_OP_GE] = "GE",
	[GLSL_COMPARE_OP_GT] = "GT",
};

static const char *
interpolation_qualifier_strings[] =
{
	[GLSL_INTERPOLATION_SMOOTH]        = "SMOOTH",
	[GLSL_INTERPOLATION_FLAT]          = "FLAT",
	[GLSL_INTERPOLATION_NOPERSPECTIVE] = "NOPERSPECTIVE",
};

static const char *
jump_type_strings[] =
{
	[GLSL_JUMP_CONTINUE] = "CONTINUE",
	[GLSL_JUMP_BREAK]    = "BREAK",
	[GLSL_JUMP_RETURN]   = "RETURN",
	[GLSL_JUMP_DISCARD]  = "DISCARD",
};

static const char *
precision_qualifier_strings[] =
{
	[GLSL_PRECISION_LOW]    = "LOW",
	[GLSL_PRECISION_MEDIUM] = "MEDIUM",
	[GLSL_PRECISION_HIGH]   = "HIGH",
};

static const char *
storage_qualifier_strings[] =
{
	[GLSL_STORAGE_CONST]      = "CONST",
	[GLSL_STORAGE_IN]         = "IN",
	[GLSL_STORAGE_OUT]        = "OUT",
	[GLSL_STORAGE_INOUT]      = "INOUT",
	[GLSL_STORAGE_CENTROID]   = "CENTROID",
	[GLSL_STORAGE_PATCH]      = "PATCH",
	[GLSL_STORAGE_SAMPLE]     = "SAMPLE",
	[GLSL_STORAGE_UNIFORM]    = "UNIFORM",
	[GLSL_STORAGE_BUFFER]     = "BUFFER",
	[GLSL_STORAGE_SHARED]     = "SHARED",
	[GLSL_STORAGE_COHERENT]   = "COHERENT",
	[GLSL_STORAGE_VOLATILE]   = "VOLATILE",
	[GLSL_STORAGE_RESTRICT]   = "RESTRICT",
	[GLSL_STORAGE_READONLY]   = "READONLY",
	[GLSL_STORAGE_WRITEONLY]  = "WRITEONLY",
	[GLSL_STORAGE_SUBROUTINE] = "SUBROUTINE",
};

static const char *
type_strings[] =
{
	[GLSL_TYPE_VOID]                   = "VOID",
	[GLSL_TYPE_FLOAT]                  = "FLOAT",
	[GLSL_TYPE_DOUBLE]                 = "DOUBLE",
	[GLSL_TYPE_INT]                    = "INT",
	[GLSL_TYPE_UINT]                   = "UINT",
	[GLSL_TYPE_BOOL]                   = "BOOL",
	[GLSL_TYPE_VEC2]                   = "VEC2",
	[GLSL_TYPE_VEC3]                   = "VEC3",
	[GLSL_TYPE_VEC4]                   = "VEC4",
	[GLSL_TYPE_DVEC2]                  = "DVEC2",
	[GLSL_TYPE_DVEC3]                  = "DVEC3",
	[GLSL_TYPE_DVEC4]                  = "DVEC4",
	[GLSL_TYPE_BVEC2]                  = "BVEC2",
	[GLSL_TYPE_BVEC3]                  = "BVEC3",
	[GLSL_TYPE_BVEC4]                  = "BVEC4",
	[GLSL_TYPE_IVEC2]                  = "IVEC2",
	[GLSL_TYPE_IVEC3]                  = "IVEC3",
	[GLSL_TYPE_IVEC4]                  = "IVEC4",
	[GLSL_TYPE_UVEC2]                  = "UVEC2",
	[GLSL_TYPE_UVEC3]                  = "UVEC3",
	[GLSL_TYPE_UVEC4]                  = "UVEC4",
	[GLSL_TYPE_MAT2X2]                 = "MAT2X2",
	[GLSL_TYPE_MAT2X3]                 = "MAT2X3",
	[GLSL_TYPE_MAT2X4]                 = "MAT2X4",
	[GLSL_TYPE_MAT3X2]                 = "MAT3X2",
	[GLSL_TYPE_MAT3X3]                 = "MAT3X3",
	[GLSL_TYPE_MAT3X4]                 = "MAT3X4",
	[GLSL_TYPE_MAT4X2]                 = "MAT4X2",
	[GLSL_TYPE_MAT4X3]                 = "MAT4X3",
	[GLSL_TYPE_MAT4X4]                 = "MAT4X4",
	[GLSL_TYPE_DMAT2X2]                = "DMAT2X2",
	[GLSL_TYPE_DMAT2X3]                = "DMAT2X3",
	[GLSL_TYPE_DMAT2X4]                = "DMAT2X4",
	[GLSL_TYPE_DMAT3X2]                = "DMAT3X2",
	[GLSL_TYPE_DMAT3X3]                = "DMAT3X3",
	[GLSL_TYPE_DMAT3X4]                = "DMAT3X4",
	[GLSL_TYPE_DMAT4X2]                = "DMAT4X2",
	[GLSL_TYPE_DMAT4X3]                = "DMAT4X3",
	[GLSL_TYPE_DMAT4X4]                = "DMAT4X4",
	[GLSL_TYPE_ATOMIC_UINT]            = "ATOMIC_UINT",
	[GLSL_TYPE_SAMPLER2D]              = "SAMPLER2D",
	[GLSL_TYPE_SAMPLER3D]              = "SAMPLER3D",
	[GLSL_TYPE_SAMPLERCUBE]            = "SAMPLERCUBE",
	[GLSL_TYPE_SAMPLER2DSHADOW]        = "SAMPLER2DSHADOW",
	[GLSL_TYPE_SAMPLERCUBESHADOW]      = "SAMPLERCUBESHADOW",
	[GLSL_TYPE_SAMPLER2DARRAY]         = "SAMPLER2DARRAY",
	[GLSL_TYPE_SAMPLER2DARRAYSHADOW]   = "SAMPLER2DARRAYSHADOW",
	[GLSL_TYPE_SAMPLERCUBEARRAY]       = "SAMPLERCUBEARRAY",
	[GLSL_TYPE_SAMPLERCUBEARRAYSHADOW] = "SAMPLERCUBEARRAYSHADOW",
	[GLSL_TYPE_ISAMPLER2D]             = "ISAMPLER2D",
	[GLSL_TYPE_ISAMPLER3D]             = "ISAMPLER3D",
	[GLSL_TYPE_ISAMPLERCUBE]           = "ISAMPLERCUBE",
	[GLSL_TYPE_ISAMPLER2DARRAY]        = "ISAMPLER2DARRAY",
	[GLSL_TYPE_ISAMPLERCUBEARRAY]      = "ISAMPLERCUBEARRAY",
	[GLSL_TYPE_USAMPLER2D]             = "USAMPLER2D",
	[GLSL_TYPE_USAMPLER3D]             = "USAMPLER3D",
	[GLSL_TYPE_USAMPLERCUBE]           = "USAMPLERCUBE",
	[GLSL_TYPE_USAMPLER2DARRAY]        = "USAMPLER2DARRAY",
	[GLSL_TYPE_USAMPLERCUBEARRAY]      = "USAMPLERCUBEARRAY",
	[GLSL_TYPE_SAMPLER1D]              = "SAMPLER1D",
	[GLSL_TYPE_SAMPLER1DSHADOW]        = "SAMPLER1DSHADOW",
	[GLSL_TYPE_SAMPLER1DARRAY]         = "SAMPLER1DARRAY",
	[GLSL_TYPE_SAMPLER1DARRAYSHADOW]   = "SAMPLER1DARRAYSHADOW",
	[GLSL_TYPE_ISAMPLER1D]             = "ISAMPLER1D",
	[GLSL_TYPE_ISAMPLER1DARRAY]        = "ISAMPLER1DARRAY",
	[GLSL_TYPE_USAMPLER1D]             = "USAMPLER1D",
	[GLSL_TYPE_USAMPLER1DARRAY]        = "USAMPLER1DARRAY",
	[GLSL_TYPE_SAMPLER2DRECT]          = "SAMPLER2DRECT",
	[GLSL_TYPE_SAMPLER2DRECTSHADOW]    = "SAMPLER2DRECTSHADOW",
	[GLSL_TYPE_ISAMPLER2DRECT]         = "ISAMPLER2DRECT",
	[GLSL_TYPE_USAMPLER2DRECT]         = "USAMPLER2DRECT",
	[GLSL_TYPE_SAMPLERBUFFER]          = "SAMPLERBUFFER",
	[GLSL_TYPE_ISAMPLERBUFFER]         = "ISAMPLERBUFFER",
	[GLSL_TYPE_USAMPLERBUFFER]         = "USAMPLERBUFFER",
	[GLSL_TYPE_SAMPLER2DMS]            = "SAMPLER2DMS",
	[GLSL_TYPE_ISAMPLER2DMS]           = "ISAMPLER2DMS",
	[GLSL_TYPE_USAMPLER2DMS]           = "USAMPLER2DMS",
	[GLSL_TYPE_SAMPLER2DMSARRAY]       = "SAMPLER2DMSARRAY",
	[GLSL_TYPE_ISAMPLER2DMSARRAY]      = "ISAMPLER2DMSARRAY",
	[GLSL_TYPE_USAMPLER2DMSARRAY]      = "USAMPLER2DMSARRAY",
	[GLSL_TYPE_IMAGE2D]                = "IMAGE2D",
	[GLSL_TYPE_IIMAGE2D]               = "IIMAGE2D",
	[GLSL_TYPE_UIMAGE2D]               = "UIMAGE2D",
	[GLSL_TYPE_IMAGE3D]                = "IMAGE3D",
	[GLSL_TYPE_IIMAGE3D]               = "IIMAGE3D",
	[GLSL_TYPE_UIMAGE3D]               = "UIMAGE3D",
	[GLSL_TYPE_IMAGECUBE]              = "IMAGECUBE",
	[GLSL_TYPE_IIMAGECUBE]             = "IIMAGECUBE",
	[GLSL_TYPE_UIMAGECUBE]             = "UIMAGECUBE",
	[GLSL_TYPE_IMAGEBUFFER]            = "IMAGEBUFFER",
	[GLSL_TYPE_IIMAGEBUFFER]           = "IIMAGEBUFFER",
	[GLSL_TYPE_UIMAGEBUFFER]           = "UIMAGEBUFFER",
	[GLSL_TYPE_IMAGE1D]                = "IMAGE1D",
	[GLSL_TYPE_IIMAGE1D]               = "IIMAGE1D",
	[GLSL_TYPE_UIMAGE1D]               = "UIMAGE1D",
	[GLSL_TYPE_IMAGE1DARRAY]           = "IMAGE1DARRAY",
	[GLSL_TYPE_IIMAGE1DARRAY]          = "IIMAGE1DARRAY",
	[GLSL_TYPE_UIMAGE1DARRAY]          = "UIMAGE1DARRAY",
	[GLSL_TYPE_IMAGE2DRECT]            = "IMAGE2DRECT",
	[GLSL_TYPE_IIMAGE2DRECT]           = "IIMAGE2DRECT",
	[GLSL_TYPE_UIMAGE2DRECT]           = "UIMAGE2DRECT",
	[GLSL_TYPE_IMAGE2DARRAY]           = "IMAGE2DARRAY",
	[GLSL_TYPE_IIMAGE2DARRAY]          = "IIMAGE2DARRAY",
	[GLSL_TYPE_UIMAGE2DARRAY]          = "UIMAGE2DARRAY",
	[GLSL_TYPE_IMAGECUBEARRAY]         = "IMAGECUBEARRAY",
	[GLSL_TYPE_IIMAGECUBEARRAY]        = "IIMAGECUBEARRAY",
	[GLSL_TYPE_UIMAGECUBEARRAY]        = "UIMAGECUBEARRAY",
	[GLSL_TYPE_IMAGE2DMS]              = "IMAGE2DMS",
	[GLSL_TYPE_IIMAGE2DMS]             = "IIMAGE2DMS",
	[GLSL_TYPE_UIMAGE2DMS]             = "UIMAGE2DMS",
	[GLSL_TYPE_IMAGE2DMSARRAY]         = "IMAGE2DMSARRAY",
	[GLSL_TYPE_IIMAGE2DMSARRAY]        = "IIMAGE2DMSARRAY",
	[GLSL_TYPE_UIMAGE2DMSARRAY]        = "UIMAGE2DMSARRAY",
};

static const char *
unary_op_strings[] =
{
	[GLSL_UNARY_OP_POS]      = "POS",
	[GLSL_UNARY_OP_NEG]      = "NEG",
	[GLSL_UNARY_OP_NOT]      = "NOT",
	[GLSL_UNARY_OP_INV]      = "INV",
	[GLSL_UNARY_OP_PRE_INC]  = "PRE_INC",
	[GLSL_UNARY_OP_PRE_DEC]  = "PRE_DEC",
	[GLSL_UNARY_OP_POST_INC] = "POST_INC",
	[GLSL_UNARY_OP_POST_DEC] = "POST_DEC",
};

static void print_node(struct glsl_node *node, size_t indent);

static void
print_array_accessor(struct glsl_node *nodep, size_t indent)
{
	struct glsl_array_accessor_node *node;

	node = (struct glsl_array_accessor_node*)nodep;
	print_node(node->expression, indent + 1);
	print_node(node->index, indent + 1);
}

static void
print_assignment(struct glsl_node *nodep, size_t indent)
{
	struct glsl_assignment_node *node;

	node = (struct glsl_assignment_node*)nodep;
	print_indent(indent + 1);
	printf("%s\n", assignment_strings[node->op]);
	print_node(node->dst, indent + 1);
	print_node(node->expr, indent + 1);
}

static void
print_binary_op(struct glsl_node *nodep, size_t indent)
{
	struct glsl_binary_op_node *node;

	node = (struct glsl_binary_op_node*)nodep;
	print_indent(indent + 1);
	printf("%s\n", binary_op_strings[node->op]);
	print_node(node->left, indent + 1);
	print_node(node->right, indent + 1);
}

static void
print_block_declaration(struct glsl_node *nodep, size_t indent)
{
	struct glsl_block_declaration_node *node;

	node = (struct glsl_block_declaration_node*)nodep;
	print_node(node->type_qualifier, indent + 1);
	print_node(node->identifier, indent + 1);
	print_node(node->declaration_list, indent + 1);
	if (node->name)
		print_node(node->name, indent + 1);
	if (node->array_specifier)
		print_node(node->array_specifier, indent + 1);
}

static void
print_case(struct glsl_node *nodep, size_t indent)
{
	struct glsl_case_node *node;

	node = (struct glsl_case_node*)nodep;
	print_node(node->expression, indent + 1);
}

static void
print_compare_op(struct glsl_node *nodep, size_t indent)
{
	struct glsl_compare_op_node *node;

	node = (struct glsl_compare_op_node*)nodep;
	print_indent(indent + 1);
	printf("%s\n", compare_op_strings[node->op]);
	print_node(node->left, indent + 1);
	print_node(node->right, indent + 1);
}

static void
print_condition(struct glsl_node *nodep, size_t indent)
{
	struct glsl_condition_node *node;

	node = (struct glsl_condition_node*)nodep;
	print_node(node->condition, indent + 1);
	if (node->true_statement)
		print_node(node->true_statement, indent + 1);
	if (node->false_statement)
		print_node(node->false_statement, indent + 1);
}

static void
print_declaration(struct glsl_node *nodep, size_t indent)
{
	struct glsl_declaration_node *node;

	node = (struct glsl_declaration_node*)nodep;
	print_node(node->type, indent + 1);
	if (node->entries)
		print_node(node->entries, indent + 1);
}

static void
print_declaration_entry(struct glsl_node *nodep, size_t indent)
{
	struct glsl_declaration_entry_node *node;

	node = (struct glsl_declaration_entry_node*)nodep;
	print_node(node->identifier, indent + 1);
	if (node->array_specifier)
		print_node(node->array_specifier, indent + 1);
	if (node->initializer)
		print_node(node->initializer, indent + 1);
}

static void
print_field_accessor(struct glsl_node *nodep, size_t indent)
{
	struct glsl_field_accessor_node *node;

	node = (struct glsl_field_accessor_node*)nodep;
	print_node(node->expression, indent + 1);
	print_node(node->field, indent + 1);
}

static void
print_for(struct glsl_node *nodep, size_t indent)
{
	struct glsl_for_node *node;

	node = (struct glsl_for_node*)nodep;
	print_node(node->initialize, indent + 1);
	print_node(node->condition, indent + 1);
	print_node(node->iteration, indent + 1);
	print_node(node->statement, indent + 1);
}

static void
print_fully_specified_type(struct glsl_node *nodep, size_t indent)
{
	struct glsl_fully_specified_type_node *node;

	node = (struct glsl_fully_specified_type_node*)nodep;
	if (node->type_specifier)
		print_node(node->type_specifier, indent + 1);
	if (node->qualifiers)
		print_node(node->qualifiers, indent + 1);
}

static void
print_function(struct glsl_node *nodep, size_t indent)
{
	struct glsl_function_node *node;

	node = (struct glsl_function_node*)nodep;
	print_node(node->prototype, indent + 1);
	if (node->content)
		print_node(node->content, indent + 1);
}

static void
print_function_call(struct glsl_node *nodep, size_t indent)
{
	struct glsl_function_call_node *node;

	node = (struct glsl_function_call_node*)nodep;
	print_node(node->identifier, indent + 1);
	if (node->arguments)
		print_node(node->arguments, indent + 1);
}

static void
print_identifier(struct glsl_node *nodep, size_t indent)
{
	struct glsl_identifier_node *node;

	node = (struct glsl_identifier_node*)nodep;
	print_indent(indent + 1);
	printf("%s\n", node->identifier);
}

static void
print_identifier_declaration(struct glsl_node *nodep, size_t indent)
{
	struct glsl_identifier_declaration_node *node;

	node = (struct glsl_identifier_declaration_node*)nodep;
	print_node(node->type_qualifier, indent + 1);
	if (node->identifiers)
		print_node(node->identifiers, indent + 1);
}

static void
print_immediate_bool(struct glsl_node *nodep, size_t indent)
{
	struct glsl_immediate_bool_node *node;

	node = (struct glsl_immediate_bool_node*)nodep;
	print_indent(indent + 1);
	printf("%s\n", node->value ? "TRUE" : "FALSE");
}

static void
print_immediate_double(struct glsl_node *nodep, size_t indent)
{
	struct glsl_immediate_double_node *node;

	node = (struct glsl_immediate_double_node*)nodep;
	print_indent(indent + 1);
	printf("%lf\n", node->value);
}

static void
print_immediate_float(struct glsl_node *nodep, size_t indent)
{
	struct glsl_immediate_float_node *node;

	node = (struct glsl_immediate_float_node*)nodep;
	print_indent(indent + 1);
	printf("%f\n", node->value);
}

static void
print_immediate_int(struct glsl_node *nodep, size_t indent)
{
	struct glsl_immediate_int_node *node;

	node = (struct glsl_immediate_int_node*)nodep;
	print_indent(indent + 1);
	printf("%" PRId32 "\n", node->value);
}

static void
print_immediate_uint(struct glsl_node *nodep, size_t indent)
{
	struct glsl_immediate_uint_node *node;

	node = (struct glsl_immediate_uint_node*)nodep;
	print_indent(indent + 1);
	printf("%" PRIu32 "\n", node->value);
}

static void
print_interpolation_qualifier(struct glsl_node *nodep, size_t indent)
{
	struct glsl_interpolation_qualifier_node *node;

	node = (struct glsl_interpolation_qualifier_node*)nodep;
	print_indent(indent + 1);
	printf("%s\n", interpolation_qualifier_strings[node->qualifier]);
}

static void
print_invariant_qualifier(struct glsl_node *nodep, size_t indent)
{
	(void)nodep;
	(void)indent;
}

static void
print_jump(struct glsl_node *nodep, size_t indent)
{
	struct glsl_jump_node *node;

	node = (struct glsl_jump_node*)nodep;
	print_indent(indent + 1);
	printf("%s\n", jump_type_strings[node->type]);
	if (node->expression)
		print_node(node->expression, indent + 1);
}

static void
print_layout_qualifier(struct glsl_node *nodep, size_t indent)
{
	struct glsl_layout_qualifier_node *node;

	node = (struct glsl_layout_qualifier_node*)nodep;
	print_node(node->identifier, indent + 1);
	if (node->value)
		print_node(node->value, indent + 1);
}

static void
print_layout_qualifiers(struct glsl_node *nodep, size_t indent)
{
	struct glsl_layout_qualifiers_node *node;

	node = (struct glsl_layout_qualifiers_node*)nodep;
	print_node(node->qualifiers, indent + 1);
}

static void
print_list(struct glsl_node *nodep, size_t indent)
{
	struct glsl_list_node *node;
	struct glsl_node *child;

	node = (struct glsl_list_node*)nodep;
	TAILQ_FOREACH(child, &node->nodes, chain)
		print_node(child, indent + 1);
}

static void
print_precise_qualifier(struct glsl_node *nodep, size_t indent)
{
	(void)nodep;
	(void)indent;
}

static void
print_precision_declaration(struct glsl_node *nodep, size_t indent)
{
	struct glsl_precision_declaration_node *node;

	node = (struct glsl_precision_declaration_node*)nodep;
	print_indent(indent + 1);
	printf("%s\n", precision_qualifier_strings[node->precision_qualifier]);
	print_node(node->type_specifier, indent + 1);
}

static void
print_precision_qualifier(struct glsl_node *nodep, size_t indent)
{
	struct glsl_precision_qualifier_node *node;

	node = (struct glsl_precision_qualifier_node*)nodep;
	print_indent(indent + 1);
	printf("%s\n", precision_qualifier_strings[node->qualifier]);
}

static void
print_prototype(struct glsl_node *nodep, size_t indent)
{
	struct glsl_prototype_node *node;

	node = (struct glsl_prototype_node*)nodep;
	print_node(node->return_type, indent + 1);
	print_node(node->identifier, indent + 1);
	if (node->parameters)
		print_node(node->parameters, indent + 1);
}

static void
print_prototype_parameter(struct glsl_node *nodep, size_t indent)
{
	struct glsl_prototype_parameter_node *node;

	node = (struct glsl_prototype_parameter_node*)nodep;
	if (node->identifier)
		print_node(node->identifier, indent + 1);
	if (node->type_qualifiers)
		print_node(node->type_qualifiers, indent + 1);
	if (node->type_specifier)
		print_node(node->type_specifier, indent + 1);
	if (node->array_specifier)
		print_node(node->array_specifier, indent + 1);
}

static void
print_shared_qualifier(struct glsl_node *nodep, size_t indent)
{
	(void)nodep;
	(void)indent;
}

static void
print_storage_qualifier(struct glsl_node *nodep, size_t indent)
{
	struct glsl_storage_qualifier_node *node;

	node = (struct glsl_storage_qualifier_node*)nodep;
	print_indent(indent + 1);
	printf("%s\n", storage_qualifier_strings[node->qualifier]);
}

static void
print_struct_definition(struct glsl_node *nodep, size_t indent)
{
	struct glsl_struct_definition_node *node;

	node = (struct glsl_struct_definition_node*)nodep;
	if (node->identifier)
		print_node(node->identifier, indent + 1);
	if (node->members)
		print_node(node->members, indent + 1);
}

static void
print_struct_member(struct glsl_node *nodep, size_t indent)
{
	struct glsl_struct_member_node *node;

	node = (struct glsl_struct_member_node*)nodep;
	if (node->type_qualifiers)
		print_node(node->type_qualifiers, indent + 1);
	if (node->type_specifier)
		print_node(node->type_specifier, indent + 1);
	if (node->identifiers)
		print_node(node->identifiers, indent + 1);
}

static void
print_struct_member_identifier(struct glsl_node *nodep, size_t indent)
{
	struct glsl_struct_member_identifier_node *node;

	node = (struct glsl_struct_member_identifier_node*)nodep;
	print_node(node->identifier, indent + 1);
	if (node->array_specifier)
		print_node(node->array_specifier, indent + 1);
}

static void
print_subroutine_qualifier(struct glsl_node *nodep, size_t indent)
{
	struct glsl_subroutine_qualifier_node *node;

	node = (struct glsl_subroutine_qualifier_node*)nodep;
	print_node(node->qualifiers, indent + 1);
}

static void
print_switch(struct glsl_node *nodep, size_t indent)
{
	struct glsl_switch_node *node;

	node = (struct glsl_switch_node*)nodep;
	print_node(node->expression, indent + 1);
	print_node(node->statements, indent + 1);
}

static void
print_ternary(struct glsl_node *nodep, size_t indent)
{
	struct glsl_ternary_node *node;

	node = (struct glsl_ternary_node*)nodep;
	print_node(node->cond, indent + 1);
	print_node(node->true_expr, indent + 1);
	print_node(node->false_expr, indent + 1);
}

static void
print_type(struct glsl_node *nodep, size_t indent)
{
	struct glsl_type_node *node;

	node = (struct glsl_type_node*)nodep;
	print_indent(indent + 1);
	printf("%s\n", type_strings[node->type]);
}

static void
print_type_specifier(struct glsl_node *nodep, size_t indent)
{
	struct glsl_type_specifier_node *node;

	node = (struct glsl_type_specifier_node*)nodep;
	print_node(node->type, indent + 1);
	if (node->array_specifier)
		print_node(node->array_specifier, indent + 1);
}

static void
print_unary_op(struct glsl_node *nodep, size_t indent)
{
	struct glsl_unary_op_node *node;

	node = (struct glsl_unary_op_node*)nodep;
	print_indent(indent + 1);
	printf("%s\n", unary_op_strings[node->op]);
	print_node(node->expr, indent + 1);
}

static void
print_while(struct glsl_node *nodep, size_t indent)
{
	struct glsl_while_node *node;

	node = (struct glsl_while_node*)nodep;
	print_indent(indent + 1);
	printf("%s\n", node->do_while ? "DOWHILE" : "WHILE");
	print_node(node->condition, indent + 1);
	print_node(node->statement, indent + 1);
}

static void
print_node(struct glsl_node *node, size_t indent)
{
	print_indent(indent);
	printf("%s\n", node_type_strings[node->type]);
	print_indent(indent);
	printf("{\n");
	switch (node->type)
	{
		case GLSL_NODE_ARRAY_ACCESSOR:
			print_array_accessor(node, indent);
			break;
		case GLSL_NODE_ASSIGNMENT:
			print_assignment(node, indent);
			break;
		case GLSL_NODE_BINARY_OP:
			print_binary_op(node, indent);
			break;
		case GLSL_NODE_BLOCK_DECLARATION:
			print_block_declaration(node, indent);
			break;
		case GLSL_NODE_CASE:
			print_case(node, indent);
			break;
		case GLSL_NODE_COMPARE_OP:
			print_compare_op(node, indent);
			break;
		case GLSL_NODE_CONDITION:
			print_condition(node, indent);
			break;
		case GLSL_NODE_DECLARATION:
			print_declaration(node, indent);
			break;
		case GLSL_NODE_DECLARATION_ENTRY:
			print_declaration_entry(node, indent);
			break;
		case GLSL_NODE_FIELD_ACCESSOR:
			print_field_accessor(node, indent);
			break;
		case GLSL_NODE_FOR:
			print_for(node, indent);
			break;
		case GLSL_NODE_FULLY_SPECIFIED_TYPE:
			print_fully_specified_type(node, indent);
			break;
		case GLSL_NODE_FUNCTION:
			print_function(node, indent);
			break;
		case GLSL_NODE_FUNCTION_CALL:
			print_function_call(node, indent);
			break;
		case GLSL_NODE_IDENTIFIER:
			print_identifier(node, indent);
			break;
		case GLSL_NODE_IDENTIFIER_DECLARATION:
			print_identifier_declaration(node, indent);
			break;
		case GLSL_NODE_IMMEDIATE_BOOL:
			print_immediate_bool(node, indent);
			break;
		case GLSL_NODE_IMMEDIATE_DOUBLE:
			print_immediate_double(node, indent);
			break;
		case GLSL_NODE_IMMEDIATE_FLOAT:
			print_immediate_float(node, indent);
			break;
		case GLSL_NODE_IMMEDIATE_INT:
			print_immediate_int(node, indent);
			break;
		case GLSL_NODE_IMMEDIATE_UINT:
			print_immediate_uint(node, indent);
			break;
		case GLSL_NODE_INTERPOLATION_QUALIFIER:
			print_interpolation_qualifier(node, indent);
			break;
		case GLSL_NODE_INVARIANT_QUALIFIER:
			print_invariant_qualifier(node, indent);
			break;
		case GLSL_NODE_JUMP:
			print_jump(node, indent);
			break;
		case GLSL_NODE_LAYOUT_QUALIFIER:
			print_layout_qualifier(node, indent);
			break;
		case GLSL_NODE_LAYOUT_QUALIFIERS:
			print_layout_qualifiers(node, indent);
			break;
		case GLSL_NODE_LIST:
			print_list(node, indent);
			break;
		case GLSL_NODE_PRECISE_QUALIFIER:
			print_precise_qualifier(node, indent);
			break;
		case GLSL_NODE_PRECISION_DECLARATION:
			print_precision_declaration(node, indent);
			break;
		case GLSL_NODE_PRECISION_QUALIFIER:
			print_precision_qualifier(node, indent);
			break;
		case GLSL_NODE_PROTOTYPE:
			print_prototype(node, indent);
			break;
		case GLSL_NODE_PROTOTYPE_PARAMETER:
			print_prototype_parameter(node, indent);
			break;
		case GLSL_NODE_SHARED_QUALIFIER:
			print_shared_qualifier(node, indent);
			break;
		case GLSL_NODE_STORAGE_QUALIFIER:
			print_storage_qualifier(node, indent);
			break;
		case GLSL_NODE_STRUCT_DEFINITION:
			print_struct_definition(node, indent);
			break;
		case GLSL_NODE_STRUCT_MEMBER:
			print_struct_member(node, indent);
			break;
		case GLSL_NODE_STRUCT_MEMBER_IDENTIFIER:
			print_struct_member_identifier(node, indent);
			break;
		case GLSL_NODE_SUBROUTINE_QUALIFIER:
			print_subroutine_qualifier(node, indent);
			break;
		case GLSL_NODE_SWITCH:
			print_switch(node, indent);
			break;
		case GLSL_NODE_TERNARY:
			print_ternary(node, indent);
			break;
		case GLSL_NODE_TYPE:
			print_type(node, indent);
			break;
		case GLSL_NODE_TYPE_SPECIFIER:
			print_type_specifier(node, indent);
			break;
		case GLSL_NODE_UNARY_OP:
			print_unary_op(node, indent);
			break;
		case GLSL_NODE_WHILE:
			print_while(node, indent);
			break;
	}
	print_indent(indent);
	printf("}\n");
}

void
glsl_print(struct glsl_node *node)
{
	print_node(node, 0);
}
