#ifndef CALCULATOR_HEADER
#define CALCULATOR_HEADER

#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../../support/logging/Logger.h"
#include "../../support/type/CompilerState.h"
#include "../../support/type/ModuleDestructor.h"
#include <stdbool.h>

/** Initialize module's internal state. */
ModuleDestructor initializeCalculatorModule();

typedef struct {
	bool succeeded;
} ComputationResult;

ComputationResult executeCalculator(CompilerState * compilerState);

#endif
