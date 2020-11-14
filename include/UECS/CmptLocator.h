#pragma once

#include "CmptTag.h"

#include "CmptType.h"

#include <UContainer/Span.h>

namespace Ubpa::UECS {
	// locate components in function's argument list for Archetype
	// immutable
	class CmptLocator {
	public:
		CmptLocator(Span<const CmptAccessType> types);

		CmptLocator();

		template<typename Func>
		static CmptLocator Generate();

		template<typename Func>
		CmptLocator& Combine();

		size_t HashCode() const noexcept { return hashCode; }

		const CmptAccessTypeSet& CmptAccessTypes() const noexcept { return cmptTypes; }

		bool operator==(const CmptLocator& rhs) const noexcept;

		bool HasWriteCmptType() const noexcept;
	private:
		void UpdateHashCode() noexcept;

		CmptAccessTypeSet cmptTypes;

		size_t hashCode;
	};
}

#include "detail/CmptsLocator.inl"
