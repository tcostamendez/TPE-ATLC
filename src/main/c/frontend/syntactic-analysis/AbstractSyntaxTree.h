#ifndef ABSTRACT_SYNTAX_TREE_HEADER
#define ABSTRACT_SYNTAX_TREE_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/ModuleDestructor.h"
#include <stdlib.h>

/** Initialize module's internal state. */
ModuleDestructor initializeAbstractSyntaxTreeModule();

typedef enum DeclarationType DeclarationType;
typedef enum CircuitItemType CircuitItemType;
typedef enum StatementType StatementType;
typedef enum UnaryOperatorType UnaryOperatorType;
typedef enum BinaryOperatorType BinaryOperatorType;
typedef enum ExpressionType ExpressionType;

typedef struct IdentifierList IdentifierList;
typedef struct Connection Connection;
typedef struct ConnectionList ConnectionList;
typedef struct Declaration Declaration;
typedef struct Expression Expression;
typedef struct CombinationalAssignment CombinationalAssignment;
typedef struct SequentialAssignment SequentialAssignment;
typedef struct SequentialAssignmentList SequentialAssignmentList;
typedef struct ClockBlock ClockBlock;
typedef struct Instance Instance;
typedef struct Statement Statement;
typedef struct CircuitItem CircuitItem;
typedef struct CircuitItemList CircuitItemList;
typedef struct Circuit Circuit;
typedef struct CircuitList CircuitList;
typedef struct Program Program;

enum DeclarationType {
	INPUT_DECLARATION,
	OUTPUT_DECLARATION,
	WIRE_DECLARATION,
	REG_DECLARATION
};

enum CircuitItemType {
	DECLARATION_ITEM,
	STATEMENT_ITEM
};

enum StatementType {
	COMBINATIONAL_ASSIGNMENT_STATEMENT,
	CLOCK_BLOCK_STATEMENT,
	INSTANCE_STATEMENT
};

enum UnaryOperatorType {
	NOT_OPERATOR
};

enum BinaryOperatorType {
	AND_OPERATOR,
	OR_OPERATOR,
	XOR_OPERATOR
};

enum ExpressionType {
	IDENTIFIER_EXPRESSION,
	UNARY_EXPRESSION,
	BINARY_EXPRESSION
};

struct IdentifierList {
	char * identifier;
	IdentifierList * next;
	IdentifierList * tail;
};

struct Connection {
	char * portName;
	char * signalName;
};

struct ConnectionList {
	Connection * connection;
	ConnectionList * next;
	ConnectionList * tail;
};

struct Declaration {
	DeclarationType type;
	IdentifierList * identifiers;
};

struct Expression {
	ExpressionType type;
	union {
		char * identifier;
		struct {
			UnaryOperatorType unaryOperator;
			Expression * operand;
		};
		struct {
			BinaryOperatorType binaryOperator;
			Expression * leftExpression;
			Expression * rightExpression;
		};
	};
};

struct CombinationalAssignment {
	char * target;
	Expression * expression;
};

struct SequentialAssignment {
	char * target;
	Expression * expression;
};

struct SequentialAssignmentList {
	SequentialAssignment * assignment;
	SequentialAssignmentList * next;
	SequentialAssignmentList * tail;
};

struct ClockBlock {
	char * clockSignal;
	SequentialAssignmentList * assignments;
};

struct Instance {
	char * circuitName;
	ConnectionList * inputConnections;
	ConnectionList * outputConnections;
};

struct Statement {
	StatementType type;
	union {
		CombinationalAssignment * combinationalAssignment;
		ClockBlock * clockBlock;
		Instance * instance;
	};
};

struct CircuitItem {
	CircuitItemType type;
	union {
		Declaration * declaration;
		Statement * statement;
	};
};

struct CircuitItemList {
	CircuitItem * item;
	CircuitItemList * next;
	CircuitItemList * tail;
};

struct Circuit {
	char * name;
	CircuitItemList * items;
};

struct CircuitList {
	Circuit * circuit;
	CircuitList * next;
	CircuitList * tail;
};

struct Program {
	CircuitList * circuits;
};

void destroyIdentifierList(IdentifierList * identifierList);
void destroyConnection(Connection * connection);
void destroyConnectionList(ConnectionList * connectionList);
void destroyDeclaration(Declaration * declaration);
void destroyExpression(Expression * expression);
void destroyCombinationalAssignment(CombinationalAssignment * assignment);
void destroySequentialAssignment(SequentialAssignment * assignment);
void destroySequentialAssignmentList(SequentialAssignmentList * assignmentList);
void destroyClockBlock(ClockBlock * clockBlock);
void destroyInstance(Instance * instance);
void destroyStatement(Statement * statement);
void destroyCircuitItem(CircuitItem * item);
void destroyCircuitItemList(CircuitItemList * itemList);
void destroyCircuit(Circuit * circuit);
void destroyCircuitList(CircuitList * circuitList);
void destroyProgram(Program * program);

#endif
