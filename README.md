[![✗](https://img.shields.io/badge/Release-v2.0.0-ffb600.svg?style=for-the-badge)](https://github.com/tcostamendez/TPE-ATLC/releases)

[![✗](https://github.com/tcostamendez/TPE-ATLC/actions/workflows/pipeline.yaml/badge.svg?branch=production)](https://github.com/tcostamendez/TPE-ATLC/actions/workflows/pipeline.yaml)

# TPE-ATLC

Proyecto de compilador de ATLC desarrollado en C con Flex y Bison. El estado actual del repositorio corresponde a la **Etapa 2 (Frontend)** del proyecto: análisis léxico, análisis sintáctico y construcción del AST para un DSL de descripción de hardware orientado a circuitos booleanos secuenciales síncronos.

* [Alcance de la Etapa 2](#alcance-de-la-etapa-2)
* [Resumen del lenguaje](#resumen-del-lenguaje)
* [Requisitos](#requisitos)
* [Configuración](#configuración)
* [Comandos](#comandos)
* [Tests](#tests)
* [Documentación](#documentación)
* [CI/CD](#cicd)
* [Extensiones recomendadas](#extensiones-recomendadas)

## Alcance de la Etapa 2

Esta entrega implementa:

* análisis léxico con Flex
* análisis sintáctico con Bison
* construcción del AST para programas válidos
* tests de aceptación/rechazo orientados al parser

Esta entrega **no** implementa todavía:

* análisis semántico
* tablas de símbolos
* validación de la interfaz de instancias
* detección de ciclos combinacionales
* simulación
* generación de código

Los módulos del backend se mantienen como stubs para que la pipeline del compilador siga conectada, mientras la Etapa 2 termina luego de la construcción del AST.

## Resumen del lenguaje

El frontend de la Etapa 2 reconoce programas compuestos por una o más definiciones `circuit` con:

* declaraciones: `input`, `output`, `wire`, `reg`
* asignaciones combinacionales con `=`
* bloques secuenciales con `on rising_edge(clk) { ... }`
* asignaciones secuenciales con `<=`
* expresiones booleanas con `not`, `and`, `xor`, `or`
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

## Requisitos

* [Docker](https://www.docker.com/)

El entorno previsto para compilar y correr los tests es el setup de Docker que viene con el repositorio. Esto es importante porque el host puede tener una versión antigua de `bison` o no tener `cmake` instalado.

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

Compila un programa desde la entrada estándar:

```bash
src/main/bash/run.sh <programa>
```

El ejecutable retorna:

* `0` cuando el frontend acepta el programa y construye un AST
* distinto de cero cuando el análisis léxico o sintáctico rechaza el programa

### Tests

Corre la suite de aceptación/rechazo de la Etapa 2:

```bash
src/main/bash/test.sh
```

### Detener

```bash
exit
docker compose down
```

## Tests

La suite de tests bajo `src/test/c` sólo cubre sintaxis.

* `src/test/c/accept`: programas válidos que deben llegar a la construcción del AST
* `src/test/c/reject`: programas inválidos que deben fallar en el frontend
* `*.stderr`: fragmentos esperados del diagnóstico para casos seleccionados de falla

La Etapa 2 todavía puede aceptar programas que son semánticamente inválidos, ya que la validación semántica corresponde a la Etapa 3.

## Documentación

* Especificación de la Etapa 1: [doc/Especificacion-Stage1.pdf](doc/Especificacion-Stage1.pdf)
* Notas de implementación de la Etapa 2: [doc/Stage2-Frontend.md](doc/Stage2-Frontend.md)

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
