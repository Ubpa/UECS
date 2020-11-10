#pragma once

#include "CmptType.h"

#include <cassert>

namespace Ubpa::UECS {
	// CmptType + void*
	class CmptPtr {
	public:
		constexpr CmptPtr(CmptType type, void* p) noexcept : type{ type }, p{ p } {}
		template<typename Cmpt>
		constexpr CmptPtr(Cmpt* p) noexcept : type{ CmptType::Of<Cmpt> }, p{ p } {}
		constexpr CmptPtr() noexcept : CmptPtr{ Invalid() } {}

		constexpr void* Ptr() const noexcept { return p; }

		constexpr CmptType Type() const noexcept { return type; }

		static constexpr CmptPtr Invalid() noexcept { return { CmptType::Invalid(), nullptr }; };
		constexpr bool Valid() const noexcept { return p != nullptr && type.Valid(); }

		template<typename Cmpt>
		constexpr Cmpt* As() const noexcept { return reinterpret_cast<Cmpt*>(p); }
	private:
		CmptType type;
		void* p;
	};

	// CmptAccessType + void*
	class CmptAccessPtr {
	public:
		constexpr CmptAccessPtr(CmptType type, void* p, AccessMode mode) noexcept : accessType{ type, mode }, p{ p } {}
		constexpr CmptAccessPtr(CmptAccessType accessType, void* p) noexcept : accessType{ accessType }, p{ p } {}
		constexpr CmptAccessPtr(CmptPtr p, AccessMode mode) noexcept : accessType{ p.Type(), mode }, p{ p.Ptr() } {}
		template<typename TaggedCmpt>
		constexpr CmptAccessPtr(TaggedCmpt p) noexcept : accessType{ CmptAccessType::Of<TaggedCmpt> }, p{ CastToVoidPointer(p) } {}
		explicit constexpr CmptAccessPtr(CmptPtr p) noexcept : CmptAccessPtr{ p, AccessMode::LATEST } {}
		explicit constexpr CmptAccessPtr() noexcept : CmptAccessPtr{ Invalid() } {}

		explicit constexpr operator CmptPtr() const noexcept { return { CmptType{accessType}, p }; }

		constexpr void* Ptr() const noexcept { return p; }

		constexpr CmptAccessType AccessType() const noexcept { return accessType; }

		static constexpr CmptAccessPtr Invalid() noexcept { return { CmptAccessType::Invalid(), nullptr }; };
		constexpr bool Valid() const noexcept { return p != nullptr && accessType.Valid(); }

		// check: type's access mode must be equal to <mode>
		template<typename Cmpt, AccessMode mode>
		constexpr auto As() const noexcept {
			assert(accessType.GetAccessMode() == mode);
			if constexpr (mode == AccessMode::LAST_FRAME)
				return LastFrame<Cmpt>{p};
			else if constexpr (mode == AccessMode::WRITE)
				return Write<Cmpt>{p};
			else if constexpr (mode == AccessMode::LATEST)
				return Latest<Cmpt>{p};
			else if constexpr (mode == AccessMode::LAST_FRAME_SINGLETON)
				return LastFrame<Singleton<Cmpt>>{p};
			else if constexpr (mode == AccessMode::WRITE_SINGLETON)
				return Write<Singleton<Cmpt>>{p};
			else if constexpr (mode == AccessMode::LATEST_SINGLETON)
				return Latest<Singleton<Cmpt>>{p};
			else
				static_assert(false);
		}
	private:
		friend class EntityMngr;
		CmptAccessType accessType;
		void* p;
	};
}
