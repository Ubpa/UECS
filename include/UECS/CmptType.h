#pragma once

#include <UTemplate/TypeID.h>

namespace Ubpa {
	class CmptType {
	public:
		explicit constexpr CmptType(size_t id) : hashcode{ id } {}
		explicit constexpr CmptType(std::string_view type_name) : hashcode{ RuntimeTypeID(type_name) } {}

		template<typename Cmpt>
		static constexpr CmptType Of() noexcept { return CmptType{ TypeID<Cmpt> }; }

		constexpr size_t HashCode() const noexcept { return hashcode; }

		template<typename Cmpt>
		static constexpr size_t HashCodeOf() noexcept { return TypeID<Cmpt>; }

		template<typename Cmpt>
		bool Is() const noexcept { return hashcode == HashCodeOf<Cmpt>(); }

	private:
		size_t hashcode;
	};
}

#include "detail/CmptType.inl"
