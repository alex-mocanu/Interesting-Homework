#!/bin/bash

timeout 15 ./accpop 400 input/easy1.in output/easy1.out | tail -n 50
timeout 15 ./accpop 400 input/easy2.in output/easy2.out | tail -n 50
timeout 15 ./accpop 400 input/easy3.in output/easy3.out | tail -n 50
timeout 20 ./accpop 200 input/medium1.in output/medium1.out | tail -n 50
timeout 20 ./accpop 200 input/medium2.in output/medium2.out | tail -n 50
timeout 20 ./accpop 200 input/medium3.in output/medium3.out | tail -n 50
timeout 25 ./accpop 150 input/hard1.in output/hard1.out | tail -n 50
timeout 25 ./accpop 150 input/hard2.in output/hard2.out | tail -n 50
timeout 25 ./accpop 100 input/hard3.in output/hard3.out | tail -n 50
timeout 25 ./accpop 100 input/extreme1.in output/extreme1.out | tail -n 50
