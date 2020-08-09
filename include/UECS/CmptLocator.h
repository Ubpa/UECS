#pragma once

#include "CmptTag.h"

#include "CmptType.h"

#include <set>

namespace Ubpa::UECS {
	// locate components in function's argument list for Archetype
	// TODO: combine with a system function's locator
	class CmptLocator {
	public:
		CmptLocator(const CmptType* types, size_t num);

		CmptLocator();

		template<typename Func>
		static CmptLocator Generate();

		template<typename Func>
		CmptLocator& Combine();

		size_t HashCode() const noexcept { return hashCode; }

		const std::set<CmptType>& CmptTypes() const noexcept { return cmptTypes; }

		bool operator==(const CmptLocator& rhs) const noexcept;
	private:
		size_t GenHashCode() const noexcept;

		std::set<CmptType> cmptTypes;

		size_t hashCode;
	};
}

#include "detail/CmptsLocator.inl"
