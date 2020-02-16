#pragma once

#include <stdbool.h>
#include <stdint.h>

struct ReflectedType;

bool ReadJson(
	const struct ReflectedType* type,
	void* object,
	const char* json,
	uint32_t len,
	const char* debugName);
