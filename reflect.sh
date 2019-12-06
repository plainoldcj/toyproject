#!/bin/bash
find ./src -type f -exec grep -l REFLECTED {} \; | xargs ./build/reflect
