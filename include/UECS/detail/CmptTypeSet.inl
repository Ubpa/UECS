#pragma once

namespace Ubpa::UECS {
	inline CmptTypeSet::CmptTypeSet(const CmptType* types, size_t num) {
		assert(types != nullptr && num != 0);
		for (size_t i = 0; i < num; i++)
			insert(types[i]);
		hashcode = HashCodeOf(*this);
	}

	template<typename... Cmpts>
	CmptTypeSet::CmptTypeSet(TypeList<Cmpts...>)
		: std::set<CmptType>{ CmptType::Of<Cmpts>... }, hashcode{ HashCodeOf<Cmpts...>() } {}

	template<typename... Cmpts>
	constexpr size_t CmptTypeSet::HashCodeOf() noexcept {
		return HashCodeOf(QuickSort_t<TypeList<Cmpts...>, TypeID_Less>{});
	}

	template<typename... CmptTypes>
	void CmptTypeSet::Insert(CmptTypes... types) {
		static_assert((std::is_same_v<CmptTypes, CmptType> &&...));
		(insert(types), ...);
		hashcode = HashCodeOf(*this);
	}

	template<typename... CmptTypes>
	void CmptTypeSet::Erase(CmptTypes... types) noexcept {
		static_assert((std::is_same_v<CmptTypes, CmptType> &&...));
		(erase(types), ...);
		hashcode = HashCodeOf(*this);
	}

	inline bool CmptTypeSet::Contains(CmptType type) const {
		return find(type) != cend();
	}

	template<typename CmptTypeContainer>
	bool CmptTypeSet::Contains(const CmptTypeContainer& types) const {
		for (auto type : types) {
			if (!Contains(type))
				return false;
		}
		return true;
	}

	template<typename CmptTypeContainer>
	bool CmptTypeSet::ContainsAny(const CmptTypeContainer& types) const {
		if (types.empty())
			return true;

		for (auto type : types) {
			if (Contains(type))
				return true;
		}

		return false;
	}

	template<typename CmptTypeContainer>
	bool CmptTypeSet::NotContain(const CmptTypeContainer& types) const {
		for (auto type : types) {
			if (Contains(type))
				return false;
		}
		return true;
	}

	inline bool CmptTypeSet::IsMatch(const ArchetypeFilter& filter) const {
		return Contains(filter.AllCmptTypes())
			&& ContainsAny(filter.AnyCmptTypes())
			&& NotContain(filter.NoneCmptTypes());
	}

	inline bool CmptTypeSet::IsMatch(const CmptLocator& locator) const {
		for (const auto& t : locator.CmptTypes()) {
			if (!Contains(t))
				return false;
		}
		return true;
	}

	inline bool CmptTypeSet::IsMatch(const EntityQuery& query) const {
		return IsMatch(query.filter) && IsMatch(query.locator);
	}

	template<typename... Cmpts>
	bool CmptTypeSet::Is() const {
		static_assert(IsSet_v<Cmpts>);
		return sizeof...(Cmpts) == size() && (Contains(CmptType::Of<Cmpts>) &&...);
	}

	template<typename... Cmpts>
	static constexpr size_t CmptTypeSet::HashCodeOf(TypeList<Cmpts...>) noexcept {
		return HashCodeOf(std::array<CmptType, sizeof...(Cmpts)>{CmptType::Of<Cmpts>...});
	}

	template<typename Container>
	static constexpr size_t CmptTypeSet::HashCodeOf(const Container& cmpts) noexcept {
		size_t seed = TypeID<CmptTypeSet>;
		for (const CmptType& cmpt : cmpts)
			seed = hash_combine(seed, cmpt.HashCode());
		return seed;
	}

	inline bool CmptTypeSet::operator==(const CmptTypeSet& rhs) const {
		return static_cast<const std::set<CmptType>&>(*this) == static_cast<const std::set<CmptType>&>(rhs);
	}
}

namespace std {
	template<>
	struct hash<Ubpa::UECS::CmptTypeSet> {
		size_t operator()(const Ubpa::UECS::CmptTypeSet& types) const noexcept {
			return types.HashCode();
		}
	};
}
