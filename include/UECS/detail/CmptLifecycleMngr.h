#pragma once

#include <UTemplate/TypeID.h>

#include <type_traits>
#include <map>
#include <functional>

namespace Ubpa {
	class CmptLifecycleMngr {
	public:
		static CmptLifecycleMngr& Instance() {
			static CmptLifecycleMngr instance;
			return instance;
		}

		void Destruct(size_t id, void* cmpt) const {
			destructors.find(id)->second(cmpt);
		}

		void MoveConstruct(size_t id, void* dst, void* src) const {
			move_constructors.find(id)->second(dst, src);
		}

		template<typename Cmpt>
		void Register() {
			static_assert(std::is_move_constructible_v<Cmpt>);
			static_assert(std::is_constructible_v<Cmpt>);
			destructors[TypeID<Cmpt>] = [](void* cmpt) {
				reinterpret_cast<Cmpt*>(cmpt)->~Cmpt();
			};
			move_constructors[TypeID<Cmpt>] = [](void* dst, void* src) {
				new(dst)Cmpt(std::move(*reinterpret_cast<Cmpt*>(src)));
			};
		}

	private:
		std::map<size_t, std::function<void(void*)>> destructors;
		std::map<size_t, std::function<void(void*, void*)>> move_constructors; // dst <- src

		CmptLifecycleMngr() = default;
	};
}
