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

		template<typename Container>
		void InsertAll(const Container&);
		template<typename Container>
		void InsertAny(const Container&);
		template<typename Container>
		void InsertNone(const Container&);
		template<typename Container>
		void EraseAll(const Container&);
		template<typename Container>
		void EraseAny(const Container&);
		template<typename Container>
		void EraseNone(const Container&);

		bool operator==(const EntityFilter& filter) const noexcept;

	private:
		size_t GenAllHashCode() const noexcept;
		size_t GenAnyHashCode() const noexcept;
		size_t GenNoneHashCode() const noexcept;
		size_t GenCombinedHashCode() const noexcept;

		std::set<CmptType> allCmptTypes;
		std::set<CmptType> anyCmptTypes;
		std::set<CmptType> noneCmptTypes;

		size_t allHashCode;
		size_t anyHashCode;
		size_t noneHashCode;

		size_t combinedHashCode; // after allHashCode, anyHashCode, noneHashCode
	};
}

#include "detail/EntityFilter.inl"
