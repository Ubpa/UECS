#pragma once

#include "CmptType.h"

#include <UTemplate/TypeList.h>

#include <set>

namespace Ubpa::UECS {
	// filter Archetype with All, Any and None
	class ArchetypeFilter {
	public:
		ArchetypeFilter();

		template<typename... AllCmpts, typename... AnyCmpts, typename... NoneCmpts>
		ArchetypeFilter(TypeList<AllCmpts...> allList, TypeList<AnyCmpts...> anyList, TypeList<NoneCmpts...> noneList);

		template<typename... AllCmpts>
		static ArchetypeFilter CreateAll() { return { TypeList<AllCmpts...>, TypeList<>{}, TypeList<>{} }; }
		template<typename... AnyCmpts>
		static ArchetypeFilter CreateAny() { return { TypeList<>{}, TypeList<AnyCmpts...>, TypeList<>{} }; }
		template<typename... NoneCmpts>
		static ArchetypeFilter CreateNone() { return { TypeList<>{}, TypeList<>{}, TypeList<NoneCmpts...> }; }

		ArchetypeFilter(
			std::set<CmptType> allCmptTypes,
			std::set<CmptType> anyCmptTypes = {},
			std::set<CmptType> noneCmptTypes = {}
		);

		size_t HashCode() const noexcept { return combinedHashCode; }

		const std::set<CmptType>& AllCmptTypes() const noexcept { return allCmptTypes; }
		const std::set<CmptType>& AnyCmptTypes() const noexcept { return anyCmptTypes; }
		const std::set<CmptType>& NoneCmptTypes() const noexcept { return noneCmptTypes; }

		// [API]
		// <Mode><Type>(CmptTypeContainer|CmptType*, num)
		// - <Mode>: Insert | Erase
		// - <Type>: All | Any | None
		// - side effect: update hashcode

		void InsertAll(const CmptType* types, size_t num);
		void InsertAny(const CmptType* types, size_t num);
		void InsertNone(const CmptType* types, size_t num);
		void EraseAll(const CmptType* types, size_t num);
		void EraseAny(const CmptType* types, size_t num);
		void EraseNone(const CmptType* types, size_t num);

		template<typename CmptTypeContainer> void InsertAll(const CmptTypeContainer&);
		template<typename CmptTypeContainer> void InsertAny(const CmptTypeContainer&);
		template<typename CmptTypeContainer> void InsertNone(const CmptTypeContainer&);
		template<typename CmptTypeContainer> void EraseAll(const CmptTypeContainer&);
		template<typename CmptTypeContainer> void EraseAny(const CmptTypeContainer&);
		template<typename CmptTypeContainer> void EraseNone(const CmptTypeContainer&);

		bool operator==(const ArchetypeFilter& filter) const noexcept;

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

		size_t combinedHashCode;
	};
}

#include "detail/ArchetypeFilter.inl"
