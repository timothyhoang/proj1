#include "parser.h"
#include "lexer.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/** An array of the different string values of keywords. */
char *keywords[] = {"and", "or", "+", "-", "*", "/", "lt", "eq", 
		    "function", "struct", "arrow", "assign", "if", 
		    "while", "for", "sequence", "intprint", "stringprint",
		    "readint"};
/** Sister array of keywords. Keeps track of the corresponding enum. */
int enums[] = {node_AND, node_OR, node_PLUS, node_MINUS, node_MUL, node_DIV,
	       node_LT, node_EQ, node_FUNCTION, node_STRUCT, 
	       node_ARROW, node_ASSIGN, node_IF, node_WHILE, node_FOR, 
	       node_SEQ, node_I_PRINT, node_S_PRINT, node_READ_INT};

int enum_scope[] = {global_var, global_func, local_var};

/** Array of number of arguments for each keyword */
int keyword_num_args[] = {2, 2, 2, 2, 2, 2, 2, 2,
			  1, 1, 2, 2, 3,
			  3, 4, 1, 1, 1,
			  0};

/** A hashmap used for more efficient lookups of (keyword, enum) pairs. */
smap *keyword_str_to_enum;

/** Initializes keyword_str_to_enum so that it contains a map
 *  from the string value of a keyword to its corresponding enum. */
void initialize_keyword_to_enum_mapping();
void initialize_keyword_to_num_args_mapping();

void parse_init() {
    decls = smap_new();
    structs = smap_new();
    all_decls = smap_new();
    local_decls = smap_new();
    stack_sizes = smap_new();
    num_args = smap_new();
    strings = smap_new();
    keyword_str_to_enum = smap_new();
}

void parse_close() {
    smap_del_contents(decls);
    smap_del(decls);
    smap_del(all_decls);
    smap_del(local_decls);
    smap_del(structs);
    smap_del(stack_sizes);
    smap_del(num_args);
    smap_del(strings);
    smap_del(keyword_str_to_enum);
}

void add_child_node(AST *parent_node, AST *child_node) {    
    //printf("FUNCTION CHECK 1: current_node->val: %s\n", parent_node->val);
    if(parent_node->last_child != NULL) {
	//printf("CHECK 1.1\n");
	AST_lst *child_lst = (AST_lst *) safe_malloc(sizeof(AST_lst));
	//printf("CHECK 1.2\n");
	child_lst->val = child_node;
	child_lst->next = NULL;
	//printf("CHECK 1.3\n");
	parent_node->last_child->next = child_lst;
	//printf("CHECK 1.4\n");
	parent_node->last_child = child_lst;
	//printf("CHECK 1.5\n");
	//printf("added child to child list\n");
    } else {
	//printf("CHECK 2.1\n");
	AST_lst *child_lst = (AST_lst *) safe_malloc(sizeof(AST_lst));
       	child_lst->val = child_node;
	child_lst->next = NULL;
	parent_node->children = child_lst;
	parent_node->last_child = child_lst;
	//printf("created child list\n");
    }
    //printf("FUNCTION CHECK 2: current_node->val: %s\n", parent_node->val);
}

