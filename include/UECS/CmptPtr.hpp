#pragma once

#include "AccessTypeID.hpp"

#include <cassert>

namespace Ubpa::UECS {
	// TypeID + void*
	class CmptPtr {
	public:
		constexpr CmptPtr(TypeID type, void* p) noexcept : type{ type }, p{ p } {}
		template<typename Cmpt>
		constexpr CmptPtr(Cmpt* p) noexcept : type{ TypeID_of<Cmpt> }, p{ p } {}
		constexpr CmptPtr() noexcept : CmptPtr{ Invalid() } {}

		constexpr void* Ptr() const noexcept { return p; }

		constexpr TypeID Type() const noexcept { return type; }

		static constexpr CmptPtr Invalid() noexcept { return { TypeID{}, nullptr }; };
		constexpr bool Valid() const noexcept { return p != nullptr && type.Valid(); }

		template<typename Cmpt>
		constexpr Cmpt* As() const noexcept { return reinterpret_cast<Cmpt*>(p); }
	private:
		TypeID type;
		void* p;
	};

	// AccessTypeID + void*
	class CmptAccessPtr {
	public:
		constexpr CmptAccessPtr(TypeID type, void* p, AccessMode mode) noexcept : accessType{ type, mode }, p{ p } {}
		constexpr CmptAccessPtr(AccessTypeID accessType, void* p) noexcept : accessType{ accessType }, p{ p } {}
		constexpr CmptAccessPtr(CmptPtr p, AccessMode mode) noexcept : accessType{ p.Type(), mode }, p{ p.Ptr() } {}
		template<typename TaggedCmpt>
		constexpr CmptAccessPtr(TaggedCmpt p) noexcept : accessType{ AccessTypeID_of<TaggedCmpt> }, p{ CastToVoidPointer(p) } {}
		explicit constexpr CmptAccessPtr(CmptPtr p) noexcept : CmptAccessPtr{ p, AccessMode::LATEST } {}
		explicit constexpr CmptAccessPtr() noexcept : CmptAccessPtr{ Invalid() } {}

		explicit constexpr operator CmptPtr() const noexcept { return { TypeID{accessType}, p }; }

		constexpr void* Ptr() const noexcept { return p; }

		constexpr AccessTypeID AccessType() const noexcept { return accessType; }

		static constexpr CmptAccessPtr Invalid() noexcept { return { AccessTypeID{}, nullptr }; };
		constexpr bool Valid() const noexcept { return p != nullptr && accessType.Valid(); }

		// check: type's access mode must be equal to <mode>
		template<typename Cmpt, AccessMode mode>
		constexpr auto As() const noexcept {
			assert(accessType.GetAccessMode() == mode);
			if constexpr (mode == AccessMode::LAST_FRAME)
				return LastFrame<Cmpt>{reinterpret_cast<Cmpt*>(p)};
			else if constexpr (mode == AccessMode::WRITE)
				return Write<Cmpt>{reinterpret_cast<Cmpt*>(p)};
			else if constexpr (mode == AccessMode::LATEST)
				return Latest<Cmpt>{reinterpret_cast<Cmpt*>(p)};
			else
				static_assert(always_false<Cmpt>);
		}
	private:
		friend class EntityMngr;
		AccessTypeID accessType;
		void* p;
	};
}
