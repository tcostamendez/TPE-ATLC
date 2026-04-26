#include "BisonActions.h"
#include <stddef.h>

/* MODULE INTERNAL STATE */

static CompilerState * _compilerState = NULL;
static Logger * _logger = NULL;

/** Shutdown module's internal state. */
void _shutdownBisonActionsModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: BisonActions...");
		destroyLogger(_logger);
		_logger = NULL;
	}
	_compilerState = NULL;
}

ModuleDestructor initializeBisonActionsModule(CompilerState * compilerState) {
	_compilerState = compilerState;
	_logger = createLogger("BisonActions");
	return _shutdownBisonActionsModule;
}

static void _logSyntacticAnalyzerAction(const char * functionName) {
	logDebugging(_logger, "%s", functionName);
}

static void * _appendNode(void * head, void * nextNode, size_t nextOffset) {
	if (head == NULL) {
		return nextNode;
	}

	char * current = head;
	while (*(void **) (current + nextOffset) != NULL) {
		current = *(void **) (current + nextOffset);
	}
	*(void **) (current + nextOffset) = nextNode;
	return head;
}

IdentifierList * IdentifierListSemanticAction(char * identifier) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	IdentifierList * node = calloc(1, sizeof(IdentifierList));
	node->identifier = identifier;
	return node;
}

IdentifierList * AppendIdentifierSemanticAction(IdentifierList * identifierList, char * identifier) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return _appendNode(identifierList, IdentifierListSemanticAction(identifier), offsetof(IdentifierList, next));
}

Connection * ConnectionSemanticAction(char * portName, char * signalName) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Connection * connection = calloc(1, sizeof(Connection));
	connection->portName = portName;
	connection->signalName = signalName;
	return connection;
}

ConnectionList * ConnectionListSemanticAction(Connection * connection) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ConnectionList * node = calloc(1, sizeof(ConnectionList));
	node->connection = connection;
	return node;
}

ConnectionList * AppendConnectionSemanticAction(ConnectionList * connectionList, Connection * connection) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return _appendNode(connectionList, ConnectionListSemanticAction(connection), offsetof(ConnectionList, next));
}

Declaration * DeclarationSemanticAction(DeclarationType type, IdentifierList * identifiers) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Declaration * declaration = calloc(1, sizeof(Declaration));
	declaration->type = type;
	declaration->identifiers = identifiers;
	return declaration;
}

Expression * IdentifierExpressionSemanticAction(char * identifier) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expression = calloc(1, sizeof(Expression));
	expression->type = IDENTIFIER_EXPRESSION;
	expression->identifier = identifier;
	return expression;
}

Expression * UnaryExpressionSemanticAction(UnaryOperatorType unaryOperator, Expression * operand) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expression = calloc(1, sizeof(Expression));
	expression->type = UNARY_EXPRESSION;
	expression->unaryOperator = unaryOperator;
	expression->operand = operand;
	return expression;
}

Expression * BinaryExpressionSemanticAction(Expression * leftExpression, Expression * rightExpression, BinaryOperatorType binaryOperator) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expression = calloc(1, sizeof(Expression));
	expression->type = BINARY_EXPRESSION;
	expression->binaryOperator = binaryOperator;
	expression->leftExpression = leftExpression;
	expression->rightExpression = rightExpression;
	return expression;
}

SequentialAssignment * SequentialAssignmentSemanticAction(char * target, Expression * expression) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	SequentialAssignment * assignment = calloc(1, sizeof(SequentialAssignment));
	assignment->target = target;
	assignment->expression = expression;
	return assignment;
}

SequentialAssignmentList * SequentialAssignmentListSemanticAction(SequentialAssignment * assignment) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	SequentialAssignmentList * node = calloc(1, sizeof(SequentialAssignmentList));
	node->assignment = assignment;
	return node;
}

SequentialAssignmentList * AppendSequentialAssignmentSemanticAction(SequentialAssignmentList * assignmentList, SequentialAssignment * assignment) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return _appendNode(assignmentList, SequentialAssignmentListSemanticAction(assignment), offsetof(SequentialAssignmentList, next));
}

ClockBlock * ClockBlockSemanticAction(char * clockSignal, SequentialAssignmentList * assignments) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ClockBlock * clockBlock = calloc(1, sizeof(ClockBlock));
	clockBlock->clockSignal = clockSignal;
	clockBlock->assignments = assignments;
	return clockBlock;
}

Instance * InstanceSemanticAction(char * circuitName, ConnectionList * inputConnections, ConnectionList * outputConnections) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Instance * instance = calloc(1, sizeof(Instance));
	instance->circuitName = circuitName;
	instance->inputConnections = inputConnections;
	instance->outputConnections = outputConnections;
	return instance;
}

Statement * CombinationalAssignmentStatementSemanticAction(char * target, Expression * expression) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Statement * statement = calloc(1, sizeof(Statement));
	statement->type = COMBINATIONAL_ASSIGNMENT_STATEMENT;
	statement->combinationalAssignment.target = target;
	statement->combinationalAssignment.expression = expression;
	return statement;
}

Statement * ClockBlockStatementSemanticAction(ClockBlock * clockBlock) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Statement * statement = calloc(1, sizeof(Statement));
	statement->type = CLOCK_BLOCK_STATEMENT;
	statement->clockBlock = clockBlock;
	return statement;
}

Statement * InstanceStatementSemanticAction(Instance * instance) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Statement * statement = calloc(1, sizeof(Statement));
	statement->type = INSTANCE_STATEMENT;
	statement->instance = instance;
	return statement;
}

CircuitItem * DeclarationCircuitItemSemanticAction(Declaration * declaration) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	CircuitItem * item = calloc(1, sizeof(CircuitItem));
	item->type = DECLARATION_ITEM;
	item->declaration = declaration;
	return item;
}

CircuitItem * StatementCircuitItemSemanticAction(Statement * statement) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	CircuitItem * item = calloc(1, sizeof(CircuitItem));
	item->type = STATEMENT_ITEM;
	item->statement = statement;
	return item;
}

CircuitItemList * CircuitItemListSemanticAction(CircuitItem * item) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	CircuitItemList * node = calloc(1, sizeof(CircuitItemList));
	node->item = item;
	return node;
}

CircuitItemList * AppendCircuitItemSemanticAction(CircuitItemList * itemList, CircuitItem * item) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return _appendNode(itemList, CircuitItemListSemanticAction(item), offsetof(CircuitItemList, next));
}

Circuit * CircuitSemanticAction(char * name, CircuitItemList * items) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Circuit * circuit = calloc(1, sizeof(Circuit));
	circuit->name = name;
	circuit->items = items;
	return circuit;
}

CircuitList * CircuitListSemanticAction(Circuit * circuit) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	CircuitList * node = calloc(1, sizeof(CircuitList));
	node->circuit = circuit;
	return node;
}

CircuitList * AppendCircuitSemanticAction(CircuitList * circuitList, Circuit * circuit) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return _appendNode(circuitList, CircuitListSemanticAction(circuit), offsetof(CircuitList, next));
}

Program * ProgramSemanticAction(CircuitList * circuits) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Program * program = calloc(1, sizeof(Program));
	program->circuits = circuits;
	_compilerState->abstractSyntaxTree = program;
	return program;
}
