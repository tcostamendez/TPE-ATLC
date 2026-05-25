#include "Calculator.h"
#include <string.h>

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

typedef struct {
	bool ok;
	bool * dependencies;
	size_t signalCount;
	SemanticCircuit * circuit;
} DependencyGraph;

static bool _same(const char * left, const char * right) {
	return left != NULL && right != NULL && strcmp(left, right) == 0;
}

static char * _copy(const char * text) {
	if (text == NULL) {
		return NULL;
	}
	char * copy = calloc(strlen(text) + 1, sizeof(char));
	if (copy != NULL) {
		strcpy(copy, text);
	}
	return copy;
}

const char * declarationTypeName(DeclarationType type) {
	switch (type) {
		case INPUT_DECLARATION: return "input";
		case OUTPUT_DECLARATION: return "output";
		case WIRE_DECLARATION: return "wire";
		case REG_DECLARATION: return "reg";
		default: return "unknown";
	}
}

void _shutdownCalculatorModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: Calculator...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeCalculatorModule() {
	_logger = createLogger("SemanticAnalyzer");
	return _shutdownCalculatorModule;
}

SemanticCircuit * findSemanticCircuit(SemanticModel * model, const char * name) {
	if (model == NULL || name == NULL) {
		return NULL;
	}
	for (size_t index = 0; index < model->circuitCount; ++index) {
		if (_same(model->circuits[index].name, name)) {
			return &model->circuits[index];
		}
	}
	return NULL;
}

SemanticSignal * findSemanticSignal(SemanticCircuit * circuit, const char * name) {
	if (circuit == NULL || name == NULL) {
		return NULL;
	}
	for (size_t index = 0; index < circuit->signalCount; ++index) {
		if (_same(circuit->signals[index].name, name)) {
			return &circuit->signals[index];
		}
	}
	return NULL;
}

static size_t _signalIndex(SemanticCircuit * circuit, const char * name) {
	for (size_t index = 0; index < circuit->signalCount; ++index) {
		if (_same(circuit->signals[index].name, name)) {
			return index;
		}
	}
	return (size_t) -1;
}

static size_t _circuitIndex(SemanticModel * model, const char * name) {
	for (size_t index = 0; index < model->circuitCount; ++index) {
		if (_same(model->circuits[index].name, name)) {
			return index;
		}
	}
	return (size_t) -1;
}

static bool _appendSignal(SemanticCircuit * circuit, const char * name, DeclarationType type) {
	if (findSemanticSignal(circuit, name) != NULL) {
		logError(_logger, "Semantic error in circuit '%s': signal '%s' is redeclared.", circuit->name, name);
		return false;
	}

	SemanticSignal * resized = realloc(circuit->signals, sizeof(SemanticSignal) * (circuit->signalCount + 1));
	if (resized == NULL) {
		logError(_logger, "Semantic analyzer ran out of memory while indexing signals.");
		return false;
	}

	circuit->signals = resized;
	SemanticSignal * signal = &circuit->signals[circuit->signalCount++];
	signal->name = _copy(name);
	signal->type = type;
	signal->combinationallyAssigned = false;
	signal->sequentiallyAssigned = false;
	if (signal->name == NULL) {
		logError(_logger, "Semantic analyzer ran out of memory while copying a signal name.");
		return false;
	}

	switch (type) {
		case INPUT_DECLARATION: ++circuit->inputCount; break;
		case OUTPUT_DECLARATION: ++circuit->outputCount; break;
		case WIRE_DECLARATION: ++circuit->wireCount; break;
		case REG_DECLARATION: ++circuit->regCount; break;
	}
	return true;
}

static bool _forEachExpressionIdentifier(Expression * expression, bool (* visitor)(const char *, void *), void * context) {
	if (expression == NULL) {
		return true;
	}
	switch (expression->type) {
		case IDENTIFIER_EXPRESSION:
			return visitor(expression->identifier, context);
		case UNARY_EXPRESSION:
			return _forEachExpressionIdentifier(expression->operand, visitor, context);
		case BINARY_EXPRESSION:
			return _forEachExpressionIdentifier(expression->leftExpression, visitor, context)
				&& _forEachExpressionIdentifier(expression->rightExpression, visitor, context);
	}
	return true;
}

