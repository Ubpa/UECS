#pragma once

#include <UTemplate/TypeID.h>

#include <unordered_map>
#include <functional>

namespace Ubpa {
	class RuntimeCmptTraits {
	public:
		static RuntimeCmptTraits& Instance() {
			static RuntimeCmptTraits instance;
			return instance;
		}

		void Destruct(size_t id, void* cmpt) const {
			auto target = destructors.find(id);
			if (target != destructors.end())
				target->second(cmpt);
		}

		void MoveConstruct(size_t id, size_t size, void* dst, void* src) const {
			auto target = move_constructors.find(id);

			if (target != move_constructors.end())
				target->second(dst, src);
			else
				memcpy(dst, src, size);
		}

		template<typename Cmpt>
		void Register() {
			static_assert(std::is_move_constructible_v<Cmpt>);
			static_assert(std::is_destructible_v<Cmpt>);

			if constexpr (!std::is_trivially_destructible_v<Cmpt>) {
				destructors[TypeID<Cmpt>] = [](void* cmpt) {
					reinterpret_cast<Cmpt*>(cmpt)->~Cmpt();
				};
			}
			if constexpr (!std::is_trivially_move_constructible_v<Cmpt>) {
				move_constructors[TypeID<Cmpt>] = [](void* dst, void* src) {
					new(dst)Cmpt(std::move(*reinterpret_cast<Cmpt*>(src)));
				};
			}
		}

	private:
		std::unordered_map<size_t, std::function<void(void*)>> destructors;
		std::unordered_map<size_t, std::function<void(void*, void*)>> move_constructors; // dst <- src

		RuntimeCmptTraits() = default;
	};
}
