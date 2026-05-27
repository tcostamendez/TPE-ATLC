#include "Generator.h"
#include "../domain-specific/Calculator.h"

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

static void _emitExpression(Expression * expression);
static void _emitCircuitFunctions(SemanticCircuit * circuit);

static void _emitStateTypeName(const char * circuitName) {
	printf("State_c_%s", circuitName);
}

static void _emitInitFunctionName(const char * circuitName) {
	printf("init_c_%s", circuitName);
}

static void _emitFreeFunctionName(const char * circuitName) {
	printf("free_c_%s", circuitName);
}

static void _emitSettleFunctionName(const char * circuitName) {
	printf("settle_c_%s", circuitName);
}

static void _emitTickFunctionName(const char * circuitName) {
	printf("tick_c_%s", circuitName);
}

static void _emitSignalFieldName(const char * signalName) {
	printf("sig_%s", signalName);
}

static void _emitPreviousSignalFieldName(const char * signalName) {
	printf("prev_sig_%s", signalName);
}

static void _emitInputVariableName(const char * signalName) {
	printf("in_sig_%s", signalName);
}

static void _emitInstanceFieldName(size_t index) {
	printf("inst_%zu", index);
}

void _shutdownGeneratorModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: Generator...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeGeneratorModule() {
	_logger = createLogger("Generator");
	return _shutdownGeneratorModule;
}

static size_t _instanceIndex(CircuitItemList * limit, Statement * statement) {
	size_t index = 0;
	for (CircuitItemList * node = limit; node != NULL; node = node->next) {
		if (node->item->type == STATEMENT_ITEM && node->item->statement->type == INSTANCE_STATEMENT) {
			if (node->item->statement == statement) {
				return index;
			}
			++index;
		}
	}
	return index;
}

static void _emitSignalFields(SemanticCircuit * circuit) {
	for (size_t index = 0; index < circuit->signalCount; ++index) {
		printf("\tint ");
		_emitSignalFieldName(circuit->signals[index].name);
		printf(";\n");
		if (circuit->signals[index].type == INPUT_DECLARATION) {
			printf("\tint ");
			_emitPreviousSignalFieldName(circuit->signals[index].name);
			printf(";\n");
		}
	}
}

static void _emitInstanceFields(SemanticCircuit * circuit) {
	size_t index = 0;
	for (CircuitItemList * node = circuit->ast->items; node != NULL; node = node->next) {
		if (node->item->type == STATEMENT_ITEM && node->item->statement->type == INSTANCE_STATEMENT) {
			printf("\t");
			_emitStateTypeName(node->item->statement->instance->circuitName);
			printf(" * ");
			_emitInstanceFieldName(index++);
			printf(";\n");
		}
	}
}

static void _emitExpression(Expression * expression) {
	if (expression == NULL) {
		printf("0");
		return;
	}
	switch (expression->type) {
		case IDENTIFIER_EXPRESSION:
			printf("(state->");
			_emitSignalFieldName(expression->identifier);
			printf(")");
			break;
		case UNARY_EXPRESSION:
			printf("(!");
			_emitExpression(expression->operand);
			printf(")");
			break;
		case BINARY_EXPRESSION:
			printf("(");
			_emitExpression(expression->leftExpression);
			if (expression->binaryOperator == AND_OPERATOR) {
				printf(" && ");
			}
			else if (expression->binaryOperator == OR_OPERATOR) {
				printf(" || ");
			}
			else {
				printf(" != ");
			}
			_emitExpression(expression->rightExpression);
			printf(")");
			break;
	}
}

static void _emitInstanceSettle(Instance * instance, size_t index) {
	for (ConnectionList * node = instance->inputConnections; node != NULL; node = node->next) {
		printf("\tstate->");
		_emitInstanceFieldName(index);
		printf("->");
		_emitSignalFieldName(node->connection->portName);
		printf(" = state->");
		_emitSignalFieldName(node->connection->signalName);
		printf(";\n");
	}
	printf("\t");
	_emitSettleFunctionName(instance->circuitName);
	printf("(state->");
	_emitInstanceFieldName(index);
	printf(");\n");
	for (ConnectionList * node = instance->outputConnections; node != NULL; node = node->next) {
		printf("\tstate->");
		_emitSignalFieldName(node->connection->signalName);
		printf(" = state->");
		_emitInstanceFieldName(index);
		printf("->");
		_emitSignalFieldName(node->connection->portName);
		printf(";\n");
	}
}

