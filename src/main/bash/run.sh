#! /bin/bash

set -euo pipefail

BASE_PATH="$(dirname "$0")/../../.."
cd "$BASE_PATH"

if [ ! -x ".build/Flex-Bison-Compiler" ]; then
	echo "Missing executable .build/Flex-Bison-Compiler. Run src/main/bash/build.sh first."
	exit 1
fi

INPUT="$1"
shift 1
".build/Flex-Bison-Compiler" "$@" <"$INPUT"
