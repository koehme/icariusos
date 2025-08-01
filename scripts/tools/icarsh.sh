#!/bin/bash

if [[ ! -f ./bin/ICARIUS.BIN ]]; then
  echo "❌ RUN FROM PROJECT ROOT DIR!"
  exit 1
fi

make icarsh && ./fat16.sh