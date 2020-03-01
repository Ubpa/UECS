#pragma once

#include "Archetype.h"
#include "Component.h"

namespace Ubpa {
	class World;

	class Entity {
	public:
		template<typename Cmpt, typename... Args>
		inline void Init(Args... args) {
			archeType->Init<Cmpt>(ID, std::forward<Args>(args)...);
		}

		template<typename Cmpt>
		inline Cmpt& Get() {
			return archeType->Get<Cmpt>(ID);
		}

		inline bool IsAlive() const noexcept { return isAlive; }

	private:
		friend class World;
		bool isAlive{ false };
		detail::ArcheType* archeType;
		size_t ID;
	};
}
