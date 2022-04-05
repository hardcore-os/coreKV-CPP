#!/bin/bash
echo "start clean project"
cd src
bazel clean
cd ..
echo "end clean project"
