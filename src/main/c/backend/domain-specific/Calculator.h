#ifndef CALCULATOR_HEADER
#define CALCULATOR_HEADER

#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../../support/logging/Logger.h"
#include "../../support/type/CompilerState.h"
#include "../../support/type/ModuleDestructor.h"
#include <stdbool.h>

/** Initialize module's internal state. */
ModuleDestructor initializeCalculatorModule();

typedef struct SemanticSignal SemanticSignal;
typedef struct SemanticCircuit SemanticCircuit;
typedef struct SemanticModel SemanticModel;

struct SemanticSignal {
	char * name;
	DeclarationType type;
	bool combinationallyAssigned;
	bool sequentiallyAssigned;
	bool usedAsSource;
};

struct SemanticCircuit {
	Circuit * ast;
	char * name;
	SemanticSignal * signals;
	size_t signalCount;
	size_t inputCount;
	size_t outputCount;
	size_t wireCount;
	size_t regCount;
};

struct SemanticModel {
	SemanticCircuit * circuits;
	size_t circuitCount;
	SemanticCircuit * topCircuit;
};

typedef struct {
	bool succeeded;
} ComputationResult;

ComputationResult executeCalculator(CompilerState * compilerState);
void destroySemanticModel(SemanticModel * model);
SemanticCircuit * findSemanticCircuit(SemanticModel * model, const char * name);
SemanticSignal * findSemanticSignal(SemanticCircuit * circuit, const char * name);
const char * declarationTypeName(DeclarationType type);

#endif
