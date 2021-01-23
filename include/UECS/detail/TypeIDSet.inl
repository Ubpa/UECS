#pragma once

namespace Ubpa::UECS {
	inline void TypeIDSet::Insert(std::span<const TypeID> types) {
		for(const auto& type : types)
			data.insert(type);
	}

	inline void TypeIDSet::Erase(std::span<const TypeID> types) noexcept {
		for (const auto& type : types)
			data.erase(type);
	}

	inline bool TypeIDSet::Contains(TypeID type) const {
		return data.find(type) != data.end();
	}

	inline bool TypeIDSet::ContainsAll(std::span<const TypeID> types) const {
		for (const auto& type : types) {
			if (!Contains(type))
				return false;
		}
		return true;
	}

	template<typename TypeIDContainer>
	bool TypeIDSet::ContainsAll(const TypeIDContainer& types) const {
		for (const auto& type : types) {
			if (!Contains(type))
				return false;
		}
		return true;
	}

	inline bool TypeIDSet::ContainsAny(std::span<const TypeID> types) const {
		for (const auto& type : types) {
			if (Contains(type))
				return true;
		}
		return false;
	}

	template<typename TypeIDContainer>
	bool TypeIDSet::ContainsAny(const TypeIDContainer& types) const {
		if (types.empty())
			return true;

		for (const auto& type : types) {
			if (Contains(type))
				return true;
		}

		return false;
	}

	template<typename TypeIDContainer>
	bool TypeIDSet::NotContain(const TypeIDContainer& types) const {
		for (const auto& type : types) {
			if (Contains(type))
				return false;
		}
		return true;
	}

	inline bool TypeIDSet::IsMatch(const ArchetypeFilter& filter) const {
		return ContainsAll(filter.all)
			&& ContainsAny(filter.any)
			&& NotContain(filter.none);
	}

	inline bool TypeIDSet::IsMatch(const CmptLocator& locator) const {
		for (const auto& t : locator.AccessTypeIDs()) {
			if (!Contains(t))
				return false;
		}
		return true;
	}

	inline bool TypeIDSet::IsMatch(const EntityQuery& query) const {
		return IsMatch(query.filter) && IsMatch(query.locator);
	}

	inline std::size_t TypeIDSet::GetValue() const noexcept {
		std::size_t seed = TypeID_of<TypeIDSet>.GetValue();
		for (const auto& t : data)
			seed = hash_combine(seed, t.GetValue());
		return seed;
	}
}

namespace std {
	template<>
	struct hash<Ubpa::UECS::TypeIDSet> {
		std::size_t operator()(const Ubpa::UECS::TypeIDSet& types) const noexcept {
			return types.GetValue();
		}
	};
}
