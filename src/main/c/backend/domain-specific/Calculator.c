#include "Calculator.h"

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

/** Shutdown module's internal state. */
void _shutdownCalculatorModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: Calculator...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeCalculatorModule() {
	_logger = createLogger("Calculator");
	return _shutdownCalculatorModule;
}

ComputationResult executeCalculator(CompilerState * compilerState) {
	if (compilerState->abstractSyntaxTree == NULL) {
		logDebugging(_logger, "Skipping semantic evaluation because the frontend did not produce an AST.");
		return (ComputationResult) { .succeeded = false };
	}

	logDebugging(_logger, "Skipping semantic evaluation: stage 2 stops after building the AST.");
	return (ComputationResult) { .succeeded = true };
}
