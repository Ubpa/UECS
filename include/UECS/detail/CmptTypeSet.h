#pragma	once

#include "../EntityQuery.h"

#include <UContainer/Span.h>

#include <set>

namespace Ubpa::UECS {
	struct CmptTypeSet {
		std::set<CmptType> data;

		size_t HashCode() const noexcept;

		void Insert(Span<const CmptType> types);
		void Erase(Span<const CmptType> types) noexcept;
		bool Contains(CmptType type) const;

		bool ContainsAll(Span<const CmptType> types) const;
		template<typename CmptTypeContainer>
		bool ContainsAll(const CmptTypeContainer& types) const;

		bool ContainsAny(Span<const CmptType> types) const;
		template<typename CmptTypeContainer>
		bool ContainsAny(const CmptTypeContainer& types) const;

		template<typename CmptTypeContainer>
		bool NotContain(const CmptTypeContainer& types) const;

		bool IsMatch(const ArchetypeFilter& filter) const;

		bool IsMatch(const CmptLocator& locator) const;

		bool IsMatch(const EntityQuery& query) const;

		bool operator==(const CmptTypeSet& rhs) const noexcept { return data == rhs.data; }
	};
}

#include "CmptTypeSet.inl"
