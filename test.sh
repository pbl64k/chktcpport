#!/bin/sh
for HOST in "github.com" "localhost"; do
  for PORT in 21 22 25 80 81 443 7777 12345; do
    if ./chktcpport $HOST $PORT 3 2>/dev/null; then
      echo $HOST $PORT ": open";
    else
      echo $HOST $PORT ": closed";
    fi ;
  done ;
done
