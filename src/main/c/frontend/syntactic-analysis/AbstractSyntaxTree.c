#include "AbstractSyntaxTree.h"

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

/** Shutdown module's internal state. */
void _shutdownAbstractSyntaxTreeModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: AbstractSyntaxTree...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeAbstractSyntaxTreeModule() {
	_logger = createLogger("AbstractSyntaxTree");
	return _shutdownAbstractSyntaxTreeModule;
}

void destroyIdentifierList(IdentifierList * identifierList) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	while (identifierList != NULL) {
		IdentifierList * next = identifierList->next;
		free(identifierList->identifier);
		free(identifierList);
		identifierList = next;
	}
}

void destroyConnection(Connection * connection) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (connection != NULL) {
		free(connection->portName);
		free(connection->signalName);
		free(connection);
	}
}

void destroyConnectionList(ConnectionList * connectionList) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	while (connectionList != NULL) {
		ConnectionList * next = connectionList->next;
		destroyConnection(connectionList->connection);
		free(connectionList);
		connectionList = next;
	}
}

void destroyDeclaration(Declaration * declaration) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (declaration != NULL) {
		destroyIdentifierList(declaration->identifiers);
		free(declaration);
	}
}

void destroyExpression(Expression * expression) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (expression != NULL) {
		switch (expression->type) {
			case IDENTIFIER_EXPRESSION:
				free(expression->identifier);
				break;
			case UNARY_EXPRESSION:
				destroyExpression(expression->operand);
				break;
			case BINARY_EXPRESSION:
				destroyExpression(expression->leftExpression);
				destroyExpression(expression->rightExpression);
				break;
		}
		free(expression);
	}
}

void destroySequentialAssignment(SequentialAssignment * assignment) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (assignment != NULL) {
		free(assignment->target);
		destroyExpression(assignment->expression);
		free(assignment);
	}
}

void destroySequentialAssignmentList(SequentialAssignmentList * assignmentList) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	while (assignmentList != NULL) {
		SequentialAssignmentList * next = assignmentList->next;
		destroySequentialAssignment(assignmentList->assignment);
		free(assignmentList);
		assignmentList = next;
	}
}

void destroyClockBlock(ClockBlock * clockBlock) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (clockBlock != NULL) {
		free(clockBlock->clockSignal);
		destroySequentialAssignmentList(clockBlock->assignments);
		free(clockBlock);
	}
}

void destroyInstance(Instance * instance) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (instance != NULL) {
		free(instance->circuitName);
		destroyConnectionList(instance->inputConnections);
		destroyConnectionList(instance->outputConnections);
		free(instance);
	}
}

void destroyStatement(Statement * statement) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (statement != NULL) {
		switch (statement->type) {
			case COMBINATIONAL_ASSIGNMENT_STATEMENT:
				free(statement->combinationalAssignment.target);
				destroyExpression(statement->combinationalAssignment.expression);
				break;
			case CLOCK_BLOCK_STATEMENT:
				destroyClockBlock(statement->clockBlock);
				break;
			case INSTANCE_STATEMENT:
				destroyInstance(statement->instance);
				break;
		}
		free(statement);
	}
}

void destroyCircuitItem(CircuitItem * item) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (item != NULL) {
		switch (item->type) {
			case DECLARATION_ITEM:
				destroyDeclaration(item->declaration);
				break;
			case STATEMENT_ITEM:
				destroyStatement(item->statement);
				break;
		}
		free(item);
	}
}

void destroyCircuitItemList(CircuitItemList * itemList) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	while (itemList != NULL) {
		CircuitItemList * next = itemList->next;
		destroyCircuitItem(itemList->item);
		free(itemList);
		itemList = next;
	}
}

void destroyCircuit(Circuit * circuit) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (circuit != NULL) {
		free(circuit->name);
		destroyCircuitItemList(circuit->items);
		free(circuit);
	}
}

void destroyCircuitList(CircuitList * circuitList) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	while (circuitList != NULL) {
		CircuitList * next = circuitList->next;
		destroyCircuit(circuitList->circuit);
		free(circuitList);
		circuitList = next;
	}
}

void destroyProgram(Program * program) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (program != NULL) {
		destroyCircuitList(program->circuits);
		free(program);
	}
}
