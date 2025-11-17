#!/bin/bash

function run_test() {
  for nthr in 1 2 4 8 16 32; do
    ./$1 $nthr >/dev/null
    for i in $(seq 1 3); do
      ./$1 $nthr
    done
    echo
  done
}

echo "test spin lock using Lock-Based queue"
run_test "test-LockBased"

echo "test spin lock using Lock-Free queue"
run_test "test-LockFree"

echo "test spin lock using Wait-Free queue"
run_test "test-WaitFree"