# Notas del Frontend de la Etapa 2

## Alcance de la entrega

Este snapshot del repositorio cierra la Etapa 2 del proyecto de ATLC. El alcance implementado es:

* análisis léxico basado en Flex
* análisis sintáctico basado en Bison
* construcción del AST para el DSL de circuitos
* tests de aceptación/rechazo orientados al comportamiento del frontend

El análisis semántico queda intencionalmente fuera del alcance de esta etapa y se difiere a la Etapa 3.

## Subconjunto del DSL implementado

El frontend actual acepta:

* definiciones de módulos `circuit`
* declaraciones de señales con `input`, `output`, `wire`, `reg`
* asignaciones combinacionales con `=`
* bloques secuenciales escritos como `on rising_edge(clk) { ... }`
* asignaciones secuenciales con `<=`
* expresiones booleanas con `not`, `and`, `xor`, `or`
* instanciaciones de subcircuitos con conexiones nombradas de entrada/salida

Los programas pueden contener múltiples circuitos en la misma entrada.

## Cobertura del AST

El AST modela:

* raíces de programa con múltiples circuitos
* definiciones de circuito
* ítems de declaración
* ítems de sentencia
* asignaciones combinacionales
* bloques sensibles al reloj
* asignaciones secuenciales
* expresiones booleanas
* instancias con conexiones nombradas de entrada/salida

Las listas se representan como nodos de listas enlazadas para mantenerse alineadas con el template del proyecto y con el estilo de implementación actual en C.

## Estrategia de validación

El flujo de validación previsto es:

```bash
docker compose build compiler
docker compose run --rm compiler src/main/bash/build.sh
docker compose run --rm compiler src/main/bash/test.sh
```

Por qué Docker:

* el repositorio espera una versión de `bison` más nueva que la disponible en algunos entornos host
* `cmake` puede no estar instalado localmente
* el proyecto está pensado para ser reproducible dentro del toolchain provisto basado en Ubuntu

## Comportamiento de ejecución esperado

En esta etapa, el ejecutable retorna:

* `0` cuando el frontend tokeniza la entrada, la parsea con éxito y construye un AST
* distinto de cero cuando el análisis léxico o sintáctico rechaza la entrada

El backend permanece conectado a través de módulos stub solamente para preservar la forma de la pipeline del compilador. No se debe esperar ningún resultado de validación semántica ni de generación de código en la Etapa 2.

## Fuera de alcance hasta la Etapa 3

Las siguientes funcionalidades quedan deliberadamente fuera de esta entrega:

* tablas de símbolos
* chequeos de declaración/uso
* restricciones de tipo o rol entre señales
* validación de la interfaz de las instancias
* detección de ciclos combinacionales ilegales
* simulación a través de ciclos de reloj
* generación de código
