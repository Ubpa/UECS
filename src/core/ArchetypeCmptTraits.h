#pragma once

#include <UTemplate/Type.h>

#include <small_vector.h>

#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <tuple>

namespace Ubpa::UECS {
	class RTDCmptTraits;

	// run-time static component traits
	class ArchetypeCmptTraits {
	public:
		bool IsTrivial(TypeID) const;
		std::size_t Sizeof(TypeID) const;
		std::size_t Alignof(TypeID) const;
		void CopyConstruct(TypeID, void* dst, void* src) const;
		void MoveConstruct(TypeID, void* dst, void* src) const;
		void MoveAssign(TypeID, void* dst, void* src) const;
		void Destruct(TypeID, void* cmpt) const;

		struct CmptTraits {
			TypeID ID;
			bool trivial;
			size_t size;
			size_t alignment;
			std::function<void(void*, void*)> copy_ctor; // dst <- src
			std::function<void(void*, void*)> move_ctor; // dst <- src
			std::function<void(void*, void*)> move_assign; // dst <- src
			std::function<void(void*)> dtor;
		};

		const CmptTraits* GetTraits(TypeID ID) const noexcept;

		void Register(const RTDCmptTraits&, TypeID);

		void Deregister(TypeID) noexcept;

	private:
		small_vector<CmptTraits, 16> cmpt_traits;
	};
}
