#!/bin/bash
usage()
{
  echo "Usage: $0 <debug|release>"
}

if [[ $# -ne 1 ]] ; then
  usage
  exit 1
fi

cmake --workflow --preset debug
