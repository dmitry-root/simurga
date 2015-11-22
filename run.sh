#!/bin/bash

make install || exit 1

echo
echo Running simurga ...

export SIMURGA_DATA=$HOME/projects/simurga/image
$SIMURGA_DATA/bin/simurga