static bool _validateExpressionIdentifier(const char * identifier, void * context) {
	SemanticCircuit * circuit = context;
	if (findSemanticSignal(circuit, identifier) == NULL) {
		logError(_logger, "Semantic error in circuit '%s': signal '%s' is used but not declared.", circuit->name, identifier);
		return false;
	}
	return true;
}

static bool _connectionPortExists(SemanticCircuit * child, DeclarationType type, const char * portName) {
	SemanticSignal * signal = findSemanticSignal(child, portName);
	return signal != NULL && signal->type == type;
}

static bool _hasDuplicateConnection(ConnectionList * list, const char * portName) {
	int count = 0;
	for (ConnectionList * node = list; node != NULL; node = node->next) {
		if (_same(node->connection->portName, portName)) {
			++count;
		}
	}
	return 1 < count;
}

static bool _validateConnectionSet(SemanticCircuit * parent, SemanticCircuit * child, ConnectionList * list, DeclarationType portType) {
	bool ok = true;
	size_t expected = portType == INPUT_DECLARATION ? child->inputCount : child->outputCount;
	size_t actual = 0;
	for (ConnectionList * node = list; node != NULL; node = node->next) {
		Connection * connection = node->connection;
		++actual;
		if (_hasDuplicateConnection(list, connection->portName)) {
			logError(_logger, "Semantic error in circuit '%s': port '%s' is connected more than once in instance of '%s'.", parent->name, connection->portName, child->name);
			ok = false;
		}
		if (!_connectionPortExists(child, portType, connection->portName)) {
			logError(_logger, "Semantic error in circuit '%s': '%s' is not a valid %s port of '%s'.", parent->name, connection->portName, declarationTypeName(portType), child->name);
			ok = false;
		}
		SemanticSignal * parentSignal = findSemanticSignal(parent, connection->signalName);
		if (parentSignal == NULL) {
			logError(_logger, "Semantic error in circuit '%s': instance connection uses undeclared signal '%s'.", parent->name, connection->signalName);
			ok = false;
		}
		else if (portType == OUTPUT_DECLARATION) {
			if (parentSignal->type == INPUT_DECLARATION || parentSignal->type == REG_DECLARATION) {
				logError(_logger, "Semantic error in circuit '%s': instance output cannot drive %s signal '%s'.", parent->name, declarationTypeName(parentSignal->type), parentSignal->name);
				ok = false;
			}
			if (parentSignal->combinationallyAssigned) {
				logError(_logger, "Semantic error in circuit '%s': signal '%s' has multiple combinational definitions.", parent->name, parentSignal->name);
				ok = false;
			}
			parentSignal->combinationallyAssigned = true;
		}
	}
	if (actual != expected) {
		logError(_logger, "Semantic error in circuit '%s': instance of '%s' connects %zu %s ports but %zu are required.", parent->name, child->name, actual, declarationTypeName(portType), expected);
		ok = false;
	}
	return ok;
}

static bool _addDependency(const char * identifier, void * context) {
	DependencyGraph * graph = context;
	SemanticSignal * dependency = findSemanticSignal(graph->circuit, identifier);
	if (dependency == NULL) {
		graph->ok = false;
		return false;
	}
	size_t source = _signalIndex(graph->circuit, identifier);
	size_t target = graph->signalCount;
	graph->dependencies[target * graph->circuit->signalCount + source] = true;
	return true;
}

static bool _detectCycleFrom(DependencyGraph * graph, size_t node, bool * visiting, bool * visited) {
	if (visiting[node]) {
		return true;
	}
	if (visited[node]) {
		return false;
	}
	visiting[node] = true;
	for (size_t next = 0; next < graph->circuit->signalCount; ++next) {
		SemanticSignal * signal = &graph->circuit->signals[next];
		if (!graph->dependencies[node * graph->circuit->signalCount + next]) {
			continue;
		}
		if (signal->type == INPUT_DECLARATION || signal->type == REG_DECLARATION) {
			continue;
		}
		if (_detectCycleFrom(graph, next, visiting, visited)) {
			return true;
		}
	}
	visiting[node] = false;
	visited[node] = true;
	return false;
}

