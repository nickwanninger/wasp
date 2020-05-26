#!/usr/bin/env bash
set -e

mkdir -p ./data

wasp_invoke_fib() {
  n=$1
  aws lambda invoke --function-name wasp-fib \
    --cli-binary-format raw-in-base64-out \
    --query 'LogResult' --log-type Tail \
    --payload "{\"x\":$n}" \
    --output text \
    /dev/null |
    base64 -d
}

wasp_grep_duration() {
  raw_input=$1
  echo "$raw_input" | grep -E '^REPORT' | sed -r 's/.?*Duration:\s*([0-9]+\.[0-9]+)\s*ms.*?/\1/'
}

wasp_grep_rid() {
  raw_input=$1
  echo "$raw_input" | grep -E '^REPORT' | sed -r 's/.?*RequestId:\s*([-0-9a-z]+)\s*.*?$/\1/'
}

wasp_experiment_fib() {
  n=$1
  output=$(wasp_invoke_fib "$n")
  rid=$(wasp_grep_rid "$output")
  ms=$(wasp_grep_duration "$output")
  printf "%s,%s,%s\n" "$n" "$rid" "$ms"
}

echo "n,rid,time_ms"

for i in {0..10} ; do
  for _ in {0..5} ; do
    wasp_experiment_fib $i
  done
done
