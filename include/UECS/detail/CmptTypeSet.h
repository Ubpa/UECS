#pragma	once

#include "Util.h"
#include "../EntityQuery.h"

#include <UTemplate/TypeID.h>
#include <UTemplate/TemplateList.h>

#include <set>

namespace Ubpa::UECS {
	struct CmptTypeSet {
		std::set<CmptType> data;

		size_t HashCode() const;

		void Insert(const CmptType* types, size_t num);
		void Erase(const CmptType* types, size_t num);
		bool Contains(CmptType type) const;

		bool ContainsAll(const CmptType* types, size_t num) const;
		template<typename CmptTypeContainer>
		bool ContainsAll(const CmptTypeContainer& types) const;

		bool ContainsAny(const CmptType* types, size_t num) const;
		template<typename CmptTypeContainer>
		bool ContainsAny(const CmptTypeContainer& types) const;

		template<typename CmptTypeContainer>
		bool NotContain(const CmptTypeContainer& types) const;

		bool IsMatch(const ArchetypeFilter& filter) const;

		bool IsMatch(const CmptLocator& locator) const;

		bool IsMatch(const EntityQuery& query) const;

		bool operator==(const CmptTypeSet& rhs) const { return data == rhs.data; }
	};
}

#include "CmptTypeSet.inl"
