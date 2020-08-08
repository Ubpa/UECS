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

		size_t HashCode() const noexcept { return hashCode; }

		const std::set<CmptType>& LastFrameCmptTypes() const noexcept { return lastFrameCmptTypes; }
		const std::set<CmptType>& WriteCmptTypes() const noexcept { return writeCmptTypes; }
		const std::set<CmptType>& LatestCmptTypes() const noexcept { return latestCmptTypes; }
		const std::set<CmptType>& CmptTypes() const noexcept { return cmptTypes; }

		bool operator==(const CmptLocator& rhs) const noexcept;
	private:
		size_t GenHashCode() const noexcept;

		std::set<CmptType> lastFrameCmptTypes;
		std::set<CmptType> writeCmptTypes;
		std::set<CmptType> latestCmptTypes;
		std::set<CmptType> cmptTypes;

		size_t hashCode;
	};
}