AST *gen_AST(AST *current_node, lexer *lex) {
    //printf("BEGIN\n");
    AST *new_node;
    read_token(lex);
    token_type type = peek_type(lex);
    char* value = peek_value(lex);
    
    if(type == token_END) {
	    fprintf(stderr, "ERROR: Missing ')'");
	    exit(1);
    }
    
    while(type != token_END) {
	switch(type) {
	case token_INT: ;
	    //printf("11111\n");
	    new_node = (AST *) safe_malloc(sizeof(AST));
	    new_node->type = node_INT;
	    new_node->val = safe_malloc(strlen(value) + 1);
	    strcpy(new_node->val, value);
	    new_node->children = NULL;
	    new_node->last_child = NULL;
	    add_child_node(current_node, new_node);
	    break;
	case token_STRING: ;
	    //printf("22222\n");
	    new_node = (AST *) safe_malloc(sizeof(AST));
	    new_node->type = node_STRING;
	    new_node->val = safe_malloc(strlen(value) + 1);
	    strcpy(new_node->val, value);
	    new_node->children = NULL;
	    new_node->last_child = NULL;
	    add_child_node(current_node, new_node);
	    break;
	case token_NAME: ;
	    //printf("33333: %s\n", value);
	    if(lookup_keyword_enum(value) != -1) {
		fprintf(stderr, "ERROR: variable names cannot match keywords\n");
		exit(1);
	    }		
	    new_node = (AST *) safe_malloc(sizeof(AST));
	    new_node->type = node_VAR;
	    new_node->val = safe_malloc(strlen(value) + 1);
	    strcpy(new_node->val, value);
	    new_node->children = NULL;
	    new_node->last_child = NULL;
	    add_child_node(current_node, new_node);
	    break;
	case token_KEYWORD:
	    //printf("44444\n"); 
	    if(lookup_keyword_enum(value) == -1) {
		if(isdigit(value[0])) {
		    fprintf(stderr, "ERROR: Function names cannot begin with a numeric\n");
		    exit(1);
		}
		int i = 0;
		while(value[i] != '\0') {
		    if(!isdigit(value[i]) && !isalpha(value[i]) && value[i] != '_') {
			fprintf(stderr, "ERROR: Improper function name\n");
			exit(1);
		    }
		    i++;
		}
		
		current_node->type = node_CALL;
		current_node->val = safe_malloc(strlen(value) + 1);
		strcpy(current_node->val, value);
	    } else {
		current_node->type = lookup_keyword_enum(value);
		current_node->val = safe_malloc(strlen(value) + 1);
		strcpy(current_node->val, value);
	    }
	    break;
	case token_OPEN_PAREN:
	    //printf("55555\n");
	    new_node = (AST *) safe_malloc(sizeof(AST));
	    new_node->children = NULL;
	    new_node->last_child = NULL;
	    new_node = gen_AST(new_node, lex);
	    add_child_node(current_node, new_node);
	    break;
	case token_CLOSE_PAREN:
	    //printf("66666\n");
	    if(current_node->val) {
		return current_node;
	    } else {
		fprintf(stderr, "ERROR: empty expression");
		exit(1);
	    }
	    break;
	case token_END:
	    //printf("77777\n");
	    break;
	case token_SENTINEL:
	    //printf("88888\n");
	    fprintf(stderr, "ERROR");
	    exit(1);
	    break;
	}
	read_token(lex);
	type = peek_type(lex);
	value = peek_value(lex);
    }
    
    if(current_node->val) {
	return current_node;
    } else {
	fprintf(stderr, "ERROR: empty expression");
	exit(1);
    }
}

AST *build_ast(lexer *lex) {
    /* TODO: Implement me. */
    /* Hint: switch statements are pretty cool, and they work 
     *       brilliantly with enums. */
    if(lex) {
	AST *root;
	//read_token(lex);
	if(peek_type(lex) != token_OPEN_PAREN) {
	    fprintf(stderr, "ERROR: Encountered atom outside of top level parenthetical\n");
	    exit(1);
	} else {
	    root = (AST *) safe_malloc(sizeof(AST));
	    root->children = NULL;
	    root->last_child = NULL;
	}
	return gen_AST(root, lex);
    } else {
	fprintf(stderr, "ERROR: NULL pointer as input\n");
	exit(1);
    }
}

void free_ast (AST *ptr) {
    /* TODO: Implement me. */
    //printf("CALLED\n");
    if(ptr) {
	//printf("%s\n", ptr->val);
	if(ptr->children) {
	    AST_lst *lst1 = ptr->children;
	    AST_lst *lst2 = lst1;
	    while(lst1) {
		lst2 = lst1->next;
		if(lst1->val)
		    free_ast(lst1->val);;
		lst1 = lst2;
	    }
	    if(ptr->val)
		free(ptr->val);
	    free(ptr);
	}
    } else {
	fprintf(stderr, "ERROR: NULL pointer as input\n");
	exit(1);
    }
}