static bool _validateCircuit(SemanticModel * model, SemanticCircuit * circuit) {
	bool ok = true;
	bool * dependencies = calloc(circuit->signalCount * circuit->signalCount, sizeof(bool));
	if (dependencies == NULL && circuit->signalCount != 0) {
		logError(_logger, "Semantic analyzer ran out of memory while preparing dependency checks.");
		return false;
	}
	DependencyGraph graph = {
		.ok = true,
		.dependencies = dependencies,
		.signalCount = 0,
		.circuit = circuit
	};

	for (CircuitItemList * itemNode = circuit->ast->items; itemNode != NULL; itemNode = itemNode->next) {
		CircuitItem * item = itemNode->item;
		if (item->type != STATEMENT_ITEM) {
			continue;
		}
		Statement * statement = item->statement;
		if (statement->type == COMBINATIONAL_ASSIGNMENT_STATEMENT) {
			CombinationalAssignment * assignment = statement->combinationalAssignment;
			SemanticSignal * target = findSemanticSignal(circuit, assignment->target);
			if (target == NULL) {
				logError(_logger, "Semantic error in circuit '%s': assignment target '%s' is not declared.", circuit->name, assignment->target);
				ok = false;
			}
			else {
				if (target->type == INPUT_DECLARATION || target->type == REG_DECLARATION) {
					logError(_logger, "Semantic error in circuit '%s': %s signal '%s' cannot be assigned with '='.", circuit->name, declarationTypeName(target->type), target->name);
					ok = false;
				}
				if (target->combinationallyAssigned) {
					logError(_logger, "Semantic error in circuit '%s': signal '%s' has multiple combinational definitions.", circuit->name, target->name);
					ok = false;
				}
				target->combinationallyAssigned = true;
				graph.signalCount = _signalIndex(circuit, target->name);
				_forEachExpressionIdentifier(assignment->expression, _addDependency, &graph);
			}
			ok = _forEachExpressionIdentifier(assignment->expression, _validateExpressionIdentifier, circuit) && ok;
		}
		else if (statement->type == CLOCK_BLOCK_STATEMENT) {
			ClockBlock * clockBlock = statement->clockBlock;
			SemanticSignal * clock = findSemanticSignal(circuit, clockBlock->clockSignal);
			if (clock == NULL || clock->type != INPUT_DECLARATION) {
				logError(_logger, "Semantic error in circuit '%s': clock '%s' must be a declared input.", circuit->name, clockBlock->clockSignal);
				ok = false;
			}
			for (SequentialAssignmentList * assignmentNode = clockBlock->assignments; assignmentNode != NULL; assignmentNode = assignmentNode->next) {
				SequentialAssignment * assignment = assignmentNode->assignment;
				SemanticSignal * target = findSemanticSignal(circuit, assignment->target);
				if (target == NULL) {
					logError(_logger, "Semantic error in circuit '%s': sequential target '%s' is not declared.", circuit->name, assignment->target);
					ok = false;
				}
				else {
					if (target->type != REG_DECLARATION) {
						logError(_logger, "Semantic error in circuit '%s': %s signal '%s' cannot be assigned with '<='.", circuit->name, declarationTypeName(target->type), target->name);
						ok = false;
					}
					if (target->sequentiallyAssigned) {
						logError(_logger, "Semantic error in circuit '%s': register '%s' has multiple sequential definitions.", circuit->name, target->name);
						ok = false;
					}
					target->sequentiallyAssigned = true;
				}
				ok = _forEachExpressionIdentifier(assignment->expression, _validateExpressionIdentifier, circuit) && ok;
			}
		}
		else if (statement->type == INSTANCE_STATEMENT) {
			Instance * instance = statement->instance;
			SemanticCircuit * child = findSemanticCircuit(model, instance->circuitName);
			if (child == NULL) {
				logError(_logger, "Semantic error in circuit '%s': instance references unknown circuit '%s'.", circuit->name, instance->circuitName);
				ok = false;
				continue;
			}
			ok = _validateConnectionSet(circuit, child, instance->inputConnections, INPUT_DECLARATION) && ok;
			ok = _validateConnectionSet(circuit, child, instance->outputConnections, OUTPUT_DECLARATION) && ok;
			for (ConnectionList * outputNode = instance->outputConnections; outputNode != NULL; outputNode = outputNode->next) {
				size_t target = _signalIndex(circuit, outputNode->connection->signalName);
				if (target == (size_t) -1) {
					continue;
				}
				graph.signalCount = target;
				for (ConnectionList * inputNode = instance->inputConnections; inputNode != NULL; inputNode = inputNode->next) {
					_addDependency(inputNode->connection->signalName, &graph);
				}
			}
		}
	}

	for (size_t index = 0; index < circuit->signalCount; ++index) {
		SemanticSignal * signal = &circuit->signals[index];
		if (signal->type == OUTPUT_DECLARATION && !signal->combinationallyAssigned) {
			logError(_logger, "Semantic error in circuit '%s': output '%s' is not determined.", circuit->name, signal->name);
			ok = false;
		}
	}

	bool * visiting = calloc(circuit->signalCount, sizeof(bool));
	bool * visited = calloc(circuit->signalCount, sizeof(bool));
	if ((visiting == NULL || visited == NULL) && circuit->signalCount != 0) {
		logError(_logger, "Semantic analyzer ran out of memory while checking cycles.");
		ok = false;
	}
	else {
		for (size_t index = 0; index < circuit->signalCount; ++index) {
			SemanticSignal * signal = &circuit->signals[index];
			if ((signal->type == WIRE_DECLARATION || signal->type == OUTPUT_DECLARATION) && _detectCycleFrom(&graph, index, visiting, visited)) {
				logError(_logger, "Semantic error in circuit '%s': combinational cycle detected around signal '%s'.", circuit->name, signal->name);
				ok = false;
				break;
			}
		}
	}

	free(visiting);
	free(visited);
	free(dependencies);
	return ok && graph.ok;
}

