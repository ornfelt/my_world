#include "ast.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void *emit_node(enum node_type type, size_t size)
{
	fprintf(stderr, "\033[1mcreate node type %d\033[0m\n", type);
	struct node *node = calloc(size, 1);
	if (!node)
		return NULL;
	node->type = type;
	return node;
}

struct node *emit_unary_op_node(enum unary_op op, struct node *expr)
{
	struct unary_op_node *node = emit_node(NODE_UNARY_OP, sizeof(*node));
	if (!node)
		return NULL;
	node->op = op;
	node->expr = expr;
	return &node->node;
}

struct node *emit_binary_op_node(enum binary_op op, struct node *left, struct node *right)
{
	struct binary_op_node *node = emit_node(NODE_BINARY_OP, sizeof(*node));
	if (!node)
		return NULL;
	node->op = op;
	node->left = left;
	node->right = right;
	return &node->node;
}

struct node *emit_compare_op_node(enum compare_op op, struct node *left, struct node *right)
{
	struct compare_op_node *node = emit_node(NODE_COMPARE_OP, sizeof(*node));
	if (!node)
		return NULL;
	node->op = op;
	node->left = left;
	node->right = right;
	return &node->node;
}

struct node *emit_assignment_node(enum assignment_op op, struct node *dst, struct node *expr)
{
	struct assignment_node *node = emit_node(NODE_ASSIGNMENT, sizeof(*node));
	if (!node)
		return NULL;
	node->op = op;
	node->dst = dst;
	node->expr = expr;
	return &node->node;
}

struct node *emit_ternary_node(struct node *cond, struct node *true_expr, struct node *false_expr)
{
	struct ternary_node *node = emit_node(NODE_TERNARY, sizeof(*node));
	if (!node)
		return NULL;
	node->cond = cond;
	node->true_expr = true_expr;
	node->false_expr = false_expr;
	return &node->node;
}

struct node *emit_struct_member_identifier_node(const char *identifier)
{
	struct struct_member_identifier_node *node = emit_node(NODE_STRUCT_MEMBER_IDENTIFIER, sizeof(*node));
	if (!node)
		return NULL;
	if (identifier)
	{
		node->identifier = strdup(identifier);
		if (!node->identifier)
		{
			free(node);
			return NULL;
		}
	}
	else
	{
		node->identifier = NULL;
	}
	return &node->node;
}

void push_struct_member_identifier(struct node *node, struct node *add)
{
	struct struct_member_identifier_node *member = (struct struct_member_identifier_node*)node;
	while (member->next)
		member = member->next;
	member->next = (struct struct_member_identifier_node*)add;
}

struct node *emit_struct_member_node(struct node *type_qualifiers, struct node *type_specifier, struct node *identifiers)
{
	struct struct_member_node *node = emit_node(NODE_STRUCT_MEMBER, sizeof(*node));
	if (!node)
		return NULL;
	node->type_qualifiers = type_qualifiers;
	node->type_specifier = type_specifier;
	node->identifiers = identifiers;
	return &node->node;
}

void push_struct_member(struct node *node, struct node *add)
{
	struct struct_member_node *member = (struct struct_member_node*)node;
	while (member->next)
		member = member->next;
	member->next = (struct struct_member_node*)add;
}

struct node *emit_struct_definition_node(const char *identifier, struct node *members)
{
	struct struct_definition_node *node = emit_node(NODE_STRUCT_DEFINITION, sizeof(*node));
	if (!node)
		return NULL;
	if (identifier)
	{
		node->identifier = strdup(identifier);
		if (!node->identifier)
		{
			free(node);
			return NULL;
		}
	}
	else
	{
		node->identifier = NULL;
	}
	return &node->node;
}

struct node *emit_function_node(struct node *prototype, struct node *content)
{
	struct function_node *node = emit_node(NODE_FUNCTION, sizeof(*node));
	if (!node)
		return NULL;
	node->prototype = prototype;
	node->content = content;
	return &node->node;
}

struct node *emit_layout_qualifier_node(const char *identifier, struct node *value)
{
	struct layout_qualifier_node *node = emit_node(NODE_LAYOUT_QUALIFIER, sizeof(*node));
	if (!node)
		return NULL;
	node->identifier = strdup(identifier);
	if (!node->identifier)
	{
		free(node);
		return NULL;
	}
	node->value = value;
	return &node->node;
}

void push_layout_qualifier(struct node *node, struct node *add)
{
	struct layout_qualifier_node *qualifiers = (struct layout_qualifier_node*)node;
	while (qualifiers->next)
		qualifiers = qualifiers->next;
	qualifiers->next = (struct layout_qualifier_node*)add;
}

struct type_qualifier_node *emit_type_qualifier_node(enum qualifier_type type)
{
	struct type_qualifier_node *node = emit_node(NODE_TYPE_QUALIFIER, sizeof(*node));
	if (!node)
		return NULL;
	node->type = type;
	node->next = NULL;
	return node;
}

struct node *emit_storage_qualifier_node(enum storage_qualifier qualifier)
{
	struct type_qualifier_node *node = emit_type_qualifier_node(QUALIFIER_STORAGE);
	if (!node)
		return NULL;
	node->storage_qualifier = qualifier;
	return &node->node;
}

struct node *emit_layout_qualifier_specifier_node(struct node *qualifier)
{
	struct type_qualifier_node *node = emit_type_qualifier_node(QUALIFIER_LAYOUT);
	if (!node)
		return NULL;
	node->layout_qualifier = qualifier;
	return &node->node;
}

