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
		template<typename... CmptTypes>
		CmptTypeSet(CmptTypes... types) : std::set<CmptType>{ types... } {
			static_assert((std::is_same_v<CmptTypes, CmptType> &&...));
		}

		template<typename... Cmpts>
		static constexpr size_t HashCodeOf() noexcept;

		size_t HashCode() const { return hashcode; }

		template<typename... Cmpts>
		void Insert();

		template<typename... Cmpts>
		void Erase() noexcept;

		template<typename... Cmpts>
		constexpr bool IsContain() const;

		template<typename... Cmpts>
		constexpr bool IsContain(TypeList<Cmpts...>) const;

		inline bool IsContain(CmptType type) const;

		template<typename CmptTypeContainer>
		bool IsContain(const CmptTypeContainer& types) const;

		template<typename... Cmpts>
		constexpr bool IsContainAny() const;

		template<typename... Cmpts>
		constexpr bool IsContainAny(TypeList<Cmpts...>) const;

		template<typename CmptTypeContainer>
		bool IsContainAny(const CmptTypeContainer& types) const;

		template<typename... Cmpts>
		constexpr bool IsNotContain() const;

		template<typename... Cmpts>
		constexpr bool IsNotContain(TypeList<Cmpts...>) const;

		inline bool IsNotContain(CmptType type) const;

		template<typename CmptTypeContainer>
		bool IsNotContain(const CmptTypeContainer& types) const;

		inline bool IsMatch(const EntityFilter& filter) const;

		inline bool IsMatch(const EntityLocator& locator) const;

		inline bool IsMatch(const EntityQuery& query) const;

		template<typename... Cmpts>
		bool Is() const;

		using std::set<CmptType>::begin;
		using std::set<CmptType>::end;
		using std::set<CmptType>::size;

		friend bool operator==(const CmptTypeSet& x, const CmptTypeSet& y) {
			return static_cast<const std::set<CmptType>&>(x) == static_cast<const std::set<CmptType>&>(y);
		}

	private:
		template<typename... Cmpts>
		static constexpr size_t HashCodeOf(TypeList<Cmpts...>) noexcept;

		template<typename Container>
		static constexpr size_t HashCodeOf(const Container& cmpts);

		size_t hashcode;
	};
}

#include "CmptTypeSet.inl"
