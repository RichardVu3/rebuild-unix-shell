#! usr/bin/env bash
set -o nounset
set -o errexit
set -o pipefail

gcc -I./include/ -o ./bin/msh ./src/*.c

# bash scripts/build.sh && cd tests/ && gcc -I../include/ -o test_parse_tok ../src/shell.c ../src/job.c ../src/history.c ../src/signal_handlers.c test_parse_tok.c && echo "Running Test parse_tok" && ./test_parse_tok && gcc -I../include/ -o test_seperate_args ../src/shell.c ../src/job.c ../src/history.c ../src/signal_handlers.c test_separate_args.c && echo "Running Test seperate_args" && ./test_seperate_args && gcc -I../include/ -o test_history test_history.c ../src/history.c && echo "Running Test history" && ./test_history && cd .. && bash scripts/build.sh && cd tests/hw6_tests/ && source test_msh.sh && cd ../..