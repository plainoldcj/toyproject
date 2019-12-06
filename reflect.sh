#!/bin/bash
find ./src -type f -iname '*.h' -exec grep -l REFLECTED {} \; | xargs ./build/reflect
