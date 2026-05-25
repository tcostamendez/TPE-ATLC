#include "Generator.h"
#include "../domain-specific/Calculator.h"

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

static void _emitExpression(Expression * expression);
static void _emitCircuitFunctions(SemanticCircuit * circuit);

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
		printf("\tint %s;\n", circuit->signals[index].name);
		if (circuit->signals[index].type == INPUT_DECLARATION) {
			printf("\tint prev_%s;\n", circuit->signals[index].name);
		}
	}
}

static void _emitInstanceFields(SemanticCircuit * circuit) {
	size_t index = 0;
	for (CircuitItemList * node = circuit->ast->items; node != NULL; node = node->next) {
		if (node->item->type == STATEMENT_ITEM && node->item->statement->type == INSTANCE_STATEMENT) {
			printf("\tState_%s * inst_%zu;\n", node->item->statement->instance->circuitName, index++);
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
			printf("(state->%s)", expression->identifier);
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
		printf("\tstate->inst_%zu->%s = state->%s;\n", index, node->connection->portName, node->connection->signalName);
	}
	printf("\tsettle_%s(state->inst_%zu);\n", instance->circuitName, index);
	for (ConnectionList * node = instance->outputConnections; node != NULL; node = node->next) {
		printf("\tstate->%s = state->inst_%zu->%s;\n", node->connection->signalName, index, node->connection->portName);
	}
}

static void _emitInstanceTick(Instance * instance, size_t index) {
	printf("\ttick_%s(state->inst_%zu", instance->circuitName, index);
	for (ConnectionList * node = instance->inputConnections; node != NULL; node = node->next) {
		printf(", state->%s", node->connection->signalName);
	}
	printf(");\n");
	for (ConnectionList * node = instance->outputConnections; node != NULL; node = node->next) {
		printf("\tstate->%s = state->inst_%zu->%s;\n", node->connection->signalName, index, node->connection->portName);
	}
}

static void _emitInitFunction(SemanticCircuit * circuit) {
	printf("static void init_%s(State_%s * state) {\n", circuit->name, circuit->name);
	printf("\tmemset(state, 0, sizeof(*state));\n");
	size_t index = 0;
	for (CircuitItemList * node = circuit->ast->items; node != NULL; node = node->next) {
		if (node->item->type == STATEMENT_ITEM && node->item->statement->type == INSTANCE_STATEMENT) {
			Instance * instance = node->item->statement->instance;
			printf("\tstate->inst_%zu = calloc(1, sizeof(State_%s));\n", index, instance->circuitName);
			printf("\tif (state->inst_%zu == NULL) { fprintf(stderr, \"runtime allocation failed\\n\"); exit(2); }\n", index);
			printf("\tinit_%s(state->inst_%zu);\n", instance->circuitName, index);
			++index;
		}
	}
	printf("}\n\n");
}

static void _emitFreeFunction(SemanticCircuit * circuit) {
	printf("static void free_%s(State_%s * state) {\n", circuit->name, circuit->name);
	size_t index = 0;
	for (CircuitItemList * node = circuit->ast->items; node != NULL; node = node->next) {
		if (node->item->type == STATEMENT_ITEM && node->item->statement->type == INSTANCE_STATEMENT) {
			Instance * instance = node->item->statement->instance;
			printf("\tif (state->inst_%zu != NULL) { free_%s(state->inst_%zu); free(state->inst_%zu); }\n", index, instance->circuitName, index, index);
			++index;
		}
	}
	printf("}\n\n");
}

static void _emitSettleFunction(SemanticCircuit * circuit) {
	printf("static void settle_%s(State_%s * state) {\n", circuit->name, circuit->name);
	printf("\tfor (int __settle_iter = 0; __settle_iter < %zu; ++__settle_iter) {\n", circuit->signalCount + 1);
	for (CircuitItemList * node = circuit->ast->items; node != NULL; node = node->next) {
		if (node->item->type != STATEMENT_ITEM) {
			continue;
		}
		Statement * statement = node->item->statement;
		if (statement->type == COMBINATIONAL_ASSIGNMENT_STATEMENT) {
			printf("\t\tstate->%s = ", statement->combinationalAssignment->target);
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
	printf("static void tick_%s(State_%s * state", circuit->name, circuit->name);
	for (size_t index = 0; index < circuit->signalCount; ++index) {
		if (circuit->signals[index].type == INPUT_DECLARATION) {
			printf(", int in_%s", circuit->signals[index].name);
		}
	}
	printf(") {\n");
	for (size_t index = 0; index < circuit->signalCount; ++index) {
		if (circuit->signals[index].type == INPUT_DECLARATION) {
			printf("\tstate->%s = in_%s ? 1 : 0;\n", circuit->signals[index].name, circuit->signals[index].name);
		}
	}
	printf("\tsettle_%s(state);\n", circuit->name);
	for (CircuitItemList * node = circuit->ast->items; node != NULL; node = node->next) {
		if (node->item->type != STATEMENT_ITEM || node->item->statement->type != CLOCK_BLOCK_STATEMENT) {
			continue;
		}
		ClockBlock * block = node->item->statement->clockBlock;
		if (block->edgeType == RISING_EDGE_EVENT) {
			printf("\tif (!state->prev_%s && state->%s) {\n", block->clockSignal, block->clockSignal);
		}
		else {
			printf("\tif (state->prev_%s && !state->%s) {\n", block->clockSignal, block->clockSignal);
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
			printf("\t\tstate->%s = next_%zu;\n", assignmentNode->assignment->target, assignmentIndex++);
		}
		printf("\t}\n");
	}
	for (CircuitItemList * node = circuit->ast->items; node != NULL; node = node->next) {
		if (node->item->type == STATEMENT_ITEM && node->item->statement->type == INSTANCE_STATEMENT) {
			_emitInstanceTick(node->item->statement->instance, _instanceIndex(circuit->ast->items, node->item->statement));
		}
	}
	printf("\tsettle_%s(state);\n", circuit->name);
	for (size_t index = 0; index < circuit->signalCount; ++index) {
		if (circuit->signals[index].type == INPUT_DECLARATION) {
			printf("\tstate->prev_%s = state->%s;\n", circuit->signals[index].name, circuit->signals[index].name);
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
	printf("\tState_%s state;\n", top->name);
	printf("\tinit_%s(&state);\n", top->name);
	printf("\tint cycles = 0;\n");
	printf("\tif (scanf(\"%%d\", &cycles) != 1) { free_%s(&state); return 1; }\n", top->name);
	printf("\tfor (int cycle = 0; cycle < cycles; ++cycle) {\n");
	for (size_t index = 0; index < top->signalCount; ++index) {
		if (top->signals[index].type == INPUT_DECLARATION) {
			printf("\t\tint in_%s = 0;\n", top->signals[index].name);
		}
	}
	for (size_t index = 0; index < top->signalCount; ++index) {
		if (top->signals[index].type == INPUT_DECLARATION) {
			printf("\t\tif (scanf(\"%%d\", &in_%s) != 1) { free_%s(&state); return 1; }\n", top->signals[index].name, top->name);
		}
	}
	printf("\t\ttick_%s(&state", top->name);
	for (size_t index = 0; index < top->signalCount; ++index) {
		if (top->signals[index].type == INPUT_DECLARATION) {
			printf(", in_%s", top->signals[index].name);
		}
	}
	printf(");\n");
	bool first = true;
	for (size_t index = 0; index < top->signalCount; ++index) {
		if (top->signals[index].type != OUTPUT_DECLARATION) {
			continue;
		}
		printf("\t\tprintf(\"%s%%d\", state.%s);\n", first ? "" : " ", top->signals[index].name);
		first = false;
	}
	printf("\t\tprintf(\"\\n\");\n");
	printf("\t}\n");
	printf("\tfree_%s(&state);\n", top->name);
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
		printf("typedef struct State_%s State_%s;\n", model->circuits[index].name, model->circuits[index].name);
	}
	printf("\n");
	for (size_t index = 0; index < model->circuitCount; ++index) {
		printf("static void init_%s(State_%s * state);\n", model->circuits[index].name, model->circuits[index].name);
		printf("static void free_%s(State_%s * state);\n", model->circuits[index].name, model->circuits[index].name);
		printf("static void settle_%s(State_%s * state);\n", model->circuits[index].name, model->circuits[index].name);
		printf("static void tick_%s(State_%s * state", model->circuits[index].name, model->circuits[index].name);
		for (size_t signalIndex = 0; signalIndex < model->circuits[index].signalCount; ++signalIndex) {
			if (model->circuits[index].signals[signalIndex].type == INPUT_DECLARATION) {
				printf(", int in_%s", model->circuits[index].signals[signalIndex].name);
			}
		}
		printf(");\n");
	}
	printf("\n");
	for (size_t index = 0; index < model->circuitCount; ++index) {
		SemanticCircuit * circuit = &model->circuits[index];
		printf("struct State_%s {\n", circuit->name);
		_emitSignalFields(circuit);
		_emitInstanceFields(circuit);
		printf("};\n\n");
	}
	for (size_t index = 0; index < model->circuitCount; ++index) {
		_emitCircuitFunctions(&model->circuits[index]);
	}
	_emitMain(model->topCircuit);
}
