#pragma once

#include <map>
#include <functional>

namespace Ubpa {
	class Archetype;

	struct EntityBase {
		Archetype* archetype{nullptr};
		size_t idx{ static_cast<size_t>(-1) };

		~EntityBase() {
			for (auto p : releases)
				p.second(p.first);
			releases.clear();
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
		void RegistCmptRelease(Cmpt* cmpt) {
			releases[cmpt] = [](void* c) {
				static_cast<Cmpt*>(c)->~Cmpt();
			};
		}

		void MoveCmpt(void* src, void* dst) {
			auto target = releases.find(src);
			assert(target != releases.end());
			auto release = target->second;
			releases.erase(target); // erase before change releases
			releases[dst] = release;
		}

		void ReleaseCmpt(void* cmpt) {
			auto target = releases.find(cmpt);
			assert(target != releases.end());
			target->second(cmpt);
			releases.erase(cmpt);
		}

	private:
		std::map<void*, std::function<void(void*)>> releases;
	};
}
