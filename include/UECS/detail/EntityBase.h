#pragma once

#include <map>
#include <functional>

namespace Ubpa {
	class Archetype;

	struct EntityBase {
		Archetype* archetype{nullptr};
		size_t idx{ static_cast<size_t>(-1) };

		~EntityBase() {
			for (auto [ptr, cmptFuncs] : cmptFuncsMap)
				cmptFuncs.release(ptr);
			cmptFuncsMap.clear();
			archetype = nullptr;
			idx = static_cast<size_t>(-1);
		}

		bool operator<(const EntityBase& e) const noexcept {
			return archetype < e.archetype ||
				(archetype == e.archetype && idx < e.idx);
		}

	private:
		friend class ArchetypeMngr;
		friend class Archetype;

		template<typename Cmpt>
		void RegistCmptFuncs(Cmpt* cmpt) {
			CmptFuncs cmptFuncs;
			cmptFuncs.release = [](void* c) {
				reinterpret_cast<Cmpt*>(c)->~Cmpt();
			};

			if constexpr (std::is_move_constructible_v<Cmpt>) {
				// std::is_nothrow_move_constructible_v<Cmpt>
				// move
				cmptFuncs.move = [](void* src, void* dst) {
					auto srcCmpt = reinterpret_cast<Cmpt*>(src);
					new(dst)Cmpt(std::move(*srcCmpt));
				};
			}
			else {
				// trivial copy
				cmptFuncs.move = [](void* src, void* dst) {
					memcpy(dst, src, sizeof(Cmpt));
				};
			}
			cmptFuncsMap[reinterpret_cast<void*>(cmpt)] = cmptFuncs;
		}

		void MoveCmpt(void* src, void* dst) {
			auto target = cmptFuncsMap.find(src);
			assert(target != cmptFuncsMap.end());
			auto cmptFuncs = target->second;
			cmptFuncsMap.erase(target); // erase before change cmptFuncsMap
			cmptFuncs.move(src, dst);
			cmptFuncsMap[dst] = cmptFuncs;
		}

		void ReleaseCmpt(void* cmpt) {
			auto target = cmptFuncsMap.find(cmpt);
			assert(target != cmptFuncsMap.end());
			target->second.release(cmpt);
			cmptFuncsMap.erase(target);
		}

	private:
		struct CmptFuncs {
			std::function<void(void*)> release;
			std::function<void(void*, void*)> move; // src, dst
		};
		std::map<void*, CmptFuncs> cmptFuncsMap;
	};
}
