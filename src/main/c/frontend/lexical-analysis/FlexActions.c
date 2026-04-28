#include "FlexActions.h"

/* MODULE INTERNAL STATE */

static bool _logIgnoredLexemes = true;
static LexicalAnalyzer * _lexicalAnalyzer = NULL;
static Logger * _logger = NULL;

static YYLTYPE * _currentLocation() {
	return (YYLTYPE *) _lexicalAnalyzer->location;
}

static void _consumeCurrentLexeme() {
	YYLTYPE * location = _currentLocation();
	const char * lexeme = yyget_text(_lexicalAnalyzer->scanner);
	const unsigned int length = yyget_leng(_lexicalAnalyzer->scanner);
	unsigned int line = yyget_lineno(_lexicalAnalyzer->scanner);
	unsigned int column = _lexicalAnalyzer->column;
	unsigned int firstLine = line;
	unsigned int firstColumn = column;

	for (unsigned int i = 0; i < length; ++i) {
		if (lexeme[i] == '\n') {
			++line;
			column = 1;
		}
		else {
			++column;
		}
	}

	if (location != NULL) {
		location->first_line = firstLine;
		location->first_column = firstColumn;
		location->last_line = line;
		location->last_column = length == 0 ? firstColumn : (column == 1 ? 1 : column - 1);
	}

	_lexicalAnalyzer->column = column;
}

static void _logLexicalError(const char * reason, const char * lexeme) {
	YYLTYPE * location = _currentLocation();
	const int line = (location != NULL && 0 < location->first_line) ? location->first_line : 1;
	const int column = (location != NULL && 0 < location->first_column) ? location->first_column : 1;
	if (lexeme != NULL) {
		char * escapedLexeme = escape(lexeme);
		logError(_logger, "Frontend lexical error at line %d, column %d: %s near \"%s\".", line, column, reason, escapedLexeme != NULL ? escapedLexeme : "<unavailable>");
		free(escapedLexeme);
	}
	else {
		logError(_logger, "Frontend lexical error at line %d, column %d: %s.", line, column, reason);
	}
}

/** Shutdown module's internal state. */
void _shutdownFlexActionsModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: FlexActions...");
		destroyLogger(_logger);
		_logger = NULL;
	}
	_lexicalAnalyzer = NULL;
}

ModuleDestructor initializeFlexActionsModule(LexicalAnalyzer * lexicalAnalyzer) {
	_lexicalAnalyzer = lexicalAnalyzer;
	_logger = createLogger("FlexActions");
	_logIgnoredLexemes = getBooleanOrDefault("LOG_IGNORED_LEXEMES", _logIgnoredLexemes);
	return _shutdownFlexActionsModule;
}

static void _logTokenAction(const char * actionName, Token * token) {
	if (token == NULL) {
		logError(_logger, "%s could not log a token because allocation failed.", actionName);
		return;
	}

	char * escapedLexeme = escape(token->lexeme);
	logDebugging(_logger, WARNING_COLOR "%s" DEFAULT_COLOR ": Token(context=%d, label=%d, length=%d, lexeme=%s\"%s\"%s, line=%d, column=%d, semanticValue=%p)",
		actionName,
		token->context,
		token->label,
		token->length,
		INFORMATION_COLOR, escapedLexeme != NULL ? escapedLexeme : "<unavailable>", DEFAULT_COLOR,
		token->line,
		token->column,
		token->semanticValue);
	free(escapedLexeme);
}

static CompilationStatus _pushSimpleToken(TokenLabel label, const char * actionName) {
	_consumeCurrentLexeme();
	Token * token = createToken(_lexicalAnalyzer, label);
	if (token == NULL) {
		return OUT_OF_MEMORY;
	}
	_logTokenAction(actionName, token);
	CompilationStatus status = pushToken(_lexicalAnalyzer, token);
	destroyToken(token);
	return status;
}

CompilationStatus CommentLexemeAction(FlexContext context) {
	_consumeCurrentLexeme();
	if (_logIgnoredLexemes) {
		Token * token = createToken(_lexicalAnalyzer, IGNORED);
		if (token == NULL) {
			return OUT_OF_MEMORY;
		}
		_logTokenAction(__FUNCTION__, token);
		destroyToken(token);
	}
	enterLexicalAnalyzerContext(_lexicalAnalyzer, context);
	return IN_PROGRESS;
}

CompilationStatus CommentContentLexemeAction() {
	_consumeCurrentLexeme();
	if (_logIgnoredLexemes) {
		Token * token = createToken(_lexicalAnalyzer, IGNORED);
		if (token == NULL) {
			return OUT_OF_MEMORY;
		}
		_logTokenAction(__FUNCTION__, token);
		destroyToken(token);
	}
	return IN_PROGRESS;
}

CompilationStatus CommentEndLexemeAction() {
	_consumeCurrentLexeme();
	if (_logIgnoredLexemes) {
		Token * token = createToken(_lexicalAnalyzer, IGNORED);
		if (token == NULL) {
			return OUT_OF_MEMORY;
		}
		_logTokenAction(__FUNCTION__, token);
		destroyToken(token);
	}
	leaveLexicalAnalyzerContext(_lexicalAnalyzer);
	return IN_PROGRESS;
}

CompilationStatus EOFLexemeAction() {
	_consumeCurrentLexeme();
	FlexContext context = currentLexicalAnalyzerContext(_lexicalAnalyzer);
	if (0 < context) {
		_logLexicalError("unterminated multiline comment", NULL);
		return FAILED;
	}
	Token * token = createToken(_lexicalAnalyzer, 0);
	if (token == NULL) {
		return OUT_OF_MEMORY;
	}
	_logTokenAction(__FUNCTION__, token);
	CompilationStatus status = pushToken(_lexicalAnalyzer, token);
	destroyToken(token);
	return status;
}

CompilationStatus IdentifierLexemeAction() {
	_consumeCurrentLexeme();
	Token * token = createToken(_lexicalAnalyzer, IDENTIFIER);
	if (token == NULL) {
		return OUT_OF_MEMORY;
	}
	token->semanticValue->string = calloc(token->length + 1, sizeof(char));
	if (token->semanticValue->string == NULL) {
		destroyToken(token);
		logError(_logger, "The compiler ran out of memory while storing an identifier.");
		return OUT_OF_MEMORY;
	}
	strcpy(token->semanticValue->string, token->lexeme);
	_logTokenAction(__FUNCTION__, token);
	CompilationStatus status = pushToken(_lexicalAnalyzer, token);
	destroyToken(token);
	return status;
}

CompilationStatus IgnoredLexemeAction() {
	_consumeCurrentLexeme();
	if (_logIgnoredLexemes) {
		Token * token = createToken(_lexicalAnalyzer, IGNORED);
		if (token == NULL) {
			return OUT_OF_MEMORY;
		}
		_logTokenAction(__FUNCTION__, token);
		destroyToken(token);
	}
	return IN_PROGRESS;
}

CompilationStatus SymbolLexemeAction(TokenLabel label) {
	return _pushSimpleToken(label, __FUNCTION__);
}

CompilationStatus UnknownLexemeAction() {
	_consumeCurrentLexeme();
	Token * token = createToken(_lexicalAnalyzer, UNKNOWN);
	if (token == NULL) {
		return OUT_OF_MEMORY;
	}
	_logTokenAction(__FUNCTION__, token);
	_logLexicalError("unexpected lexeme", token->lexeme);
	destroyToken(token);
	return FAILED;
}
