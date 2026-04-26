#ifndef BISON_ACTIONS_HEADER
#define BISON_ACTIONS_HEADER

#include "../../support/language/String.h"
#include "../../support/logging/Logger.h"
#include "../../support/type/CompilerState.h"
#include "../../support/type/ModuleDestructor.h"
#include "AbstractSyntaxTree.h"
#include "BisonParser.h"
#include <stdlib.h>

/** Initialize module's internal state. */
ModuleDestructor initializeBisonActionsModule(CompilerState * compilerState);

IdentifierList * IdentifierListSemanticAction(char * identifier);
IdentifierList * AppendIdentifierSemanticAction(IdentifierList * identifierList, char * identifier);
Connection * ConnectionSemanticAction(char * portName, char * signalName);
ConnectionList * ConnectionListSemanticAction(Connection * connection);
ConnectionList * AppendConnectionSemanticAction(ConnectionList * connectionList, Connection * connection);
Declaration * DeclarationSemanticAction(DeclarationType type, IdentifierList * identifiers);
Expression * IdentifierExpressionSemanticAction(char * identifier);
Expression * UnaryExpressionSemanticAction(UnaryOperatorType unaryOperator, Expression * operand);
Expression * BinaryExpressionSemanticAction(Expression * leftExpression, Expression * rightExpression, BinaryOperatorType binaryOperator);
SequentialAssignment * SequentialAssignmentSemanticAction(char * target, Expression * expression);
SequentialAssignmentList * SequentialAssignmentListSemanticAction(SequentialAssignment * assignment);
SequentialAssignmentList * AppendSequentialAssignmentSemanticAction(SequentialAssignmentList * assignmentList, SequentialAssignment * assignment);
ClockBlock * ClockBlockSemanticAction(char * clockSignal, SequentialAssignmentList * assignments);
Instance * InstanceSemanticAction(char * circuitName, ConnectionList * inputConnections, ConnectionList * outputConnections);
Statement * CombinationalAssignmentStatementSemanticAction(char * target, Expression * expression);
Statement * ClockBlockStatementSemanticAction(ClockBlock * clockBlock);
Statement * InstanceStatementSemanticAction(Instance * instance);
CircuitItem * DeclarationCircuitItemSemanticAction(Declaration * declaration);
CircuitItem * StatementCircuitItemSemanticAction(Statement * statement);
CircuitItemList * CircuitItemListSemanticAction(CircuitItem * item);
CircuitItemList * AppendCircuitItemSemanticAction(CircuitItemList * itemList, CircuitItem * item);
Circuit * CircuitSemanticAction(char * name, CircuitItemList * items);
CircuitList * CircuitListSemanticAction(Circuit * circuit);
CircuitList * AppendCircuitSemanticAction(CircuitList * circuitList, Circuit * circuit);
Program * ProgramSemanticAction(CircuitList * circuits);

#endif