static void _emitInstanceTick(Instance * instance, size_t index) {
	printf("\t");
	_emitTickFunctionName(instance->circuitName);
	printf("(state->");
	_emitInstanceFieldName(index);
	for (ConnectionList * node = instance->inputConnections; node != NULL; node = node->next) {
		printf(", state->");
		_emitSignalFieldName(node->connection->signalName);
	}
	printf(");\n");
	for (ConnectionList * node = instance->outputConnections; node != NULL; node = node->next) {
		printf("\tstate->");
		_emitSignalFieldName(node->connection->signalName);
		printf(" = state->");
		_emitInstanceFieldName(index);
		printf("->");
		_emitSignalFieldName(node->connection->portName);
		printf(";\n");
	}
}

static void _emitInitFunction(SemanticCircuit * circuit) {
	printf("void ");
	_emitInitFunctionName(circuit->name);
	printf("(");
	_emitStateTypeName(circuit->name);
	printf(" * state) {\n");
	printf("\tmemset(state, 0, sizeof(*state));\n");
	size_t index = 0;
	for (CircuitItemList * node = circuit->ast->items; node != NULL; node = node->next) {
		if (node->item->type == STATEMENT_ITEM && node->item->statement->type == INSTANCE_STATEMENT) {
			Instance * instance = node->item->statement->instance;
			printf("\tstate->");
			_emitInstanceFieldName(index);
			printf(" = calloc(1, sizeof(");
			_emitStateTypeName(instance->circuitName);
			printf("));\n");
			printf("\tif (state->");
			_emitInstanceFieldName(index);
			printf(" == NULL) { fprintf(stderr, \"runtime allocation failed\\n\"); exit(2); }\n");
			printf("\t");
			_emitInitFunctionName(instance->circuitName);
			printf("(state->");
			_emitInstanceFieldName(index);
			printf(");\n");
			++index;
		}
	}
	printf("}\n\n");
}

static void _emitFreeFunction(SemanticCircuit * circuit) {
	printf("void ");
	_emitFreeFunctionName(circuit->name);
	printf("(");
	_emitStateTypeName(circuit->name);
	printf(" * state) {\n");
	printf("\t(void) state;\n");
	size_t index = 0;
	for (CircuitItemList * node = circuit->ast->items; node != NULL; node = node->next) {
		if (node->item->type == STATEMENT_ITEM && node->item->statement->type == INSTANCE_STATEMENT) {
			Instance * instance = node->item->statement->instance;
			printf("\tif (state->");
			_emitInstanceFieldName(index);
			printf(" != NULL) { ");
			_emitFreeFunctionName(instance->circuitName);
			printf("(state->");
			_emitInstanceFieldName(index);
			printf("); free(state->");
			_emitInstanceFieldName(index);
			printf("); }\n");
			++index;
		}
	}
	printf("}\n\n");
}

static void _emitSettleFunction(SemanticCircuit * circuit) {
	printf("void ");
	_emitSettleFunctionName(circuit->name);
	printf("(");
	_emitStateTypeName(circuit->name);
	printf(" * state) {\n");
	printf("\tfor (int __settle_iter = 0; __settle_iter < %zu; ++__settle_iter) {\n", circuit->signalCount + 1);
	for (CircuitItemList * node = circuit->ast->items; node != NULL; node = node->next) {
		if (node->item->type != STATEMENT_ITEM) {
			continue;
		}
		Statement * statement = node->item->statement;
		if (statement->type == COMBINATIONAL_ASSIGNMENT_STATEMENT) {
			printf("\t\tstate->");
			_emitSignalFieldName(statement->combinationalAssignment->target);
			printf(" = ");
			_emitExpression(statement->combinationalAssignment->expression);
			printf(" ? 1 : 0;\n");
		}
		else if (statement->type == INSTANCE_STATEMENT) {
			_emitInstanceSettle(statement->instance, _instanceIndex(circuit->ast->items, statement));
		}
	}
	printf("\t}\n");
	printf("}\n\n");
}

