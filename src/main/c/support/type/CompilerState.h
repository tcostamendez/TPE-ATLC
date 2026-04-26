#ifndef COMPILER_STATE_HEADER
#define COMPILER_STATE_HEADER

typedef struct Program Program;

/**
 * The global state of the compiler. Should transport every data structure
 * needed across the different phases of a compilation.
 */
typedef struct {
	/**
	 * The root node of the AST.
	 */
	Program * abstractSyntaxTree;

	// TODO: Add a symbol table.
	// TODO: Add a stack to handle nested scopes.
	// TODO: Add more configuration.
	// TODO: Add whatever you need.
} CompilerState;

#endif
