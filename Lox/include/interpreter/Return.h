#ifndef RETURN_H
#define RETURN_H

#include "core/Common.h"

struct ReturnException {
	LiteralValue value;
	ReturnException(LiteralValue value) : value(value) {}
};

#endif	// RETURN_H