static void _emitTickFunction(SemanticCircuit * circuit) {
	printf("void ");
	_emitTickFunctionName(circuit->name);
	printf("(");
	_emitStateTypeName(circuit->name);
	printf(" * state");
	for (size_t index = 0; index < circuit->signalCount; ++index) {
		if (circuit->signals[index].type == INPUT_DECLARATION) {
			printf(", int ");
			_emitInputVariableName(circuit->signals[index].name);
		}
	}
	printf(") {\n");
	for (size_t index = 0; index < circuit->signalCount; ++index) {
		if (circuit->signals[index].type == INPUT_DECLARATION) {
			printf("\tstate->");
			_emitSignalFieldName(circuit->signals[index].name);
			printf(" = ");
			_emitInputVariableName(circuit->signals[index].name);
			printf(" ? 1 : 0;\n");
		}
	}
	printf("\t");
	_emitSettleFunctionName(circuit->name);
	printf("(state);\n");
	for (CircuitItemList * node = circuit->ast->items; node != NULL; node = node->next) {
		if (node->item->type != STATEMENT_ITEM || node->item->statement->type != CLOCK_BLOCK_STATEMENT) {
			continue;
		}
		ClockBlock * block = node->item->statement->clockBlock;
		if (block->edgeType == RISING_EDGE_EVENT) {
			printf("\tif (!state->");
			_emitPreviousSignalFieldName(block->clockSignal);
			printf(" && state->");
			_emitSignalFieldName(block->clockSignal);
			printf(") {\n");
		}
		else {
			printf("\tif (state->");
			_emitPreviousSignalFieldName(block->clockSignal);
			printf(" && !state->");
			_emitSignalFieldName(block->clockSignal);
			printf(") {\n");
		}
		size_t assignmentIndex = 0;
		for (SequentialAssignmentList * assignmentNode = block->assignments; assignmentNode != NULL; assignmentNode = assignmentNode->next) {
			printf("\t\tint next_%zu = ", assignmentIndex);
			_emitExpression(assignmentNode->assignment->expression);
			printf(" ? 1 : 0;\n");
			++assignmentIndex;
		}
		assignmentIndex = 0;
		for (SequentialAssignmentList * assignmentNode = block->assignments; assignmentNode != NULL; assignmentNode = assignmentNode->next) {
			printf("\t\tstate->");
			_emitSignalFieldName(assignmentNode->assignment->target);
			printf(" = next_%zu;\n", assignmentIndex++);
		}
		printf("\t}\n");
	}
	for (CircuitItemList * node = circuit->ast->items; node != NULL; node = node->next) {
		if (node->item->type == STATEMENT_ITEM && node->item->statement->type == INSTANCE_STATEMENT) {
			_emitInstanceTick(node->item->statement->instance, _instanceIndex(circuit->ast->items, node->item->statement));
		}
	}
	printf("\t");
	_emitSettleFunctionName(circuit->name);
	printf("(state);\n");
	for (size_t index = 0; index < circuit->signalCount; ++index) {
		if (circuit->signals[index].type == INPUT_DECLARATION) {
			printf("\tstate->");
			_emitPreviousSignalFieldName(circuit->signals[index].name);
			printf(" = state->");
			_emitSignalFieldName(circuit->signals[index].name);
			printf(";\n");
		}
	}
	printf("}\n\n");
}

static void _emitCircuitFunctions(SemanticCircuit * circuit) {
	_emitInitFunction(circuit);
	_emitFreeFunction(circuit);
	_emitSettleFunction(circuit);
	_emitTickFunction(circuit);
}

