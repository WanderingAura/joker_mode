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
