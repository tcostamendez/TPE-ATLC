#include "BisonActions.h"
#include <stddef.h>

/* MODULE INTERNAL STATE */

static CompilerState * _compilerState = NULL;
static Logger * _logger = NULL;
static CompilationStatus _status = SUCCEEDED;

static void * _allocateNode(size_t size);
static void * _appendNode(void * head, void * nextNode, size_t nextOffset, size_t tailOffset);
static void _failOutOfMemory();

/** Shutdown module's internal state. */
void _shutdownBisonActionsModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: BisonActions...");
		destroyLogger(_logger);
		_logger = NULL;
	}
	_compilerState = NULL;
	_status = SUCCEEDED;
}

ModuleDestructor initializeBisonActionsModule(CompilerState * compilerState) {
	_compilerState = compilerState;
	_logger = createLogger("BisonActions");
	_status = SUCCEEDED;
	return _shutdownBisonActionsModule;
}

CompilationStatus getBisonActionsStatus() {
	return _status;
}

static void _logSyntacticAnalyzerAction(const char * functionName) {
	logDebugging(_logger, "%s", functionName);
}

static void _failOutOfMemory() {
	if (_status != OUT_OF_MEMORY) {
		logError(_logger, "The compiler ran out of memory while building the syntax tree.");
	}
	_status = OUT_OF_MEMORY;
}

static void * _allocateNode(size_t size) {
	void * node = calloc(1, size);
	if (node == NULL) {
		_failOutOfMemory();
	}
	return node;
}

static void * _appendNode(void * head, void * nextNode, size_t nextOffset, size_t tailOffset) {
	if (head == NULL) {
		return nextNode;
	}
	if (nextNode == NULL) {
		return head;
	}

	void ** headTail = (void **) (((char *) head) + tailOffset);
	void * tail = *headTail != NULL ? *headTail : head;
	*(void **) (((char *) tail) + nextOffset) = nextNode;

	void * nextTail = *(void **) (((char *) nextNode) + tailOffset);
	*headTail = nextTail != NULL ? nextTail : nextNode;
	return head;
}

IdentifierList * IdentifierListSemanticAction(char * identifier) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (identifier == NULL) {
		return NULL;
	}

	IdentifierList * node = _allocateNode(sizeof(IdentifierList));
	if (node == NULL) {
		free(identifier);
		return NULL;
	}

	node->identifier = identifier;
	node->tail = node;
	return node;
}

IdentifierList * AppendIdentifierSemanticAction(IdentifierList * identifierList, char * identifier) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	IdentifierList * nextNode = IdentifierListSemanticAction(identifier);
	if (nextNode == NULL) {
		destroyIdentifierList(identifierList);
		return NULL;
	}
	return _appendNode(identifierList, nextNode, offsetof(IdentifierList, next), offsetof(IdentifierList, tail));
}

Connection * ConnectionSemanticAction(char * portName, char * signalName) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (portName == NULL || signalName == NULL) {
		free(portName);
		free(signalName);
		return NULL;
	}

	Connection * connection = _allocateNode(sizeof(Connection));
	if (connection == NULL) {
		free(portName);
		free(signalName);
		return NULL;
	}

	connection->portName = portName;
	connection->signalName = signalName;
	return connection;
}

ConnectionList * ConnectionListSemanticAction(Connection * connection) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (connection == NULL) {
		return NULL;
	}

	ConnectionList * node = _allocateNode(sizeof(ConnectionList));
	if (node == NULL) {
		destroyConnection(connection);
		return NULL;
	}

	node->connection = connection;
	node->tail = node;
	return node;
}

ConnectionList * AppendConnectionSemanticAction(ConnectionList * connectionList, Connection * connection) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ConnectionList * nextNode = ConnectionListSemanticAction(connection);
	if (nextNode == NULL) {
		destroyConnectionList(connectionList);
		return NULL;
	}
	return _appendNode(connectionList, nextNode, offsetof(ConnectionList, next), offsetof(ConnectionList, tail));
}

Declaration * DeclarationSemanticAction(DeclarationType type, IdentifierList * identifiers) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (identifiers == NULL) {
		return NULL;
	}

	Declaration * declaration = _allocateNode(sizeof(Declaration));
	if (declaration == NULL) {
		destroyIdentifierList(identifiers);
		return NULL;
	}

	declaration->type = type;
	declaration->identifiers = identifiers;
	return declaration;
}

