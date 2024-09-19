#ifndef TYPES_H_
#define TYPES_H_

#include "advstd.h"

namespace mpool
{
	struct find_type_end;

	template<template<typename ...> typename Check, typename... Ts>
	struct find_type_if;

	template<template<typename ...> typename Check>
	struct find_type_if<Check>
	{
		using type = find_type_end;
	};

	template<template<typename ...> typename Check, typename T1, typename... Ts>
	struct find_type_if<Check, T1, Ts...> : std::conditional_t<Check<T1>::value, advstd::type_identity<T1>, find_type_if<Check, Ts...>>
	{

	};

	template<template<typename...> typename Check, typename...Ts>
	using find_type_if_t = typename find_type_if<Check, Ts...>::type;

	template<typename T>
	struct dependant_false {
		static constexpr bool value = false;
	};

	template<typename T>
	constexpr bool dependant_false_v = dependant_false<T>::value;
}

#endif
