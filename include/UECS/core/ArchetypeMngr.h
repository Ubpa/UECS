#pragma once

#include "Archetype.h"

#include <UTemplate/Typelist.h>

namespace Ubpa {
	class World;
}

namespace Ubpa::detail {
	class ArcheTypeMngr {
	public:
		ArcheTypeMngr(World* w):w(w){}
		inline World* World() const noexcept { return w; }

		template<typename... Cmpts>
		static constexpr size_t HashOf() noexcept {
			return Ubpa::TypeID<QuickSort_t<TypeList<Cmpts...>, Ubpa::TypeID_Less>>;
		}

		inline ArcheType& GetArcheTypeOf(size_t ArcheTypeHash) {
			auto target = map.find(ArcheTypeHash);
			assert(target != map.end());
			return target->second;
		}

		template<typename... Cmpts>
		inline ArcheType& GetArcheTypeOf() {
			constexpr size_t ArcheTypeHash = Ubpa::TypeID<QuickSort_t<TypeList<Cmpts...>, Ubpa::TypeID_Less>>;
			auto target = map.find(ArcheTypeHash);
			if (target == map.end()) {
				ArcheType& c = map[ArcheTypeHash];
				c.Init<Cmpts...>();
				return c;
			}
			else
				return target->second;
		}

		template<typename... Cmpts>
		const std::vector<ArcheType*> GetArcheTypeWith() {
			std::vector<ArcheType*> rst;
			if constexpr (sizeof...(Cmpts) == 0) {
				for (auto& p : map)
					rst.push_back(&p.second);
				return rst;
			}
			else {
				for (auto& p : map) {
					if (p.second.IsContain<Cmpts...>())
						rst.push_back(&p.second);
				}
				return rst;
			}
		}

	private:
		Ubpa::World* w;
		std::map<size_t, ArcheType> map;
	};
}