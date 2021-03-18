#pragma	once

#include "../EntityQuery.h"

namespace Ubpa::UECS {
	struct TypeIDSet {
		small_flat_set<TypeID> data;

		std::size_t GetValue() const noexcept;

		void Insert(std::span<const TypeID> types);
		void Erase(std::span<const TypeID> types) noexcept;
		bool Contains(TypeID type) const;

		bool ContainsAll(std::span<const TypeID> types) const;
		template<typename TypeIDContainer>
		bool ContainsAll(const TypeIDContainer& types) const;

		bool ContainsAny(std::span<const TypeID> types) const;
		template<typename TypeIDContainer>
		bool ContainsAny(const TypeIDContainer& types) const;

		template<typename TypeIDContainer>
		bool NotContain(const TypeIDContainer& types) const;

		bool IsMatch(const ArchetypeFilter& filter) const;

		bool IsMatch(const CmptLocator& locator) const;

		bool IsMatch(const EntityQuery& query) const;

		bool operator==(const TypeIDSet& rhs) const noexcept { return data == rhs.data; }
	};
}

#include "TypeIDSet.inl"
