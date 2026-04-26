# Stage 2 Frontend Notes

## Deliverable Scope

This repository snapshot closes Stage 2 of the ATLC project. The implemented scope is:

* Flex-based lexical analysis
* Bison-based syntactic analysis
* AST construction for the circuit DSL
* acceptance/rejection tests focused on frontend behaviour

Semantic analysis is intentionally out of scope for this stage and is deferred to Stage 3.

## Implemented DSL Slice

The current frontend accepts:

* `circuit` module definitions
* signal declarations with `input`, `output`, `wire`, `reg`
* combinational assignments with `=`
* sequential blocks written as `on rising_edge(clk) { ... }`
* sequential assignments with `<=`
* boolean expressions with `not`, `and`, `xor`, `or`
* subcircuit instantiations with named input/output connections

Programs may contain multiple circuits in the same input.

## AST Coverage

The AST models:

* program roots with multiple circuits
* circuit definitions
* declaration items
* statement items
* combinational assignments
* clocked blocks
* sequential assignments
* boolean expressions
* instances with named input/output connections

Lists are represented as linked-list nodes to stay aligned with the project template and the current C implementation style.

## Validation Strategy

The intended validation flow is:

```bash
docker compose build compiler
docker compose run --rm compiler src/main/bash/build.sh
docker compose run --rm compiler src/main/bash/test.sh
```

Why Docker:

* the repository expects a newer `bison` than the one available in some host environments
* `cmake` may be absent locally
* the project is meant to be reproducible inside the provided Ubuntu-based toolchain

## Expected Runtime Behaviour

At this stage, the executable returns:

* `0` when the frontend tokenizes the input, parses it successfully and builds an AST
* non-zero when lexical or syntactic analysis rejects the input

The backend remains wired through stub modules only to preserve the compiler pipeline shape. No semantic validation or code generation result should be expected from Stage 2.

## Out of Scope Until Stage 3

The following features are deliberately not part of this deliverable:

* symbol tables
* declaration/use checks
* type or role restrictions between signals
* instance interface validation
* illegal combinational cycle detection
* simulation across clock cycles
* code generation