int num_children(AST *ptr) {
    if(ptr) {
	AST_lst *child = ptr->children;
	int i = 0;
	while(child) {
	    i++;
	    child = child->next;
	}
	return i;
    } else {
	fprintf(stderr, "ERROR: NULL pointer as input\n");
	exit(1);
    }
}


void check_tree_shape(AST *ptr) {
    /* TODO: Implement me. */
    /* Hint: This function is just asking to be table-driven */
    
    // Check that keywords have the correct number of inputs and inputs are correct node_type
    if(ptr->type == node_STRUCT || ptr->type == node_SEQ || ptr->type == node_VAR) {
	if(num_children(ptr) < lookup_keyword_num_args(ptr->val)) {
	    fprintf(stderr, "ERROR: need at least one argument for keyword '%s'\n", ptr->val);
	    exit(1);
	}
    } else if(ptr->type == node_INT || ptr->type == node_STRING) {
	if(num_children(ptr) != 0) {
	    fprintf(stderr, "ERROR: non-function variables cannot take in arguments\n");
	    exit(1);
	}
    } else if(ptr->type == node_ASSIGN) {
	if(ptr->children[0].val->type != node_VAR) {
	    fprintf(stderr, "ERROR: improper assignment to non-variable object\n");
	    exit(1);
	}
    } else if(ptr->type == node_FUNCTION) {	
	if(num_children(ptr) != 1 && num_children(ptr) != 2) {
	    fprintf(stderr, "ERROR: incorrect number of arguments for keyword '%s'\n", ptr->val);
	    exit(1);
	}
	if(lookup_keyword_enum(ptr->children[0].val->val) != -1) {
	    fprintf(stderr, "ERROR: function name cannot match keyword\n");
	    exit(1);
	}
	if(ptr->children->val->type != node_CALL) {
	    fprintf(stderr, "ERROR: improper assignment to non-variable object\n");
	    exit(1);
	}
	
	if(ptr->children->val->children) {
	    AST_lst *args = ptr->children->val->children;
	    while(args) {
		if(args->val->type != node_VAR) {
		    fprintf(stderr, "ERROR: improper function declaration\n");
		    exit(1);
		}
		args = args->next;
	    }
	}
	
	ptr->children->val->type = node_VAR;
    } else if((num_children(ptr) != lookup_keyword_num_args(ptr->val)) && ptr->type != node_CALL) {
	fprintf(stderr, "ERROR: incorrect number of arguments for keyword '%s'\n", ptr->val);
	exit(1);
    }

    if(ptr->children != NULL) {
	AST_lst *child = ptr->children;
	while(child) {
	    check_tree_shape(child->val);
	    child = child->next;
	}
    }
}

