#!/bin/bash
cd src
echo  "\033[31m use by gtest!\n \033[0m" 

bazel test tests:all

echo "\n==========================\n"

echo "\033[31m use by ourself!\n \033[0m" 


bazel run examples:AllocTest

cd ..