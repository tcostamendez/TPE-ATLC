[![Entrega](https://img.shields.io/badge/Entrega-Stage%20III-ffb600.svg?style=for-the-badge)](https://github.com/tcostamendez/TPE-ATLC/tree/development)

[![CI](https://github.com/tcostamendez/TPE-ATLC/actions/workflows/pipeline.yaml/badge.svg?branch=development)](https://github.com/tcostamendez/TPE-ATLC/actions/workflows/pipeline.yaml)

# TPE-ATLC

Proyecto de compilador de ATLC desarrollado en C con Flex y Bison. El estado actual del repositorio corresponde a la **Etapa 3 (Backend)** del proyecto: frontend, análisis semántico y generación de simuladores C99 para un DSL de descripción de hardware orientado a circuitos booleanos secuenciales síncronos.

Repositorio: https://github.com/tcostamendez/TPE-ATLC

* [Alcance de la Etapa 3](#alcance-de-la-etapa-3)
* [Resumen del lenguaje](#resumen-del-lenguaje)
* [Formato del simulador generado](#formato-del-simulador-generado)
* [Requisitos](#requisitos)
* [Configuración](#configuración)
* [Comandos](#comandos)
* [Tests](#tests)
* [Documentación](#documentación)
* [CI/CD](#cicd)
* [Extensiones recomendadas](#extensiones-recomendadas)

## Alcance de la Etapa 3

Esta entrega implementa:

* análisis léxico con Flex
* análisis sintáctico con Bison
* construcción del AST para programas válidos
* análisis semántico
* tablas de símbolos
* validación de la interfaz de instancias
* detección de ciclos combinacionales
* generación de un simulador C99 autocontenido
* tests de aceptación/rechazo y fixtures de simulación

## Resumen del lenguaje

El compilador reconoce programas compuestos por una o más definiciones `circuit` con:

* declaraciones: `input`, `output`, `wire`, `reg`
* asignaciones combinacionales con `=`
* bloques secuenciales con `on rising_edge(clk) { ... }` y `on falling_edge(clk) { ... }`
* asignaciones secuenciales con `<=`
* expresiones booleanas con `not`, `and`, `xor`, `or`
* aliases `.` para `and` y `+` para `or`
* instanciación de subcircuitos con `CircuitName(...) -> (...);`

Ejemplo:

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

## Formato del simulador generado

El compilador emite por `stdout` un programa C99 autocontenido. Ese programa lee por `stdin`:

1. un entero `N`, que indica la cantidad de ciclos a simular;
2. `N` filas con los valores booleanos de los `input` del circuito top, en orden de declaración.

Después de cada ciclo imprime los `output` del circuito top, también en orden de declaración. Por ejemplo, para un circuito con entradas `a, b` y salidas `sum, carry`:

```txt
4
0 0
0 1
1 0
1 1
```

produce una línea de salida por ciclo:

```txt
0 0
1 0
1 0
0 1
```

## Requisitos

* [Docker](https://www.docker.com/)

El entorno previsto para compilar y correr los tests es el setup de Docker que viene con el repositorio. Esta es la fuente de verdad de la entrega: el host puede tener una versión antigua de `bison`, no tener `cmake` instalado o no poder ejecutar binarios Linux generados dentro del contenedor.

## Configuración

Definir las siguientes variables de entorno para controlar el comportamiento del compilador:

| Nombre                | Default | Descripción |
| :-------------------- | :-----: | :---------- |
| `ENVIRONMENT`         | `Local` | Nombre del entorno activo. Valores disponibles: `Local`, `Development`, `Production`. |
| `LOG_IGNORED_LEXEMES` | `true`  | Cuando es `true`, los lexemas ignorados se registran en nivel `DEBUGGING`. |
| `LOGGING_LEVEL`       | `ALL`   | Nivel mínimo de logging. Valores disponibles: `ALL`, `DEBUGGING`, `INFORMATION`, `WARNING`, `ERROR`, `CRITICAL`. |

`docker compose` también puede leer estos valores desde un archivo `.env`.

## Comandos

### Iniciar un contenedor de desarrollo

```bash
docker compose run --rm compiler
```

### Build

Regenera los archivos del parser/scanner y compila el compilador:

```bash
src/main/bash/build.sh
```

### Ejecutar

Compila un programa desde la entrada estándar y emite un simulador C99 por `stdout`:

```bash
src/main/bash/run.sh <programa>
```

El ejecutable retorna:

* `0` cuando el programa pasa frontend y semántica
* distinto de cero cuando el análisis léxico, sintáctico o semántico rechaza el programa

También se puede elegir el circuito principal:

```bash
LOGGING_LEVEL=ERROR .build/Flex-Bison-Compiler --top Main <programa >simulator.c
```

Si no se indica `--top`, se usa como top el último circuito definido en el programa.

### Tests

Dentro del contenedor, corre la suite de aceptación/rechazo:

```bash
src/main/bash/test.sh
```

Dentro del contenedor, corre los fixtures que compilan y ejecutan simuladores generados:

```bash
src/main/bash/codegen-test.sh
```

Para validar la Etapa 3 completa se deben ejecutar ambos scripts luego del build, ya que `test.sh` cubre frontend y semántica, mientras que `codegen-test.sh` cubre generación de código y runtime.

### Detener

```bash
exit
docker compose down
```

## Tests

La suite de tests bajo `src/test/c` cubre sintaxis, semántica y generación. La validación reproducible de entrega es:

```bash
docker compose run --rm compiler sh -lc 'src/main/bash/build.sh && src/main/bash/test.sh && src/main/bash/codegen-test.sh'
```

* `src/test/c/accept`: programas válidos que deben generar C
* `src/test/c/reject`: programas inválidos que deben fallar en frontend o semántica
* `src/test/c/codegen`: entradas y salidas esperadas para simuladores generados
* `*.stderr`: fragmentos esperados del diagnóstico para casos seleccionados de falla

## Documentación

* Especificación de la Etapa 1: [doc/Especificacion-Stage1.pdf](doc/Especificacion-Stage1.pdf)
* Notas de implementación de la Etapa 2: [doc/Stage2-Frontend.md](doc/Stage2-Frontend.md)
* Informe de la Etapa 3: [doc/Informe-Stage3.md](doc/Informe-Stage3.md) / [doc/Informe-Stage3.pdf](doc/Informe-Stage3.pdf)

El Markdown del informe de Stage 3 es la fuente editable. El PDF incluido corresponde a la versión preparada para la entrega.

## CI/CD

Para activar la integración automática en cada push o pull request, activar GitHub Actions en la configuración del repositorio y aplicar:

| Clave                                                      | Valor |
| :--------------------------------------------------------- | :---- |
| `Actions permissions`                                      | `Allow all actions and reusable workflows` |
| `Allow GitHub Actions to create and approve pull requests` | `false` |
| `Artifact and log retention`                               | `30 days` |
| `Fork pull request workflows from outside collaborators`   | `Require approval for all outside collaborators` |
| `Workflow permissions`                                     | `Read repository contents and packages permissions` |

## Extensiones recomendadas

* [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
* [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)
* [Yash](https://marketplace.visualstudio.com/items?itemName=daohong-emilio.yash)