static void _emitMain(SemanticCircuit * top) {
	printf("int main(void) {\n");
	printf("\t");
	_emitStateTypeName(top->name);
	printf(" state;\n");
	printf("\t");
	_emitInitFunctionName(top->name);
	printf("(&state);\n");
	printf("\tint cycles = 0;\n");
	printf("\tif (scanf(\"%%d\", &cycles) != 1) { ");
	_emitFreeFunctionName(top->name);
	printf("(&state); return 1; }\n");
	printf("\tfor (int cycle = 0; cycle < cycles; ++cycle) {\n");
	for (size_t index = 0; index < top->signalCount; ++index) {
		if (top->signals[index].type == INPUT_DECLARATION) {
			printf("\t\tint ");
			_emitInputVariableName(top->signals[index].name);
			printf(" = 0;\n");
		}
	}
	for (size_t index = 0; index < top->signalCount; ++index) {
		if (top->signals[index].type == INPUT_DECLARATION) {
			printf("\t\tif (scanf(\"%%d\", &");
			_emitInputVariableName(top->signals[index].name);
			printf(") != 1) { ");
			_emitFreeFunctionName(top->name);
			printf("(&state); return 1; }\n");
		}
	}
	printf("\t\t");
	_emitTickFunctionName(top->name);
	printf("(&state");
	for (size_t index = 0; index < top->signalCount; ++index) {
		if (top->signals[index].type == INPUT_DECLARATION) {
			printf(", ");
			_emitInputVariableName(top->signals[index].name);
		}
	}
	printf(");\n");
	bool first = true;
	for (size_t index = 0; index < top->signalCount; ++index) {
		if (top->signals[index].type != OUTPUT_DECLARATION) {
			continue;
		}
		printf("\t\tprintf(\"%s%%d\", state.", first ? "" : " ");
		_emitSignalFieldName(top->signals[index].name);
		printf(");\n");
		first = false;
	}
	printf("\t\tprintf(\"\\n\");\n");
	printf("\t}\n");
	printf("\t");
	_emitFreeFunctionName(top->name);
	printf("(&state);\n");
	printf("\treturn 0;\n");
	printf("}\n");
}

void executeGenerator(CompilerState * compilerState) {
	if (compilerState == NULL || compilerState->semanticModel == NULL || compilerState->semanticModel->topCircuit == NULL) {
		logDebugging(_logger, "Skipping code generation because semantic analysis did not produce a model.");
		return;
	}

	SemanticModel * model = compilerState->semanticModel;
	printf("#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n\n");
	for (size_t index = 0; index < model->circuitCount; ++index) {
		printf("typedef struct ");
		_emitStateTypeName(model->circuits[index].name);
		printf(" ");
		_emitStateTypeName(model->circuits[index].name);
		printf(";\n");
	}
	printf("\n");
	for (size_t index = 0; index < model->circuitCount; ++index) {
		printf("void ");
		_emitInitFunctionName(model->circuits[index].name);
		printf("(");
		_emitStateTypeName(model->circuits[index].name);
		printf(" * state);\n");
		printf("void ");
		_emitFreeFunctionName(model->circuits[index].name);
		printf("(");
		_emitStateTypeName(model->circuits[index].name);
		printf(" * state);\n");
		printf("void ");
		_emitSettleFunctionName(model->circuits[index].name);
		printf("(");
		_emitStateTypeName(model->circuits[index].name);
		printf(" * state);\n");
		printf("void ");
		_emitTickFunctionName(model->circuits[index].name);
		printf("(");
		_emitStateTypeName(model->circuits[index].name);
		printf(" * state");
		for (size_t signalIndex = 0; signalIndex < model->circuits[index].signalCount; ++signalIndex) {
			if (model->circuits[index].signals[signalIndex].type == INPUT_DECLARATION) {
				printf(", int ");
				_emitInputVariableName(model->circuits[index].signals[signalIndex].name);
			}
		}
		printf(");\n");
	}
	printf("\n");
	for (size_t index = 0; index < model->circuitCount; ++index) {
		SemanticCircuit * circuit = &model->circuits[index];
		printf("struct ");
		_emitStateTypeName(circuit->name);
		printf(" {\n");
		_emitSignalFields(circuit);
		_emitInstanceFields(circuit);
		printf("};\n\n");
	}
	for (size_t index = 0; index < model->circuitCount; ++index) {
		_emitCircuitFunctions(&model->circuits[index]);
	}
	_emitMain(model->topCircuit);
}
