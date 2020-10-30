#pragma once

#include <cstring>
#include "../Array.h"

#define DefineArrayOperator(op)									\
template <typename T>											\
Array<T> operator##op(const Array<T>& a, const Array<T>& b) {	\
	Array<T> ans = a.copy();									\
	for (size_t i = 0; i < ans.measureOf(0); i++) {				\
		ans.cptr()[i] ##op= b.cptr()[i];						\
	}															\
	return ans;													\
}																\
template <typename T, typename Number_T>						\
Array<T> operator##op(const Array<T>& a, const Number_T b) {	\
	Array<T> ans = a;											\
	for (size_t i = 0; i < ans.measureOf(0); i++) {				\
		ans.cptr()[i] ##op= b;									\
	}															\
	return ans;													\
}

DefineArrayOperator(+)
DefineArrayOperator(-)
DefineArrayOperator(*)
DefineArrayOperator(/)

#undef DefineArrayOperator