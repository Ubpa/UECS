#pragma once

#include <UTemplate/TypeID.h>

#include <cassert>

namespace Ubpa {
	struct CmptPtr {
		const size_t id;

		CmptPtr(size_t id, void* p) :id{ id }, p{ p }{}
		template<typename Cmpt>
		CmptPtr(Cmpt* p) : id{ TypeID<Cmpt> }, p{ p }{}

		template<typename Cmpt>
		Cmpt* As() const noexcept {
			assert(id == TypeID<Cmpt>);
			return reinterpret_cast<Cmpt*>(p);
		}

		template<typename Cmpt>
		const Cmpt* As() const noexcept {
			return const_cast<CmptPtr*>(this)->As<Cmpt>();
		}

		void* Ptr() noexcept { return p; }
		const void* Ptr() const noexcept { return p; }

	private:
		void* const p;
	};
}
