#pragma once

namespace Ubpa::UECS {
	inline void CmptTypeSet::Insert(const CmptType* types, size_t num) {
		assert(types || num == 0);
		for (size_t i = 0; i < num; i++)
			data.insert(types[i]);
	}

	inline void CmptTypeSet::Erase(const CmptType* types, size_t num) noexcept {
		assert(types || num == 0);
		for (size_t i = 0; i < num; i++)
			data.erase(types[i]);
	}

	inline bool CmptTypeSet::Contains(CmptType type) const {
		return data.find(type) != data.end();
	}

	inline bool CmptTypeSet::ContainsAll(const CmptType* types, size_t num) const {
		assert(types || num == 0);
		for (size_t i = 0; i < num; i++) {
			if (!Contains(types[i]))
				return false;
		}
		return true;
	}

	template<typename CmptTypeContainer>
	bool CmptTypeSet::ContainsAll(const CmptTypeContainer& types) const {
		for (const auto& type : types) {
			if (!Contains(type))
				return false;
		}
		return true;
	}

	inline bool CmptTypeSet::ContainsAny(const CmptType* types, size_t num) const {
		assert(types || num == 0);
		for (size_t i = 0; i < num; i++) {
			if (Contains(types[i]))
				return true;
		}
		return false;
	}

	template<typename CmptTypeContainer>
	bool CmptTypeSet::ContainsAny(const CmptTypeContainer& types) const {
		if (types.empty())
			return true;

		for (const auto& type : types) {
			if (Contains(type))
				return true;
		}

		return false;
	}

	template<typename CmptTypeContainer>
	bool CmptTypeSet::NotContain(const CmptTypeContainer& types) const {
		for (const auto& type : types) {
			if (Contains(type))
				return false;
		}
		return true;
	}

	inline bool CmptTypeSet::IsMatch(const ArchetypeFilter& filter) const {
		return ContainsAll(filter.all)
			&& ContainsAny(filter.any)
			&& NotContain(filter.none);
	}

	inline bool CmptTypeSet::IsMatch(const CmptLocator& locator) const {
		for (const auto& t : locator.CmptAccessTypes()) {
			if (!Contains(t))
				return false;
		}
		return true;
	}

	inline bool CmptTypeSet::IsMatch(const EntityQuery& query) const {
		return IsMatch(query.filter) && IsMatch(query.locator);
	}

	inline size_t CmptTypeSet::HashCode() const noexcept {
		size_t seed = TypeID<CmptTypeSet>;
		for (const auto& t : data)
			seed = hash_combine(seed, t.HashCode());
		return seed;
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
