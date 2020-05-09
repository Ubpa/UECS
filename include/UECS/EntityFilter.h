#pragma once

#include "CmptType.h"

#include <UTemplate/TypeList.h>

#include <set>

namespace Ubpa {
	class EntityFilter {
	public:
		EntityFilter();

		template<typename... AllCmpts, typename... AnyCmpts, typename... NoneCmpts>
		EntityFilter(TypeList<AllCmpts...> allList, TypeList<AnyCmpts...> anyList, TypeList<NoneCmpts...> noneList);

		size_t HashCode() const noexcept { return combinedHashCode; }

		const std::set<CmptType>& AllCmptTypes() const noexcept { return allCmptTypes; }
		const std::set<CmptType>& AnyCmptTypes() const noexcept { return anyCmptTypes; }
		const std::set<CmptType>& NoneCmptTypes() const noexcept { return noneCmptTypes; }

	private:
		size_t GenAllHashCode() const noexcept;
		size_t GenAnyHashCode() const noexcept;
		size_t GenNoneHashCode() const noexcept;

		std::set<CmptType> allCmptTypes;
		std::set<CmptType> anyCmptTypes;
		std::set<CmptType> noneCmptTypes;

		size_t allHashCode;
		size_t anyHashCode;
		size_t noneHashCode;

		size_t combinedHashCode;
	};
}

#include "detail/EntityFilter.inl"
