#!/bin/bash
find ./src -type f -iname '*.h' -exec grep -l __REFLECTED__ {} \; | xargs ./build/bin/reflect_app ./build/reflected.c
