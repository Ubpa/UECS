#pragma once

#include "CmptTag.h"

#include "CmptType.h"

#include <set>

namespace Ubpa::UECS {
	// locate components in function's argument list for Archetype
	// immutable
	class CmptLocator {
	public:
		CmptLocator(const CmptAccessType* types, size_t num);

		CmptLocator();

		template<typename Func>
		static CmptLocator Generate();

		template<typename Func>
		CmptLocator& Combine();

		size_t HashCode() const noexcept { return hashCode; }

		const std::set<CmptAccessType>& CmptAccessTypes() const noexcept { return cmptTypes; }

		bool operator==(const CmptLocator& rhs) const;
	private:
		size_t GenHashCode() const noexcept;

		std::set<CmptAccessType> cmptTypes;

		size_t hashCode;
	};
}

#include "detail/CmptsLocator.inl"
