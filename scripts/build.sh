#! usr/bin/env bash
set -o nounset
set -o errexit
set -o pipefail

gcc -I./include/ -o ./bin/msh ./src/*.c