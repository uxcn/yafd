#!/bin/sh

DIR=$PWD

cd `dirname $0`

autoreconf --install && \
./configure "$@"

cd $DIR
