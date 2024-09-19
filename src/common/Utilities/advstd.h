#ifndef ADVSTD_H_
#define ADVSTD_H_

#include <cstddef>
#include <type_traits>

namespace advstd
{
	template<typename T>
	using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

	template<typename T>
	struct type_identity
	{
		using type = T;
	};

	template<typename T>
	using type_identity_t = typename type_identity<T>::type;
}

#endif
