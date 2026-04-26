[![✗](https://img.shields.io/badge/Release-v2.0.0-ffb600.svg?style=for-the-badge)](https://github.com/tcostamendez/TPE-ATLC/releases)

[![✗](https://github.com/tcostamendez/TPE-ATLC/actions/workflows/pipeline.yaml/badge.svg?branch=production)](https://github.com/tcostamendez/TPE-ATLC/actions/workflows/pipeline.yaml)

# TPE-ATLC

ATLC compiler project developed in C with Flex and Bison. The current repository state corresponds to **Stage 2 (Frontend)** of the project: lexical analysis, syntactic analysis and AST construction for a hardware-description DSL for synchronous sequential boolean circuits.

* [Stage 2 Scope](#stage-2-scope)
* [Language Overview](#language-overview)
* [Requirements](#requirements)
* [Configuration](#configuration)
* [Commands](#commands)
* [Tests](#tests)
* [Documentation](#documentation)
* [CI/CD](#cicd)
* [Recommended Extensions](#recommended-extensions)

## Stage 2 Scope

This deliverable implements:

* lexical analysis with Flex
* syntactic analysis with Bison
* AST construction for valid programs
* parser-oriented accept/reject tests

This deliverable does **not** implement yet:

* semantic analysis
* symbol tables
* instance interface validation
* combinational cycle detection
* simulation
* code generation

The backend modules are intentionally kept as stubs so the compiler pipeline remains wired while Stage 2 stops after AST construction.

## Language Overview

The Stage 2 frontend recognizes programs composed of one or more `circuit` definitions with:

* declarations: `input`, `output`, `wire`, `reg`
* combinational assignments with `=`
* sequential blocks with `on rising_edge(clk) { ... }`
* sequential assignments with `<=`
* boolean expressions using `not`, `and`, `xor`, `or`
* subcircuit instantiation with `CircuitName(...) -> (...);`

Example:

```txt
circuit Register1 {
input in, load, clk;
output out;
reg value;
wire next;

out = value;
next = (load and in) or ((not load) and value);

on rising_edge(clk) {
	value <= next;
}
}
```

## Requirements

* [Docker](https://www.docker.com/)

The intended build and test environment is the Docker setup shipped with the repository. This is especially important because the host environment may contain an older `bison` or may not have `cmake` installed.

## Configuration

Set the following environment variables to control the compiler behaviour:

| Name                  | Default | Description |
| :-------------------- | :-----: | :---------- |
| `ENVIRONMENT`         | `Local` | Active environment name. Available values: `Local`, `Development`, `Production`. |
| `LOG_IGNORED_LEXEMES` | `true`  | When `true`, ignored lexemes are logged at `DEBUGGING` level. |
| `LOGGING_LEVEL`       | `ALL`   | Minimum logging level. Available values: `ALL`, `DEBUGGING`, `INFORMATION`, `WARNING`, `ERROR`, `CRITICAL`. |

`docker compose` can also read these values from an `.env` file.

## Commands

### Start a development container

```bash
docker compose run --rm compiler
```

### Build

Regenerates parser/scanner sources and builds the compiler:

```bash
src/main/bash/build.sh
```

### Run

Compiles a single input program from standard input:

```bash
src/main/bash/run.sh <program>
```

The executable returns:

* `0` when the frontend accepts the program and builds an AST
* non-zero when lexical or syntactic analysis rejects the program

### Test

Runs the Stage 2 acceptance/rejection suite:

```bash
src/main/bash/test.sh
```

### Stop

```bash
exit
docker compose down
```

## Tests

The test suite under `src/test/c` is syntax-oriented only.

* `src/test/c/accept`: valid programs that must reach AST construction
* `src/test/c/reject`: invalid programs that must fail in the frontend
* `*.stderr`: expected diagnostic fragments for selected failure cases

Stage 2 may still accept programs that are semantically invalid, because semantic validation belongs to Stage 3.

## Documentation

* Stage 1 specification: [doc/Especificacion-Stage1.pdf](doc/Especificacion-Stage1.pdf)
* Stage 2 implementation notes: [doc/Stage2-Frontend.md](doc/Stage2-Frontend.md)

## CI/CD

To trigger automatic integration on push or pull requests, activate GitHub Actions in the repository settings and configure:

| Key                                                        | Value |
| :--------------------------------------------------------- | :---- |
| `Actions permissions`                                      | `Allow all actions and reusable workflows` |
| `Allow GitHub Actions to create and approve pull requests` | `false` |
| `Artifact and log retention`                               | `30 days` |
| `Fork pull request workflows from outside collaborators`   | `Require approval for all outside collaborators` |
| `Workflow permissions`                                     | `Read repository contents and packages permissions` |

## Recommended Extensions

* [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
* [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)
* [Yash](https://marketplace.visualstudio.com/items?itemName=daohong-emilio.yash)
