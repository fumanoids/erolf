#!/bin/bash

# Create the documentatin for the motorboard project.

cd "$(readlink -f "$(dirname "$0")/..")"

doxygen motorboard.doxyfile

