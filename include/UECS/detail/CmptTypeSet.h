#pragma	once

#include "Util.h"
#include "../EntityQuery.h"

#include <UTemplate/TypeID.h>
#include <UTemplate/TemplateList.h>

#include <set>

namespace Ubpa {
	class CmptTypeSet : std::set<CmptType> {
	public:
		CmptTypeSet() = default;
		template<typename... Cmpts>
		CmptTypeSet(TypeList<Cmpts...>) : std::set<CmptType>{ CmptType::Of<Cmpts>()... }{}

		template<typename... Cmpts>
		static constexpr size_t HashCodeOf() {
			return HashCodeOf(QuickSort_t<TypeList<Cmpts...>, TypeID_Less>{});
		}

		size_t HashCode() const {
			size_t rst = TypeID<CmptTypeSet>;
			for (CmptType type : *this)
				rst = hash_combine(rst, type.HashCode());
			return rst;
		}

		template<typename... Cmpts>
		void Insert() { (insert(CmptType::Of<Cmpts>()), ...); }
		template<typename... Cmpts>
		void Erase() noexcept { (erase(CmptType::Of<Cmpts>()), ...); }

		template<typename... Cmpts>
		constexpr bool IsContain() const {
			if constexpr (sizeof...(Cmpts) == 0)
				return true;
			else
				return ((find(CmptType::Of<Cmpts>()) != cend()) &&...);
		}

		template<typename... Cmpts>
		constexpr bool IsContain(TypeList<Cmpts...>) const {
			return IsContain<Cmpts...>();
		}

		bool IsContain(CmptType type) const {
			return find(type) != cend();
		}

		template<typename CmptTypeContainer>
		bool IsContain(const CmptTypeContainer& types) const {
			for (auto type : types) {
				if (!IsContain(type))
					return false;
			}
			return true;
		}

		template<typename... Cmpts>
		constexpr bool IsContainAny() const {
			if constexpr (sizeof...(Cmpts) == 0)
				return true;
			else
				return ((find(CmptType::Of<Cmpts>()) != end()) ||...);
		}

		template<typename... Cmpts>
		constexpr bool IsContainAny(TypeList<Cmpts...>) const {
			return IsContainAny<Cmpts...>();
		}

		template<typename CmptTypeContainer>
		bool IsContainAny(const CmptTypeContainer& types) const {
			if (types.empty())
				return true;

			for (auto type : types) {
				if (IsContain(type))
					return true;
			}

			return false;
		}

		template<typename... Cmpts>
		constexpr bool IsNotContain() const {
			if constexpr (sizeof...(Cmpts) == 0)
				return true;
			else
				return ((find(CmptType::Of<Cmpts>()) == end()) &&...);
		}

		template<typename... Cmpts>
		constexpr bool IsNotContain(TypeList<Cmpts...>) const {
			return IsNotContain<Cmpts...>();
		}

		bool IsNotContain(CmptType type) const {
			return find(type) == cend();
		}

		template<typename CmptTypeContainer>
		bool IsNotContain(const CmptTypeContainer& types) const {
			for (auto type : types) {
				if (IsContain(type))
					return false;
			}
			return true;
		}

		bool IsMatch(const EntityFilter& filter) const {
			return IsContain(filter.AllCmptTypes())
				&& IsContainAny(filter.AnyCmptTypes())
				&& IsNotContain(filter.NoneCmptTypes());
		}

		bool IsMatch(const EntityLocator& locator) const {
			return IsContain(locator.CmptTypes());
		}

		bool IsMatch(const EntityQuery& query) const {
			return IsMatch(query.filter) && IsMatch(query.locator);
		}

		template<typename... Cmpts>
		bool Is() const {
			return sizeof...(Cmpts) == size() && IsContain<Cmpts...>();
		}

		using std::set<CmptType>::begin;
		using std::set<CmptType>::end;
		using std::set<CmptType>::size;

		friend bool operator<(const CmptTypeSet& x, const CmptTypeSet& y) {
			return static_cast<const std::set<CmptType>&>(x) < static_cast<const std::set<CmptType>&>(y);
		}
		friend bool operator==(const CmptTypeSet& x, const CmptTypeSet& y) {
			return static_cast<const std::set<CmptType>&>(x) == static_cast<const std::set<CmptType>&>(y);
		}

	private:
		template<typename... Cmpts>
		static constexpr size_t HashCodeOf(TypeList<Cmpts...>) {
			size_t seed = TypeID<CmptTypeSet>;
			((seed = hash_combine(seed, CmptType::HashCodeOf<Cmpts>())), ...);
			return seed;
		}

	};
}
