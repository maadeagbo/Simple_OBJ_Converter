/*
* Copyright (c) 2017, Moses Adeagbo
* All rights reserved.
*/
#pragma once

#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <functional>
#include "DD_Container.h"

static size_t getCharHash(const char* s)
{
	size_t h = 5381;
	int c;
	while( (c = *s++) )
		h = ((h << 5) + h) + c;
	return h;
}

/* define implementation in std::namespace for convenience */

namespace std {
    /// Specialize hashing for char strings
    template<>
    struct hash< char* > {
	    size_t operator()(const char *s) const {
	    // http://www.cse.yorku.ca/~oz/hash.html
	    size_t h = 5381;
	    int c;
	    while ((c = *s++)) h = ((h << 5) + h) + c;
	    return h;
     	}
	};
}

// small container (8 bytes + T)
template <const int T>
struct cbuff
{
	cbuff() {}
	cbuff(const char* _str) { set(_str); }
	bool compare(const char* _str)
	{
		return strcmp(str, _str) == 0;
	}
	bool operator==(const cbuff &other) const
	{
		return hash == other.hash;
	}

	bool operator<(const cbuff &other) const
	{
		return hash < other.hash;
	}

	void set(const char* _str)
	{
		snprintf(str, T, "%s", _str);
		hash = getCharHash(str);
	}

	const char* _str() const { return str; }
	size_t gethash() const { return hash; }
private:
	char str[T];
	size_t hash;
};
