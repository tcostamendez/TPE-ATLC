%top{

#include "../../support/type/CompilationStatus.h"
#include "../../support/type/FlexContext.h"
#include "../../support/type/TokenLabel.h"
#include "../syntactic-analysis/AbstractSyntaxTree.h"
#include "../syntactic-analysis/BisonParser.h"

}

%{

extern CompilationStatus CommentContentLexemeAction();
extern CompilationStatus CommentEndLexemeAction();
extern CompilationStatus CommentLexemeAction(FlexContext context);
extern CompilationStatus EOFLexemeAction();
extern CompilationStatus IdentifierLexemeAction();
extern CompilationStatus IgnoredLexemeAction();
extern CompilationStatus SymbolLexemeAction(TokenLabel label);
extern CompilationStatus UnknownLexemeAction();

%}

%x MULTILINE_COMMENT

whitespace			[[:space:]]+
identifier			[_[:alpha:]][_[:alnum:]]*

%%

"/*"								{ return CommentLexemeAction(MULTILINE_COMMENT); }
<MULTILINE_COMMENT>"*/"				{ return CommentEndLexemeAction(); }
<MULTILINE_COMMENT>[^*]+			{ return CommentContentLexemeAction(); }
<MULTILINE_COMMENT>.				{ return CommentContentLexemeAction(); }

"circuit"							{ return SymbolLexemeAction(CIRCUIT); }
"input"								{ return SymbolLexemeAction(INPUT); }
"output"							{ return SymbolLexemeAction(OUTPUT); }
"wire"								{ return SymbolLexemeAction(WIRE); }
"reg"								{ return SymbolLexemeAction(REG); }
"on"								{ return SymbolLexemeAction(ON); }
"rising_edge"						{ return SymbolLexemeAction(RISING_EDGE); }
"and"								{ return SymbolLexemeAction(AND); }
"or"								{ return SymbolLexemeAction(OR); }
"xor"								{ return SymbolLexemeAction(XOR); }
"not"								{ return SymbolLexemeAction(NOT); }

"<="								{ return SymbolLexemeAction(NEXT_ASSIGN); }
"->"								{ return SymbolLexemeAction(ARROW); }
"="									{ return SymbolLexemeAction(ASSIGN); }
"{"									{ return SymbolLexemeAction(OPEN_BRACE); }
"}"									{ return SymbolLexemeAction(CLOSE_BRACE); }
"("									{ return SymbolLexemeAction(OPEN_PARENTHESIS); }
")"									{ return SymbolLexemeAction(CLOSE_PARENTHESIS); }
","									{ return SymbolLexemeAction(COMMA); }
";"									{ return SymbolLexemeAction(SEMICOLON); }

{identifier}							{ return IdentifierLexemeAction(); }
{whitespace}							{ return IgnoredLexemeAction(); }
.										{ return UnknownLexemeAction(); }

<<EOF>>									{ return EOFLexemeAction(); }

%%

#include "FlexActions.h"
#include "FlexExport.h"
