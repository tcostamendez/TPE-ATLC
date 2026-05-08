#include "backend/code-generation/Generator.h"
#include "backend/domain-specific/Calculator.h"
#include "frontend/Frontend.h"
#include "frontend/lexical-analysis/FlexActions.h"
#include "frontend/syntactic-analysis/BisonActions.h"
#include "support/logging/Logger.h"
#include "support/type/CompilationStatus.h"
#include "support/type/CompilerState.h"
#include "support/type/ModuleDestructor.h"

/**
 * The main entry-point of the entire application. If you use "strtok" to
 * parse anything inside this project instead of using Flex and Bison, I will
 * find you, and I will kill you (Bryan Mills; "Taken", 2008).
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
		.abstractSyntaxTree = NULL
	};
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
		logDebugging(logger, "Stage 2 ends after AST construction; semantic analysis belongs to stage 3.");
		ComputationResult computationResult = executeCalculator(&compilerState);
		if (computationResult.succeeded) {
			executeGenerator(&compilerState);
		}
		else {
			logError(logger, "The stage 2 stubs could not confirm the AST hand-off.");
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
	destroyProgram(program);
	for (int k = (sizeof(moduleDestructors)/sizeof(ModuleDestructor)) - 1; 0 <= k; --k) {
		moduleDestructors[k]();
	}
	logDebugging(logger, "Compilation is done.");
	destroyLogger(logger);
	destroyLexicalAnalyzer(lexicalAnalyzer);
	return compilationStatus;
}
