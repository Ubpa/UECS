#pragma once

#include <UTemplate/TypeID.h>

#include <type_traits>
#include <map>
#include <functional>

namespace Ubpa {
	class CmptMngr {
	public:
		static CmptMngr& Instance() {
			static CmptMngr instance;
			return instance;
		}

		void Destruct(size_t id, void* cmpt) const {
			destructors.find(id)->second(cmpt);
		}

		void MoveConstruct(size_t id, void* dst, void* src) const {
			moveconstructors.find(id)->second(dst, src);
		}

		template<typename Cmpt>
		bool Regist() {
			static bool rst = InnerRegist<Cmpt>(); // regist once
			return rst;
		}

	private:
		template<typename Cmpt>
		bool InnerRegist() {
			static_assert(std::is_move_constructible_v<Cmpt>);
			static_assert(std::is_constructible_v<Cmpt>);
			constexpr size_t id = TypeID<Cmpt>;
			destructors[id] = [](void* cmpt) {
				reinterpret_cast<Cmpt*>(cmpt)->~Cmpt();
			};
			moveconstructors[id] = [](void* dst, void* src) {
				new(dst)Cmpt(std::move(*reinterpret_cast<Cmpt*>(src)));
			};
			return true;
		}

	private:
		std::map<size_t, std::function<void(void*)>> destructors;
		std::map<size_t, std::function<void(void*, void*)>> moveconstructors; // dst <- src

		CmptMngr() = default;
	};
}