void gather_decls(AST *ast, char *env, int is_top_level) {
    /* TODO: Implement me. */
    /* Hint: switch statements are pretty cool, and they work 
     *       brilliantly with enums. */
    node_type type = ast->type;
    AST_lst *child = ast->children;
	
    switch(type) {
    case node_STRING:
	smap_put(strings, ast->val, global_var);
	break;
    case node_ASSIGN:	
	child = child->next;
	while(child) {
	    gather_decls(child->val, "", 0);
	    child = child->next;
	}
	
	if(strcmp(env, "") == 0) {
	    smap_put(all_decls, ast->children[0].val->val, global_var);
	    smap_put(decls, ast->children[0].val->val, global_var);
	    if(ast->children->next->val->type == node_STRUCT) {
		smap_put(structs, ast->children[0].val->val, num_children(ast->children->next->val));
	    }
	} else if(strcmp(env, "cond") == 0) {
	    fprintf(stderr, "ERROR: cannot have declaration inside of conditional");
	    exit(1);
	} else {
	    smap_put(local_decls, ast->children[0].val->val, local_var);
	}
	
	break;
    case node_VAR:
	if(smap_get(decls, ast->val) == -1 && smap_get(local_decls, ast->val) == -1) {
	    fprintf(stderr, "ERROR: attempting to use undeclared/uninitialized variable");
	    exit(1);
	}
	break;
    case node_FUNCTION:
	local_decls = smap_new();
	
	if(!is_top_level) {
	    fprintf(stderr, "ERROR: function call not at top level");
	    exit(1);
	}

	if(smap_get(decls, child->val->val) == global_func) {
	    fprintf(stderr, "ERROR: cannot declare a function with the same name as another function");
	    exit(1);
	} else {
	    smap_put(decls, child->val->val, global_func);
	}

	smap_put(stack_sizes, ast->children->val->val, num_children(child->val));

	AST_lst *args = child->val->children;
	while(args) {
	    smap_put(all_decls, ast->children[0].val->val, global_var);
	    smap_put(local_decls, args->val->val, local_var);
	    args = args->next;
	}
	
	child = child->next;
	while(child) {
	    gather_decls(child->val, ast->children->val->val, 0);
	    child = child->next;
	}
	
	smap_del_contents(local_decls);
	
	break;
    case node_CALL:
	if(smap_get(decls, ast->val) != global_func) {
	    fprintf(stderr, "ERROR: attempting to use undeclared function");
	    exit(1);
	}
	break;
    case node_STRUCT:
	if(strcmp(env, "") != 0) {
	    if(strcmp(env, "cond") != 0) {
		int num_childs = num_children(ast);
		int tmp = num_childs + smap_get(stack_sizes, env);	
		smap_put(stack_sizes, env, tmp);
	    }
	}
	break;
    case node_IF: 
	while(child) {
	    gather_decls(child->val, "cond", 0);
	    child = child->next;
	}
	break;
    case node_WHILE: 
	while(child) {
	    gather_decls(child->val, "cond", 0);
	    child = child->next;
	}
	break;
    case node_FOR: 
	while(child) {
	    gather_decls(child->val, "cond", 0);
	    child = child->next;
	}
	break;
    case node_S_PRINT:
	 smap_put(strings, child->val->val, global_var);
	 break;
    default:
	break;
    }
    
    while(child) {
	gather_decls(child->val, "", 0);
	child = child->next;
    }
}

node_type lookup_keyword_enum(char *str) {
    if (smap_get(keyword_str_to_enum, keywords[0]) == -1) {
	initialize_keyword_to_enum_mapping();
    }
    return smap_get(keyword_str_to_enum, str);
}

int lookup_keyword_num_args(char *str) {
    if (smap_get(num_args, keywords[0]) == -1) {
	initialize_keyword_to_num_args_mapping();
    }
    return smap_get(num_args, str);
}

void initialize_keyword_to_enum_mapping() {
    /* Note that enums is an *array*, not a pointer, so this
     * sizeof business is reasonable. */
    size_t num_keywords = sizeof(enums) / sizeof(int);
    for (size_t i = 0; i < num_keywords; i += 1) {
	smap_put(keyword_str_to_enum, keywords[i], enums[i]);
    }
}

void initialize_keyword_to_num_args_mapping() {
    size_t num_keywords = sizeof(keyword_num_args) / sizeof(int);
    for(size_t i = 0; i < num_keywords; i +=1) {
	smap_put(num_args, keywords[i], keyword_num_args[i]);
    }
}

size_t AST_lst_len(AST_lst *lst) {
    int num_fields = 0;
    while (lst) {
	num_fields += 1;
	lst = lst->next;
    }
    return num_fields;
}

smap *decls;
smap *all_decls;
smap *local_decls;
smap *structs;
smap *stack_sizes;
smap *num_args;
smap *strings;
