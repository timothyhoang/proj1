#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "lexer.h"
#include "util/util.h"
#include "parser.h"

#define INIT_BUFFER_SIZE 256

void init_lex(lexer *luthor) {
    luthor->file = NULL;
    luthor->buffer = NULL;
    luthor->type = token_SENTINEL;
    luthor->buff_len = 0;
    //printf("initializing lexer\n");
}

void open_file(lexer *lex, char *filename) {
    if (lex) {
	lex->file = fopen(filename, "r");
	if (!lex->file) {
	    fatal_error("Could not read input file.\n");
	}
	lex->buff_len = INIT_BUFFER_SIZE;
	lex->buffer = safe_calloc(INIT_BUFFER_SIZE * sizeof(char));
    }
    //printf("opening file: %s\n", filename);
}

void close_file(lexer *lex) {
    if (lex) {
	fclose(lex->file);
	free(lex->buffer);
	lex->buff_len = 0;
	lex->buffer = NULL;
    }
}

void read_token(lexer *lex) {
    /* TODO: Implement me. */
    /* HINT: fgetc() and ungetc() could be pretty useful here. */
    if(lex) {
	if(lex->file) {
	    if(!lex->buffer) 
		lex->buffer = (char*) safe_malloc(2 * sizeof(char));

	    char new_char = (char) fgetc(lex->file);
	    // Check if whitespace precedes next segment
	    if(isspace(new_char)) {
		while(isspace(new_char)) {
		    new_char = fgetc(lex->file);
		    //printf("A: SPACE\n");
		}
	    }
	    
	    lex->buffer[0] = new_char;
	    lex->buff_len = 2;
	    //printf("%c\n", new_char);
	    if(new_char == '(') {
		lex->type = token_OPEN_PAREN;
		lex->buffer[1] = '\0';
	    } else if(new_char == ')') {
		lex->type = token_CLOSE_PAREN;
		lex->buffer[1] = '\0';
	    } else if((int)new_char == EOF) {
		lex->type = token_END;
	       	lex->buffer[1] = '\0';
	    } else {
		// Get next segment of input
		int i = 1;
		while(!isspace(new_char = (char) fgetc(lex->file)) && (new_char != ')') && (new_char != '(') && (new_char != EOF)) {
		    if(i + 1 > lex->buff_len) {
			lex->buffer = safe_realloc(lex->buffer, (i + 1)*sizeof(char));
			lex->buffer[i] = new_char;
			lex->buff_len++;
			i++;
		    } else {
			lex->buffer[i] = new_char;
			i++;
		    }
		}

		// Null terminate buffer
		lex->buffer[i] = '\0';

		// Move to start of next segment
		if(new_char == '(') 
		    ungetc(new_char, lex->file);
		else if(new_char == ')')
		    ungetc(new_char, lex->file);
		else if(new_char == EOF)
		    ungetc(new_char, lex->file);
		else if(isspace(new_char)) {
		    while(isspace(new_char)) {
			new_char = fgetc(lex->file);
			//printf("B: SPACE\n");
		    }
		    ungetc(new_char, lex->file);
		}
		
		// Determine if string in buffer is an integer, string, keyword, or variable name (function names count as keywords here)
		if(lex->type == token_OPEN_PAREN) {
		    lex->type = token_KEYWORD;
		    if(isdigit(lex->buffer[0])) {
			fprintf(stderr, "ERROR: Function names cannot begin with a numeric");
			exit(1);
		    }
		} else {
		    if(isdigit(lex->buffer[0])) {
			i = 0;
			while(lex->buffer[i] != '\0') {
			    if(!isdigit(lex->buffer[i])) {
				fprintf(stderr, "ERROR: Variable names cannot begin with a numeric");
				exit(1);
			    }
			    i++;
			}
			lex->type = token_INT;
		    } else if(lex->buffer[0] == '"' && lex->buffer[i-1] == '"') {
			lex->type = token_STRING;
			lex->buffer[i-1] = '\0';
			strcpy(lex->buffer, &lex->buffer[1]);
		    } else if(strcmp(lex->buffer, "None") == 0) {
			lex->type = token_INT;
			lex->buffer[0] = '0';
			lex->buffer[1] = '\0';
		    } else {
			lex->type = token_NAME;
			if(isdigit(lex->buffer[0])) {
			    fprintf(stderr, "ERROR: Variable names cannot begin with a numeric");
			    exit(1);
			}
			i = 0;
			while(lex->buffer[i] != '\0') {
			    if(!isdigit(lex->buffer[i]) && !isalpha(lex->buffer[i]) && lex->buffer[i] != '_') {
				fprintf(stderr, "ERROR: Improper variable name");
				exit(1);
			    }
			    i++;
			}
		    }
		}	
	    }
	}	
    }
}

token_type peek_type(lexer *lex) {
    if (!lex) {
	return token_SENTINEL;
    }
    if (lex->type == token_SENTINEL) {
	read_token(lex);
    }
    return lex->type;
}

char *peek_value(lexer *lex) {
    if (!lex) {
	return NULL;
    }
    if (lex->type == token_SENTINEL) {
	read_token(lex);
    }
    return lex->buffer;
}

/*
int main(int argc, char *argv[]) {
    printf("TEST\n");
    lexer *test = malloc(sizeof(lexer));
    init_lex(test);
    open_file(test, "lexer_test2.txt");

    
    read_token(test);
    printf("1: %s\n", test->buffer);
    read_token(test);
    printf("2: %s\n", test->buffer);
    read_token(test);
    printf("3: %s\n", test->buffer);
    read_token(test);
    printf("4: %s\n", test->buffer);
    read_token(test);
    printf("5: %s\n", test->buffer);
    read_token(test);
    printf("6: %s\n", test->buffer);
    read_token(test);
    printf("7: %s\n", test->buffer);
    read_token(test);
    printf("8: %s\n", test->buffer);
    
    read_token(test);
    printf("9: %s\n", test->buffer);
    read_token(test);
    printf("10: %s\n", test->buffer);
    read_token(test);
    printf("11: %s\n", test->buffer);
    read_token(test);
    printf("12: %s\n", test->buffer);
    read_token(test);
    printf("13: %s\n", test->buffer);
    read_token(test);
    printf("14: %s\n", test->buffer);
    read_token(test);
    printf("15: %s\n", test->buffer);
    read_token(test);
    printf("16: %s\n", test->buffer);
    
    
    int i = 1;
    if(test->buffer) {
	while((int) test->type != token_END) {
	    read_token(test);
	    printf("%d: %s\n", i, test->buffer);
	    printf("%u\n", test->type);
	    i++;
	}
    }
    return 0;
}
*/