static bool _detectInstanceCycleFrom(SemanticModel * model, size_t circuitIndex, bool * visiting, bool * visited) {
	if (visiting[circuitIndex]) {
		return true;
	}
	if (visited[circuitIndex]) {
		return false;
	}
	visiting[circuitIndex] = true;
	SemanticCircuit * circuit = &model->circuits[circuitIndex];
	for (CircuitItemList * node = circuit->ast->items; node != NULL; node = node->next) {
		if (node->item->type != STATEMENT_ITEM || node->item->statement->type != INSTANCE_STATEMENT) {
			continue;
		}
		size_t childIndex = _circuitIndex(model, node->item->statement->instance->circuitName);
		if (childIndex == (size_t) -1) {
			continue;
		}
		if (_detectInstanceCycleFrom(model, childIndex, visiting, visited)) {
			return true;
		}
	}
	visiting[circuitIndex] = false;
	visited[circuitIndex] = true;
	return false;
}

static bool _validateInstanceAcyclicity(SemanticModel * model) {
	bool ok = true;
	bool * visiting = calloc(model->circuitCount, sizeof(bool));
	bool * visited = calloc(model->circuitCount, sizeof(bool));
	if ((visiting == NULL || visited == NULL) && model->circuitCount != 0) {
		free(visiting);
		free(visited);
		logError(_logger, "Semantic analyzer ran out of memory while checking instance cycles.");
		return false;
	}
	for (size_t index = 0; index < model->circuitCount; ++index) {
		if (_detectInstanceCycleFrom(model, index, visiting, visited)) {
			logError(_logger, "Semantic error: recursive circuit instantiation detected around '%s'.", model->circuits[index].name);
			ok = false;
			break;
		}
	}
	free(visiting);
	free(visited);
	return ok;
}

