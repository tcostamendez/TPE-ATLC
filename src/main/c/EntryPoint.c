#include "backend/code-generation/Generator.h"
#include "backend/domain-specific/Calculator.h"
#include "frontend/Frontend.h"
#include "frontend/lexical-analysis/FlexActions.h"
#include "frontend/syntactic-analysis/BisonActions.h"
#include "support/logging/Logger.h"
#include "support/type/CompilationStatus.h"
#include "support/type/CompilerState.h"
#include "support/type/ModuleDestructor.h"
#include <string.h>

/**
 * The main entry-point of the compiler.
 */
const int main(const int length, const char ** arguments) {
	LexicalAnalyzer * lexicalAnalyzer = createLexicalAnalyzer();
	Logger * logger = createLogger("EntryPoint");
	if (lexicalAnalyzer == NULL) {
		logCritical(logger, "The compiler could not initialize its lexical analyzer.");
		destroyLogger(logger);
		return OUT_OF_MEMORY;
	}

	for (int k = 0; k < length; ++k) {
		logDebugging(logger, "Argument %d: \"%s\"", k, arguments[k]);
	}
	CompilerState compilerState = {
		.abstractSyntaxTree = NULL,
		.semanticModel = NULL,
		.topCircuitName = NULL
	};
	for (int k = 1; k < length; ++k) {
		if (strcmp(arguments[k], "--top") == 0 && k + 1 < length) {
			compilerState.topCircuitName = arguments[++k];
		}
		else {
			logError(logger, "Unknown argument: \"%s\".", arguments[k]);
			destroyLogger(logger);
			destroyLexicalAnalyzer(lexicalAnalyzer);
			return FAILED;
		}
	}
	ModuleDestructor moduleDestructors[] = {
		initializeAbstractSyntaxTreeModule(),
		initializeFlexActionsModule(lexicalAnalyzer),
		initializeBisonActionsModule(&compilerState),
		initializeFrontendModule(lexicalAnalyzer),
		initializeCalculatorModule(),
		initializeGeneratorModule()
	};
	CompilationStatus compilationStatus = executeSyntacticAnalysis();
	if (compilationStatus == SUCCEEDED) {
		CompilationStatus bisonStatus = getBisonActionsStatus();
		if (bisonStatus != SUCCEEDED) {
			compilationStatus = bisonStatus;
		}
	}

	Program * program = compilerState.abstractSyntaxTree;
	if (compilationStatus == SUCCEEDED && program != NULL) {
		logDebugging(logger, "Frontend accepted the input program.");
		ComputationResult computationResult = executeCalculator(&compilerState);
		if (computationResult.succeeded) {
			executeGenerator(&compilerState);
		}
		else {
			logError(logger, "Semantic analysis rejected the input program.");
			compilationStatus = FAILED;
		}
	}
	else {
		logError(logger, "The frontend rejects the input program.");
		if (compilationStatus == SUCCEEDED) {
			compilationStatus = FAILED;
		}
	}
	logDebugging(logger, "Releasing AST resources...");
	destroySemanticModel(compilerState.semanticModel);
	destroyProgram(program);
	for (int k = (sizeof(moduleDestructors)/sizeof(ModuleDestructor)) - 1; 0 <= k; --k) {
		moduleDestructors[k]();
	}
	logDebugging(logger, "Compilation is done.");
	destroyLogger(logger);
	destroyLexicalAnalyzer(lexicalAnalyzer);
	return compilationStatus;
}
