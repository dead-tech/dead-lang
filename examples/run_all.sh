#!/bin/bash

set -xeuo pipefail

find examples -name "*.dl" | xargs -I '{}' -n 1 build/dl -L '{}'
