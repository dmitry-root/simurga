#!/bin/sh

export DEBUG=yes
make -j5 all
make -j5 test
make -j5 install

