#pragma once

#include <tuple>
#include <map>
#include <functional>

namespace Ubpa {
	class Archetype;

	// tuple use for compare and hash
	struct EntityData : std::tuple<Archetype*, size_t> {
		EntityData(Archetype* archetype = nullptr, size_t idx = static_cast<size_t>(-1))
			: std::tuple<Archetype*, size_t>{ archetype,idx } {}

		~EntityData() {
			for (auto p : releases)
				p.second(p.first);
			releases.clear();
		}

		inline Archetype*& archetype() noexcept { return std::get<0>(*this); }
		inline const Archetype* archetype() const noexcept { return std::get<0>(*this); }

		inline size_t& idx() noexcept { return std::get<1>(*this); }
		inline size_t idx() const noexcept { return std::get<1>(*this); }

		/*void Deregist(void* cmpt) {
			assert(releases.find(cmpt) != releases.end());
			releases.erase(cmpt);
		}*/

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
