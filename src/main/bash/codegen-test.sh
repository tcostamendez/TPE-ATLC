#! /bin/bash

set -u

BASE_PATH="$(dirname "$0")/../../.."
cd "$BASE_PATH"

GREEN='\033[0;32m'
RED='\033[0;31m'
OFF='\033[0m'
STATUS=0

if [ ! -x ".build/Flex-Bison-Compiler" ]; then
	echo "Missing executable .build/Flex-Bison-Compiler. Run src/main/bash/build.sh first."
	exit 1
fi

run_codegen_test() {
	local name="$1"
	local program="$2"
	local top="${3:-}"
	local input="src/test/c/codegen/${name}.input"
	local expected="src/test/c/codegen/${name}.stdout"
	local generated
	local executable
	local actual

	generated="$(mktemp).c"
	executable="$(mktemp)"
	actual="$(mktemp)"

	if [ -n "$top" ]; then
		LOGGING_LEVEL=ERROR ".build/Flex-Bison-Compiler" --top "$top" <"$program" >"$generated"
	else
		LOGGING_LEVEL=ERROR ".build/Flex-Bison-Compiler" <"$program" >"$generated"
	fi

	if [ "$?" != "0" ]; then
		STATUS=1
		echo -e "    $name, ${RED}compiler rejected program${OFF}"
	elif ! gcc -std=c99 -Wall -Wextra -o "$executable" "$generated"; then
		STATUS=1
		echo -e "    $name, ${RED}generated C does not compile${OFF}"
	elif ! "$executable" <"$input" >"$actual"; then
		STATUS=1
		echo -e "    $name, ${RED}generated simulator failed${OFF}"
	elif ! diff -u "$expected" "$actual"; then
		STATUS=1
		echo -e "    $name, ${RED}simulator output differs${OFF}"
	else
		echo -e "    $name, ${GREEN}passes${OFF}"
	fi

	rm -f "$generated" "$executable" "$actual"
}

echo "Generated simulators should run..."
echo ""
run_codegen_test "01-inverter-and" "src/test/c/accept/01-inverter-and"
run_codegen_test "02-half-adder" "src/test/c/accept/02-half-adder"
run_codegen_test "03-d-flip-flop" "src/test/c/accept/03-d-flip-flop"
run_codegen_test "04-symbolic-operators" "src/test/c/accept/19-symbolic-operators"
run_codegen_test "05-falling-edge" "src/test/c/accept/20-falling-edge"
run_codegen_test "06-composition" "src/test/c/accept/06-composition"
run_codegen_test "07-c-keyword-identifiers" "src/test/c/accept/21-c-keyword-identifiers"
run_codegen_test "08-explicit-top" "src/test/c/accept/22-explicit-top" "First"
echo ""
echo "All done."
exit $STATUS
