#include "code_gen.h"
#include "parser.h"

/** A counter to prevent the issuance of duplicate labels. */
unsigned label_count = 0;
/** True iff the data segment has already been partially printed. */
int data_seg_opened = 0;
/** True iff the text segment has already been partially printed. */
int text_seg_opened = 0;

void emit_strings() {
    /* TODO: Implement me. */
    smap_print_strings(strings);
}

void emit_static_memory() {
    /* TODO: Implement me. */
    smap_print_static_vars(all_decls);
    smap_print_static_structs(structs);
}

void emit_main(AST *ast) {
    /* TODO: Implement me. */
    node_type type = ast->type;
    AST_lst *child = ast->children;
	
    switch(type) {
    case node_INT:
	break;
    case node_STRING: ;
	/*
	int num = smap_get(strings, ast->val);
	printf("lw $t0, string%d\n", num);
	break;
	*/
    case node_ASSIGN:	
	if(ast->children->next->val->type == node_INT) {
	    printf("li $t0 %s\n", ast->children->next->val->val);
	    printf("move $v0 $t0\n");
	}
	break;
    case node_VAR:
	break;
    case node_PLUS:
	
	if(ast->children->val->type == node_INT) {
	    printf("li $t0 %s\n", ast->children->val->val);
	} else {
	    printf("la $t0 %s\n", ast->children->val->val);
	}

	if(ast->children->next->val->type == node_INT) {
	     printf("li $t1 %s\n", ast->children->val->val);
	} else {
	     printf("la $t1 %s\n", ast->children->val->val);
	}
	printf("$addiu $v0, $t0, $t1");
	
	break;
    case node_CALL: ;
	//	int stack_size = 4 * smap_get(stack_sizes, ast->val);
	printf("sw $ra, 0($sp)\n");
	printf("subu $sp, $sp, 4\n");
	printf("sw $fp, 0($sp)\n");
	printf("subu $sp, $sp, 4\n");
	printf("addu $fp $sp, %d\n", 8);
	break;
    case node_STRUCT:
	
	break;
    case node_IF: 

	break;
    case node_WHILE: 
	
	break;
    case node_FOR: 

	break;
    case node_S_PRINT:
	
	 break;
    default:
	break;
    }
    
    while(child) {
	emit_main(child->val);
	child = child->next;
    }
}

void emit_exit() {
    printf("    li $v0 10\n");
    printf("    syscall\n");
}

void emit_functions(AST *ast) {
    /* TODO: Implement me. */
}

int main(int argc, char *argv[]) {
    printf("BEGIN\n");
    lexer luthor;
    /* Initialize lexer. */
    init_lex(&luthor);
    open_file(&luthor, "lexer_test3.txt");

    /* Run parser. */
    parse_init();
    
    AST *ast1 = build_ast(&luthor);
    check_tree_shape(ast1);
    gather_decls(ast1, "", 1);

    
    do {
	read_token(&luthor);
	if(peek_type(&luthor) != token_END) {
	    AST *ast = build_ast(&luthor);
	    check_tree_shape(ast);
	    gather_decls(ast, "", 1);
	}
    } while (peek_type(&luthor) != token_END);

    emit_strings();
    emit_static_memory();
    emit_main(ast1);
    //printf("%d\n", smap_get(strings, "bee"));
    return 0;
}
