#pragma once

#include <UTemplate/TypeID.h>

#include <cassert>

namespace Ubpa {
	class CmptPtr {
	public:
		CmptPtr(size_t id, void* p) :id{ id }, p{ p }{}
		template<typename Cmpt>
		CmptPtr(Cmpt* p) : id{ TypeID<Cmpt> }, p{ p }{}

		template<typename Cmpt>
		Cmpt* As() const noexcept {
			assert(id == TypeID<Cmpt>);
			return reinterpret_cast<Cmpt*>(p);
		}
		size_t ID() const noexcept { return id; }
		void* Ptr() const noexcept { return p; }
	private:
		size_t id;
		void* p;
	};

	class CmptCPtr {
	public:
		CmptCPtr(size_t id, const void* p) :id{ id }, p{ p }{}
		template<typename Cmpt>
		CmptCPtr(const Cmpt* p) : id{ TypeID<Cmpt> }, p{ p }{}

		template<typename Cmpt>
		const Cmpt* As() const noexcept {
			assert(id == TypeID<Cmpt>);
			return reinterpret_cast<const Cmpt*>(p);
		}
		size_t ID() const noexcept { return id; }
		const void* Ptr() const noexcept { return p; }
	private:
		size_t id;
		const void* p;
	};
}