Expression * IdentifierExpressionSemanticAction(char * identifier) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (identifier == NULL) {
		return NULL;
	}

	Expression * expression = _allocateNode(sizeof(Expression));
	if (expression == NULL) {
		free(identifier);
		return NULL;
	}

	expression->type = IDENTIFIER_EXPRESSION;
	expression->identifier = identifier;
	return expression;
}

Expression * UnaryExpressionSemanticAction(UnaryOperatorType unaryOperator, Expression * operand) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (operand == NULL) {
		return NULL;
	}

	Expression * expression = _allocateNode(sizeof(Expression));
	if (expression == NULL) {
		destroyExpression(operand);
		return NULL;
	}

	expression->type = UNARY_EXPRESSION;
	expression->unaryOperator = unaryOperator;
	expression->operand = operand;
	return expression;
}

Expression * BinaryExpressionSemanticAction(Expression * leftExpression, Expression * rightExpression, BinaryOperatorType binaryOperator) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (leftExpression == NULL || rightExpression == NULL) {
		destroyExpression(leftExpression);
		destroyExpression(rightExpression);
		return NULL;
	}

	Expression * expression = _allocateNode(sizeof(Expression));
	if (expression == NULL) {
		destroyExpression(leftExpression);
		destroyExpression(rightExpression);
		return NULL;
	}

	expression->type = BINARY_EXPRESSION;
	expression->binaryOperator = binaryOperator;
	expression->leftExpression = leftExpression;
	expression->rightExpression = rightExpression;
	return expression;
}

SequentialAssignment * SequentialAssignmentSemanticAction(char * target, Expression * expression) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (target == NULL || expression == NULL) {
		free(target);
		destroyExpression(expression);
		return NULL;
	}

	SequentialAssignment * assignment = _allocateNode(sizeof(SequentialAssignment));
	if (assignment == NULL) {
		free(target);
		destroyExpression(expression);
		return NULL;
	}

	assignment->target = target;
	assignment->expression = expression;
	return assignment;
}

SequentialAssignmentList * SequentialAssignmentListSemanticAction(SequentialAssignment * assignment) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (assignment == NULL) {
		return NULL;
	}

	SequentialAssignmentList * node = _allocateNode(sizeof(SequentialAssignmentList));
	if (node == NULL) {
		destroySequentialAssignment(assignment);
		return NULL;
	}

	node->assignment = assignment;
	node->tail = node;
	return node;
}

SequentialAssignmentList * AppendSequentialAssignmentSemanticAction(SequentialAssignmentList * assignmentList, SequentialAssignment * assignment) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	SequentialAssignmentList * nextNode = SequentialAssignmentListSemanticAction(assignment);
	if (nextNode == NULL) {
		destroySequentialAssignmentList(assignmentList);
		return NULL;
	}
	return _appendNode(assignmentList, nextNode, offsetof(SequentialAssignmentList, next), offsetof(SequentialAssignmentList, tail));
}

ClockBlock * ClockBlockSemanticAction(char * clockSignal, SequentialAssignmentList * assignments) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (clockSignal == NULL || assignments == NULL) {
		free(clockSignal);
		destroySequentialAssignmentList(assignments);
		return NULL;
	}

	ClockBlock * clockBlock = _allocateNode(sizeof(ClockBlock));
	if (clockBlock == NULL) {
		free(clockSignal);
		destroySequentialAssignmentList(assignments);
		return NULL;
	}

	clockBlock->clockSignal = clockSignal;
	clockBlock->assignments = assignments;
	return clockBlock;
}

Instance * InstanceSemanticAction(char * circuitName, ConnectionList * inputConnections, ConnectionList * outputConnections) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (circuitName == NULL) {
		destroyConnectionList(inputConnections);
		destroyConnectionList(outputConnections);
		return NULL;
	}

	Instance * instance = _allocateNode(sizeof(Instance));
	if (instance == NULL) {
		free(circuitName);
		destroyConnectionList(inputConnections);
		destroyConnectionList(outputConnections);
		return NULL;
	}

	instance->circuitName = circuitName;
	instance->inputConnections = inputConnections;
	instance->outputConnections = outputConnections;
	return instance;
}

