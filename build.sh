#!/bin/bash
usage()
{
  echo "Usage: $0 <debug|release|release-web>"
}

if [[ $# -ne 1 ]] ; then
  usage
  exit 1
fi

build=$1

cmake --workflow --preset $build

ret=$?
if [[ $ret -ne 0 ]] ; then
  echo "Build $build failed. Exiting..."
  exit 1
fi