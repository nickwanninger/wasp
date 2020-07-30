#!/usr/bin/env bash
set -e

mkdir -p ./data

# HACK: just manually copied the output from the json
aws lambda invoke --function-name wasp-fib-dataflow \
      --payload "" \
      --output text \
      /dev/stdout | tee output.txt
