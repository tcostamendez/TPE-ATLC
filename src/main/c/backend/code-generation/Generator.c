#include "Generator.h"

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

/** Shutdown module's internal state. */
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

void executeGenerator(CompilerState * compilerState) {
	if (compilerState->abstractSyntaxTree == NULL) {
		logDebugging(_logger, "Skipping code generation because the frontend did not produce an AST.");
		return;
	}

	logDebugging(_logger, "Skipping code generation: stage 2 stops after building the AST.");
}
