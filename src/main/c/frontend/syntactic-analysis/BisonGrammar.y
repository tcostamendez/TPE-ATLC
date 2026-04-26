%{

#include "../../support/type/TokenLabel.h"
#include "../../support/logging/Logger.h"
#include "AbstractSyntaxTree.h"
#include "BisonActions.h"

static Logger * _parserLogger = NULL;

void yyerror(const YYLTYPE * location, const char * message) {
	if (_parserLogger == NULL) {
		_parserLogger = createLogger("BisonParser");
	}

	if (location != NULL) {
		logError(_parserLogger,
			"Frontend syntax error at line %d, column %d: %s",
			location->first_line,
			location->first_column,
			message);
	}
	else {
		logError(_parserLogger, "Frontend syntax error: %s", message);
	}
}

%}

%define api.pure full
%define api.push-pull push
%define api.value.union.name SemanticValue
%define parse.error detailed
%locations

%union {
	char * string;
	Connection * connection;
	ConnectionList * connectionList;
	Circuit * circuit;
	CircuitItem * circuitItem;
	CircuitItemList * circuitItemList;
	CircuitList * circuitList;
	ClockBlock * clockBlock;
	Declaration * declaration;
	Expression * expression;
	IdentifierList * identifierList;
	Instance * instance;
	Program * program;
	SequentialAssignment * sequentialAssignment;
	SequentialAssignmentList * sequentialAssignmentList;
	Statement * statement;
}

%destructor { free($$); } <string>
%destructor { destroyConnection($$); } <connection>
%destructor { destroyConnectionList($$); } <connectionList>
%destructor { destroyCircuit($$); } <circuit>
%destructor { destroyCircuitItem($$); } <circuitItem>
%destructor { destroyCircuitItemList($$); } <circuitItemList>
%destructor { destroyCircuitList($$); } <circuitList>
%destructor { destroyClockBlock($$); } <clockBlock>
%destructor { destroyDeclaration($$); } <declaration>
%destructor { destroyExpression($$); } <expression>
%destructor { destroyIdentifierList($$); } <identifierList>
%destructor { destroyInstance($$); } <instance>
%destructor { destroySequentialAssignment($$); } <sequentialAssignment>
%destructor { destroySequentialAssignmentList($$); } <sequentialAssignmentList>
%destructor { destroyStatement($$); } <statement>

%token <string> IDENTIFIER
%token CIRCUIT
%token INPUT
%token OUTPUT
%token WIRE
%token REG
%token ON
%token RISING_EDGE
%token AND
%token OR
%token XOR
%token NOT
%token OPEN_BRACE
%token CLOSE_BRACE
%token OPEN_PARENTHESIS
%token CLOSE_PARENTHESIS
%token COMMA
%token SEMICOLON
%token ASSIGN
%token NEXT_ASSIGN
%token ARROW

%token IGNORED
%token UNKNOWN

%type <connection> connection
%type <connectionList> connection_list connection_list_opt
%type <circuit> circuit
%type <circuitItem> circuit_item
%type <circuitItemList> circuit_item_list circuit_item_list_opt
%type <circuitList> circuit_list
%type <clockBlock> clock_block
%type <declaration> declaration
%type <expression> expression
%type <identifierList> identifier_list
%type <instance> instance
%type <program> program
%type <sequentialAssignment> sequential_assignment
%type <sequentialAssignmentList> sequential_assignment_list
%type <statement> statement

%left OR
%left XOR
%left AND
%right NOT

%%

program: circuit_list													{ $$ = ProgramSemanticAction($1); }
	;

circuit_list: circuit													{ $$ = CircuitListSemanticAction($1); }
	| circuit_list circuit												{ $$ = AppendCircuitSemanticAction($1, $2); }
	;

circuit: CIRCUIT IDENTIFIER OPEN_BRACE circuit_item_list_opt CLOSE_BRACE
																		{ $$ = CircuitSemanticAction($2, $4); }
	;

circuit_item_list_opt: %empty											{ $$ = NULL; }
	| circuit_item_list												{ $$ = $1; }
	;

circuit_item_list: circuit_item											{ $$ = CircuitItemListSemanticAction($1); }
	| circuit_item_list circuit_item									{ $$ = AppendCircuitItemSemanticAction($1, $2); }
	;

circuit_item: declaration												{ $$ = DeclarationCircuitItemSemanticAction($1); }
	| statement														{ $$ = StatementCircuitItemSemanticAction($1); }
	;

declaration: INPUT identifier_list SEMICOLON							{ $$ = DeclarationSemanticAction(INPUT_DECLARATION, $2); }
	| OUTPUT identifier_list SEMICOLON									{ $$ = DeclarationSemanticAction(OUTPUT_DECLARATION, $2); }
	| WIRE identifier_list SEMICOLON									{ $$ = DeclarationSemanticAction(WIRE_DECLARATION, $2); }
	| REG identifier_list SEMICOLON									{ $$ = DeclarationSemanticAction(REG_DECLARATION, $2); }
	;

identifier_list: IDENTIFIER												{ $$ = IdentifierListSemanticAction($1); }
	| identifier_list COMMA IDENTIFIER									{ $$ = AppendIdentifierSemanticAction($1, $3); }
	;

statement: IDENTIFIER ASSIGN expression SEMICOLON						{ $$ = CombinationalAssignmentStatementSemanticAction($1, $3); }
	| clock_block														{ $$ = ClockBlockStatementSemanticAction($1); }
	| instance SEMICOLON												{ $$ = InstanceStatementSemanticAction($1); }
	;

clock_block: ON RISING_EDGE OPEN_PARENTHESIS IDENTIFIER CLOSE_PARENTHESIS OPEN_BRACE sequential_assignment_list CLOSE_BRACE
																		{ $$ = ClockBlockSemanticAction($4, $7); }
	;

sequential_assignment_list: sequential_assignment						{ $$ = SequentialAssignmentListSemanticAction($1); }
	| sequential_assignment_list sequential_assignment					{ $$ = AppendSequentialAssignmentSemanticAction($1, $2); }
	;

sequential_assignment: IDENTIFIER NEXT_ASSIGN expression SEMICOLON		{ $$ = SequentialAssignmentSemanticAction($1, $3); }
	;

instance: IDENTIFIER OPEN_PARENTHESIS connection_list_opt CLOSE_PARENTHESIS ARROW OPEN_PARENTHESIS connection_list_opt CLOSE_PARENTHESIS
																		{ $$ = InstanceSemanticAction($1, $3, $7); }
	;

connection_list_opt: %empty											{ $$ = NULL; }
	| connection_list													{ $$ = $1; }
	;

connection_list: connection												{ $$ = ConnectionListSemanticAction($1); }
	| connection_list COMMA connection									{ $$ = AppendConnectionSemanticAction($1, $3); }
	;

connection: IDENTIFIER ASSIGN IDENTIFIER								{ $$ = ConnectionSemanticAction($1, $3); }
	;

expression: IDENTIFIER													{ $$ = IdentifierExpressionSemanticAction($1); }
	| NOT expression													{ $$ = UnaryExpressionSemanticAction(NOT_OPERATOR, $2); }
	| expression AND expression										{ $$ = BinaryExpressionSemanticAction($1, $3, AND_OPERATOR); }
	| expression XOR expression										{ $$ = BinaryExpressionSemanticAction($1, $3, XOR_OPERATOR); }
	| expression OR expression											{ $$ = BinaryExpressionSemanticAction($1, $3, OR_OPERATOR); }
	| OPEN_PARENTHESIS expression CLOSE_PARENTHESIS					{ $$ = $2; }
	;

%%