struct node *emit_precision_qualifier_node(enum precision_qualifier qualifier)
{
	struct type_qualifier_node *node = emit_type_qualifier_node(QUALIFIER_PRECISION);
	if (!node)
		return NULL;
	node->precision_qualifier = qualifier;
	return &node->node;
}

struct node *emit_interpolation_qualifier_node(enum interpolation_qualifier qualifier)
{
	struct type_qualifier_node *node = emit_type_qualifier_node(QUALIFIER_INTERPOLATION);
	if (!node)
		return NULL;
	node->interpolation_qualifier = qualifier;
	return &node->node;
}

struct node *emit_invariant_qualifier_node(enum invariant_qualifier qualifier)
{
	struct type_qualifier_node *node = emit_type_qualifier_node(QUALIFIER_INVARIANT);
	if (!node)
		return NULL;
	node->invariant_qualifier = qualifier;
	return &node->node;
}

struct node *emit_precise_qualifier_node(enum precise_qualifier qualifier)
{
	struct type_qualifier_node *node = emit_type_qualifier_node(QUALIFIER_PRECISE);
	if (!node)
		return NULL;
	node->precise_qualifier = qualifier;
	return &node->node;
}

void push_type_qualifier(struct node *node, struct node *add)
{
	struct type_qualifier_node *qualifiers = (struct type_qualifier_node*)node;
	while (qualifiers->next)
		qualifiers = qualifiers->next;
	qualifiers->next = (struct type_qualifier_node*)add;
}

struct node *emit_fully_specified_type_node(struct node *type_specifier, struct node *qualifiers)
{
	struct fully_specified_type_node *node = emit_node(NODE_FULLY_SPECIFIED_TYPE, sizeof(*node));
	if (!node)
		return NULL;
	node->type_specifier = type_specifier;
	node->qualifiers = qualifiers;
	return &node->node;
}

struct node *emit_prototype_node(struct node *return_type, const char *identifier)
{
	struct prototype_node *node = emit_node(NODE_PROTOTYPE, sizeof(*node));
	if (!node)
		return NULL;
	node->return_type = return_type;
	node->parameters = NULL;
	if (identifier)
	{
		node->identifier = strdup(identifier);
		if (!node->identifier)
		{
			free(node);
			return NULL;
		}
	}
	else
	{
		node->identifier = NULL;
	}
	return &node->node;
}

void push_prototype_parameter(struct node *node, struct node *parameter)
{
	struct prototype_node *prototype = (struct prototype_node*)node;
	if (!prototype->parameters)
	{
		prototype->parameters = (struct prototype_parameter_node*)parameter;
		return;
	}
	struct prototype_parameter_node *parameters = prototype->parameters;
	while (parameters->next)
		parameters = parameters->next;
	parameters->next = (struct prototype_parameter_node*)parameter;
}

struct node *emit_prototype_parameter_node(struct node *type_qualifiers, struct node *type_specifier, const char *identifier)
{
	struct prototype_parameter_node *node = emit_node(NODE_PROTOTYPE_PARAMETER, sizeof(*node));
	if (!node)
		return NULL;
	node->type_qualifiers = type_qualifiers;
	node->type_specifier = type_specifier;
	if (identifier)
	{
		node->identifier = strdup(identifier);
		if (!node->identifier)
		{
			free(node);
			return NULL;
		}
	}
	else
	{
		node->identifier = NULL;
	}
	return &node->node;
}

struct node *emit_function_call_argument(struct node *expr)
{
	struct function_call_argument_node *node = emit_node(NODE_FUNCTION_CALL_ARGUMENT, sizeof(*node));
	if (!node)
		return NULL;
	node->expr = expr;
	return &node->node;
}

void push_function_call_argument(struct node *node, struct node *add)
{
	struct function_call_node *call = (struct function_call_node*)node;
	if (!call->arguments)
	{
		call->arguments = add;
		return;
	}
	struct function_call_argument_node *argument = (struct function_call_argument_node*)call->arguments;
	while (argument->next)
		argument = argument->next;
	argument->next = (struct function_call_argument_node*)add;
}

struct node *emit_function_call_node(struct node *identifier, struct node *arguments)
{
	struct function_call_node *node = emit_node(NODE_FUNCTION_CALL, sizeof(*node));
	if (!node)
		return NULL;
	node->identifier = identifier;
	node->arguments = arguments;
	return &node->node;
}

struct node *emit_type_specifier_native_node(enum type_specifier type_specifier)
{
	struct type_specifier_node *node = emit_node(NODE_TYPE_SPECIFIER, sizeof(*node));
	if (!node)
		return NULL;
	node->type = TYPE_SPECIFIER_NATIVE;
	node->native = type_specifier;
	return &node->node;
}

struct node *emit_type_specifier_struct_node(struct node *st)
{
	struct type_specifier_node *node = emit_node(NODE_TYPE_SPECIFIER, sizeof(*node));
	if (!node)
		return NULL;
	node->type = TYPE_SPECIFIER_STRUCT;
	node->st = st;
	return &node->node;
}

struct node *emit_type_specifier_named_node(const char *identifier)
{
	fprintf(stderr, "\033[1;35mnamed specifier %s\033[0m\n", identifier);
	struct type_specifier_node *node = emit_node(NODE_TYPE_SPECIFIER, sizeof(*node));
	if (!node)
		return NULL;
	node->type = TYPE_SPECIFIER_NAMED;
	node->identifier = identifier;
	return &node->node;
}
