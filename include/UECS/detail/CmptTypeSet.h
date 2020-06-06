#pragma	once

#include "Util.h"
#include "../EntityQuery.h"

#include <UTemplate/TypeID.h>
#include <UTemplate/TemplateList.h>

#include <set>

namespace Ubpa {
	class CmptTypeSet : std::set<CmptType> {
	public:
		CmptTypeSet() : hashcode{ TypeID<CmptTypeSet> } {}
		template<typename... Cmpts>
		CmptTypeSet(TypeList<Cmpts...>);
		template<typename... CmptTypes,
			// for function overload
			typename = std::enable_if_t<(std::is_same_v<CmptTypes, CmptType>&&...)>>
		CmptTypeSet(CmptTypes... types) : std::set<CmptType>{ types... }, hashcode{ HashCodeOf(*this) } {}
		CmptTypeSet(const CmptType* types, size_t num);

		template<typename... Cmpts>
		static constexpr size_t HashCodeOf() noexcept;

		size_t HashCode() const noexcept { return hashcode; }

		template<typename... CmptTypes>
		void Insert(CmptTypes...);

		template<typename... CmptTypes>
		void Erase(CmptTypes...) noexcept;

		bool Contains(CmptType type) const;

		template<typename CmptTypeContainer>
		bool Contains(const CmptTypeContainer& types) const;

		template<typename CmptTypeContainer>
		bool ContainsAny(const CmptTypeContainer& types) const;

		template<typename CmptTypeContainer>
		bool NotContain(const CmptTypeContainer& types) const;

		bool IsMatch(const EntityFilter& filter) const;

		bool IsMatch(const EntityLocator& locator) const;

		bool IsMatch(const EntityQuery& query) const;

		template<typename... Cmpts>
		bool Is() const;

		using std::set<CmptType>::begin;
		using std::set<CmptType>::end;
		using std::set<CmptType>::size;

		bool operator==(const CmptTypeSet& rhs) const;

	private:
		template<typename... Cmpts>
		static constexpr size_t HashCodeOf(TypeList<Cmpts...>) noexcept;

		template<typename Container>
		static constexpr size_t HashCodeOf(const Container& cmpts) noexcept;

		size_t hashcode;
	};
}

#include "CmptTypeSet.inl"
