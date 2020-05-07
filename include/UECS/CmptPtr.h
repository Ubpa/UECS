#pragma once

#include "CmptType.h"

#include <cassert>

namespace Ubpa {
	class CmptPtr {
	public:
		CmptPtr(CmptType type, void* p) :type{ type }, p{ p }{}
		template<typename Cmpt>
		CmptPtr(Cmpt* p) : type{ CmptType::Of<Cmpt>() }, p{ p }{}

		template<typename Cmpt>
		Cmpt* As() const noexcept {
			assert(type.HashCode() == TypeID<Cmpt>);
			return reinterpret_cast<Cmpt*>(p);
		}
		CmptType& Type() noexcept { return type; }
		CmptType Type() const noexcept { return type; }
		void*& Ptr() noexcept { return p; }
		void* Ptr() const noexcept { return p; }
	private:
		CmptType type;
		void* p;
	};

	class CmptCPtr {
	public:
		CmptCPtr(CmptType type, const void* p) :type{ type }, p{ p }{}
		template<typename Cmpt>
		CmptCPtr(const Cmpt* p) : type{ CmptType::Of<Cmpt>() }, p{ p }{}

		template<typename Cmpt>
		const Cmpt* As() const noexcept {
			assert(type.HashCode() == TypeID<Cmpt>);
			return reinterpret_cast<Cmpt*>(p);
		}
		CmptType& Type() noexcept { return type; }
		CmptType Type() const noexcept { return type; }
		const void*& Ptr() noexcept { return p; }
		const void* Ptr() const noexcept { return p; }
	private:
		CmptType type;
		const void* p;
	};
}