Statement * CombinationalAssignmentStatementSemanticAction(char * target, Expression * expression) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (target == NULL || expression == NULL) {
		free(target);
		destroyExpression(expression);
		return NULL;
	}

	Statement * statement = _allocateNode(sizeof(Statement));
	if (statement == NULL) {
		free(target);
		destroyExpression(expression);
		return NULL;
	}

	statement->type = COMBINATIONAL_ASSIGNMENT_STATEMENT;
	statement->combinationalAssignment.target = target;
	statement->combinationalAssignment.expression = expression;
	return statement;
}

Statement * ClockBlockStatementSemanticAction(ClockBlock * clockBlock) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (clockBlock == NULL) {
		return NULL;
	}

	Statement * statement = _allocateNode(sizeof(Statement));
	if (statement == NULL) {
		destroyClockBlock(clockBlock);
		return NULL;
	}

	statement->type = CLOCK_BLOCK_STATEMENT;
	statement->clockBlock = clockBlock;
	return statement;
}

Statement * InstanceStatementSemanticAction(Instance * instance) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (instance == NULL) {
		return NULL;
	}

	Statement * statement = _allocateNode(sizeof(Statement));
	if (statement == NULL) {
		destroyInstance(instance);
		return NULL;
	}

	statement->type = INSTANCE_STATEMENT;
	statement->instance = instance;
	return statement;
}

CircuitItem * DeclarationCircuitItemSemanticAction(Declaration * declaration) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (declaration == NULL) {
		return NULL;
	}

	CircuitItem * item = _allocateNode(sizeof(CircuitItem));
	if (item == NULL) {
		destroyDeclaration(declaration);
		return NULL;
	}

	item->type = DECLARATION_ITEM;
	item->declaration = declaration;
	return item;
}

CircuitItem * StatementCircuitItemSemanticAction(Statement * statement) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (statement == NULL) {
		return NULL;
	}

	CircuitItem * item = _allocateNode(sizeof(CircuitItem));
	if (item == NULL) {
		destroyStatement(statement);
		return NULL;
	}

	item->type = STATEMENT_ITEM;
	item->statement = statement;
	return item;
}

CircuitItemList * CircuitItemListSemanticAction(CircuitItem * item) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (item == NULL) {
		return NULL;
	}

	CircuitItemList * node = _allocateNode(sizeof(CircuitItemList));
	if (node == NULL) {
		destroyCircuitItem(item);
		return NULL;
	}

	node->item = item;
	node->tail = node;
	return node;
}

CircuitItemList * AppendCircuitItemSemanticAction(CircuitItemList * itemList, CircuitItem * item) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	CircuitItemList * nextNode = CircuitItemListSemanticAction(item);
	if (nextNode == NULL) {
		destroyCircuitItemList(itemList);
		return NULL;
	}
	return _appendNode(itemList, nextNode, offsetof(CircuitItemList, next), offsetof(CircuitItemList, tail));
}

Circuit * CircuitSemanticAction(char * name, CircuitItemList * items) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (name == NULL) {
		destroyCircuitItemList(items);
		return NULL;
	}

	Circuit * circuit = _allocateNode(sizeof(Circuit));
	if (circuit == NULL) {
		free(name);
		destroyCircuitItemList(items);
		return NULL;
	}

	circuit->name = name;
	circuit->items = items;
	return circuit;
}

CircuitList * CircuitListSemanticAction(Circuit * circuit) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (circuit == NULL) {
		return NULL;
	}

	CircuitList * node = _allocateNode(sizeof(CircuitList));
	if (node == NULL) {
		destroyCircuit(circuit);
		return NULL;
	}

	node->circuit = circuit;
	node->tail = node;
	return node;
}

CircuitList * AppendCircuitSemanticAction(CircuitList * circuitList, Circuit * circuit) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	CircuitList * nextNode = CircuitListSemanticAction(circuit);
	if (nextNode == NULL) {
		destroyCircuitList(circuitList);
		return NULL;
	}
	return _appendNode(circuitList, nextNode, offsetof(CircuitList, next), offsetof(CircuitList, tail));
}

Program * ProgramSemanticAction(CircuitList * circuits) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (circuits == NULL) {
		if (_compilerState != NULL) {
			_compilerState->abstractSyntaxTree = NULL;
		}
		return NULL;
	}

	Program * program = _allocateNode(sizeof(Program));
	if (program == NULL) {
		destroyCircuitList(circuits);
		if (_compilerState != NULL) {
			_compilerState->abstractSyntaxTree = NULL;
		}
		return NULL;
	}

	program->circuits = circuits;
	if (_compilerState != NULL) {
		_compilerState->abstractSyntaxTree = program;
	}
	return program;
}
