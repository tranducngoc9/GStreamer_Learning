#!/bin/bash
for f in *.dot; do
  echo "Converting $f to PNG..."
  dot -Tpng "$f" -o "${f%.dot}.png"
done