#pragma once

#include "CmptType.h"

#include <cassert>

namespace Ubpa::UECS {
	// CmptType + void*
	class CmptPtr {
	public:
		CmptPtr(CmptType type, void* p) :type{ type }, p{ p }{}
		template<typename Cmpt>
		CmptPtr(Cmpt* p) : type{ CmptType::Of<Cmpt> }, p{ p }{}

		CmptType Type() const noexcept { return type; }

		// unchecked
		void* Ptr() const noexcept { return p; }

		// unchecked
		template<typename Cmpt>
		Cmpt* As() const noexcept { return reinterpret_cast<Cmpt*>(p); }

		template<typename Cmpt, AccessMode mode>
		auto As() const noexcept {
			if constexpr (mode == AccessMode::LAST_FRAME)
				return LastFrame<Cmpt>{p};
			else if constexpr (mode == AccessMode::WRITE)
				return Write<Cmpt>(p);
			else if constexpr (mode == AccessMode::LATEST)
				return Latest<Cmpt>(p);
			else if constexpr (mode == AccessMode::LAST_FRAME_SINGLETON)
				return LastFrame<Singleton<Cmpt>>;
			else if constexpr (mode == AccessMode::WRITE_SINGLETON)
				return Write<Singleton<Cmpt>>;
			else if constexpr (mode == AccessMode::LATEST_SINGLETON)
				return Latest<Singleton<Cmpt>>;
			else
				static_assert(false);
		}
	private:
		CmptType type;
		void* p;
	};
}
