#pragma	once

#include <UTemplate/TypeID.h>
#include <UTemplate/TemplateList.h>

#include <set>

namespace Ubpa {
	class CmptIDSet : std::set<size_t> {
	public:
		CmptIDSet() = default;
		template<typename... Cmpts>
		CmptIDSet(TypeList<Cmpts...>) : std::set<size_t>{ TypeID<Cmpts>... }{}

		template<typename... Cmpts>
		void Add() { (insert(TypeID<Cmpts>), ...); }
		template<typename... Cmpts>
		void Remove() noexcept { (erase(TypeID<Cmpts>), ...); }

		template<typename... Cmpts>
		constexpr bool IsContain() const {
			if constexpr (sizeof...(Cmpts) == 0)
				return true;
			else
				return ((find(TypeID<Cmpts>) != end()) &&...);
		}

		template<typename... Cmpts>
		constexpr bool IsContain(TypeList<Cmpts...>) const {
			return IsContain<Cmpts...>();
		}

		bool IsContain(size_t cmptHash) const {
			return find(cmptHash) != end();
		}

		template<typename IDContainer>
		bool IsContain(const IDContainer& ids) const {
			for (auto id : ids) {
				if (!IsContain(id))
					return false;
			}
			return true;
		}

		template<typename... Cmpts>
		constexpr bool IsContainAny() const {
			if constexpr (sizeof...(Cmpts) == 0)
				return true;
			else
				return ((find(TypeID<Cmpts>) != end()) ||...);
		}

		template<typename... Cmpts>
		constexpr bool IsContainAny(TypeList<Cmpts...>) const {
			return IsContainAny<Cmpts...>();
		}

		template<typename IDContainer>
		bool IsContainAny(const IDContainer& ids) const {
			if (ids.empty())
				return true;

			for (auto id : ids) {
				if (IsContain(id))
					return true;
			}

			return false;
		}

		template<typename... Cmpts>
		constexpr bool IsNotContain() const {
			if constexpr (sizeof...(Cmpts) == 0)
				return true;
			else
				return ((find(TypeID<Cmpts>) == end()) &&...);
		}

		template<typename... Cmpts>
		constexpr bool IsNotContain(TypeList<Cmpts...>) const {
			return IsNotContain<Cmpts...>();
		}

		bool IsNotContain(size_t cmptHash) const {
			return find(cmptHash) == end();
		}

		template<typename IDContainer>
		bool IsNotContain(const IDContainer& ids) const {
			for (auto id : ids) {
				if (IsContain(id))
					return false;
			}
			return true;
		}

		template<typename AllList, typename AnyList, typename NotList, typename LocateList>
		bool IsMatch() const {
			return IsContain(AllList{})
				&& IsContainAny(AnyList{})
				&& IsNotContain(NotList{})
				&& IsContain(LocateList{});
		}

		template<typename... Cmpts>
		bool Is() const {
			return sizeof...(Cmpts) == size() && IsContain<Cmpts...>();
		}

		using std::set<size_t>::begin;
		using std::set<size_t>::end;
		using std::set<size_t>::size;

		friend bool operator<(const CmptIDSet& x, const CmptIDSet& y) {
			return static_cast<const std::set<size_t>&>(x) < static_cast<const std::set<size_t>&>(y);
		}
		friend bool operator==(const CmptIDSet& x, const CmptIDSet& y) {
			return static_cast<const std::set<size_t>&>(x) == static_cast<const std::set<size_t>&>(y);
		}
	};
}