void destroySemanticModel(SemanticModel * model) {
	if (model == NULL) {
		return;
	}
	for (size_t circuitIndex = 0; circuitIndex < model->circuitCount; ++circuitIndex) {
		SemanticCircuit * circuit = &model->circuits[circuitIndex];
		free(circuit->name);
		for (size_t signalIndex = 0; signalIndex < circuit->signalCount; ++signalIndex) {
			free(circuit->signals[signalIndex].name);
		}
		free(circuit->signals);
	}
	free(model->circuits);
	free(model);
}

static SemanticModel * _buildModel(Program * program) {
	SemanticModel * model = calloc(1, sizeof(SemanticModel));
	if (model == NULL) {
		return NULL;
	}

	for (CircuitList * node = program->circuits; node != NULL; node = node->next) {
		if (findSemanticCircuit(model, node->circuit->name) != NULL) {
			logError(_logger, "Semantic error: circuit '%s' is redeclared.", node->circuit->name);
			destroySemanticModel(model);
			return NULL;
		}
		SemanticCircuit * resized = realloc(model->circuits, sizeof(SemanticCircuit) * (model->circuitCount + 1));
		if (resized == NULL) {
			destroySemanticModel(model);
			return NULL;
		}
		model->circuits = resized;
		SemanticCircuit * circuit = &model->circuits[model->circuitCount++];
		memset(circuit, 0, sizeof(SemanticCircuit));
		circuit->ast = node->circuit;
		circuit->name = _copy(node->circuit->name);
		if (circuit->name == NULL) {
			destroySemanticModel(model);
			return NULL;
		}
	}

	for (size_t circuitIndex = 0; circuitIndex < model->circuitCount; ++circuitIndex) {
		SemanticCircuit * circuit = &model->circuits[circuitIndex];
		for (CircuitItemList * itemNode = circuit->ast->items; itemNode != NULL; itemNode = itemNode->next) {
			CircuitItem * item = itemNode->item;
			if (item->type != DECLARATION_ITEM) {
				continue;
			}
			Declaration * declaration = item->declaration;
			for (IdentifierList * identifierNode = declaration->identifiers; identifierNode != NULL; identifierNode = identifierNode->next) {
				if (!_appendSignal(circuit, identifierNode->identifier, declaration->type)) {
					destroySemanticModel(model);
					return NULL;
				}
			}
		}
	}
	return model;
}

ComputationResult executeCalculator(CompilerState * compilerState) {
	if (compilerState == NULL || compilerState->abstractSyntaxTree == NULL) {
		logDebugging(_logger, "Skipping semantic analysis because the frontend did not produce an AST.");
		return (ComputationResult) { .succeeded = false };
	}

	destroySemanticModel(compilerState->semanticModel);
	compilerState->semanticModel = _buildModel(compilerState->abstractSyntaxTree);
	if (compilerState->semanticModel == NULL) {
		return (ComputationResult) { .succeeded = false };
	}

	bool ok = true;
	for (size_t index = 0; index < compilerState->semanticModel->circuitCount; ++index) {
		ok = _validateCircuit(compilerState->semanticModel, &compilerState->semanticModel->circuits[index]) && ok;
	}
	ok = _validateInstanceAcyclicity(compilerState->semanticModel) && ok;

	if (compilerState->topCircuitName != NULL) {
		compilerState->semanticModel->topCircuit = findSemanticCircuit(compilerState->semanticModel, compilerState->topCircuitName);
		if (compilerState->semanticModel->topCircuit == NULL) {
			logError(_logger, "Semantic error: top circuit '%s' does not exist.", compilerState->topCircuitName);
			ok = false;
		}
	}
	else if (compilerState->semanticModel->circuitCount != 0) {
		compilerState->semanticModel->topCircuit = &compilerState->semanticModel->circuits[compilerState->semanticModel->circuitCount - 1];
	}

	if (ok) {
		logDebugging(_logger, "Semantic analysis accepted the input program.");
	}
	return (ComputationResult) { .succeeded = ok };
}
