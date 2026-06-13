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

run_test() {
	local expectation="$1"
	local test_path="$2"
	local test_name
	local stderr_expectation
	local stderr_output
	local stdout_output
	local result

	test_name="$(basename "$test_path")"
	stderr_expectation="${test_path}.stderr"
	stdout_output="$(mktemp)"
	stderr_output="$(mktemp)"

	".build/Flex-Bison-Compiler" <"$test_path" >"$stdout_output" 2>"$stderr_output"
	result="$?"

	if [ "$expectation" = "accept" ]; then
		if [ "$result" = "0" ]; then
			echo -e "    $test_name, ${GREEN}and it does${OFF} (status $result)"
		else
			STATUS=1
			echo -e "    $test_name, ${RED}but it rejects${OFF} (status $result)"
		fi
	else
		if [ "$result" != "0" ]; then
			echo -e "    $test_name, ${GREEN}and it does${OFF} (status $result)"
		else
			STATUS=1
			echo -e "    $test_name, ${RED}but it accepts${OFF} (status $result)"
		fi
	fi

	if [ -f "$stderr_expectation" ]; then
		while IFS= read -r expected_line || [ -n "$expected_line" ]; do
			if [ -z "$expected_line" ]; then
				continue
			fi
			if ! grep -F --quiet -- "$expected_line" "$stderr_output"; then
				STATUS=1
				echo -e "    $test_name, ${RED}but stderr misses${OFF}: $expected_line"
			fi
		done <"$stderr_expectation"
	fi

	rm -f "$stdout_output" "$stderr_output"
}

run_cli_reject_test() {
	local test_path="$1"
	local test_name
	local stderr_expectation
	local stderr_output
	local stdout_output
	local result

	test_name="$(basename "$test_path")"
	stderr_expectation="${test_path}.stderr"
	stdout_output="$(mktemp)"
	stderr_output="$(mktemp)"

	".build/Flex-Bison-Compiler" --top Missing <"$test_path" >"$stdout_output" 2>"$stderr_output"
	result="$?"

	if [ "$result" != "0" ]; then
		echo -e "    $test_name, ${GREEN}and it does${OFF} (status $result)"
	else
		STATUS=1
		echo -e "    $test_name, ${RED}but it accepts${OFF} (status $result)"
	fi

	if [ -f "$stderr_expectation" ]; then
		while IFS= read -r expected_line || [ -n "$expected_line" ]; do
			if [ -z "$expected_line" ]; then
				continue
			fi
			if ! grep -F --quiet -- "$expected_line" "$stderr_output"; then
				STATUS=1
				echo -e "    $test_name, ${RED}but stderr misses${OFF}: $expected_line"
			fi
		done <"$stderr_expectation"
	fi

	rm -f "$stdout_output" "$stderr_output"
}

echo "Compiler should accept..."
echo ""

for test in src/test/c/accept/*; do
	if [ ! -f "$test" ] || [ "${test##*.}" = "stderr" ]; then
		continue
	fi
	run_test "accept" "$test"
done
echo ""

echo "Compiler should reject..."
echo ""

for test in src/test/c/reject/*; do
	if [ ! -f "$test" ] || [ "${test##*.}" = "stderr" ]; then
		continue
	fi
	run_test "reject" "$test"
done
echo ""

echo "Compiler CLI should reject..."
echo ""

for test in src/test/c/cli/*; do
	if [ ! -f "$test" ] || [ "${test##*.}" = "stderr" ]; then
		continue
	fi
	run_cli_reject_test "$test"
done
echo ""

echo "All done."
exit $STATUS
