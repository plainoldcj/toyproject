#pragma once

#include <stdbool.h>

struct ReflectedVariable;

bool ShouldReadWrite(const struct ReflectedVariable* var);
