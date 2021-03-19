#pragma once

#include <UTemplate/Type.hpp>

#include <USmallFlat/small_vector.hpp>

#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <tuple>

namespace Ubpa::UECS {
	class RTDCmptTraits;

	// run-time static component traits
	class ArchetypeCmptTraits {
	public:
		struct CmptTrait {
			TypeID ID;
			bool trivial;
			std::size_t size;
			std::size_t alignment;
			std::size_t offset{ 0 }; // offset in chunk (include Entity)

			std::function<void(void*, void*)> copy_ctor; // dst <- src
			std::function<void(void*, void*)> move_ctor; // dst <- src
			std::function<void(void*, void*)> move_assign; // dst <- src
			std::function<void(void*)> dtor;

			void CopyConstruct(void* dst, void* src) const;
			void MoveConstruct(void* dst, void* src) const;
			void MoveAssign(void* dst, void* src) const;
			void Destruct(void* cmpt) const;
		};

		std::span<CmptTrait> GetTraits() noexcept { return { cmpt_traits.data(),cmpt_traits.size() }; }
		std::span<const CmptTrait> GetTraits() const noexcept { return { cmpt_traits.data(),cmpt_traits.size() }; }
		CmptTrait* GetTrait(TypeID ID) noexcept;
		const CmptTrait* GetTrait(TypeID ID) const noexcept
		{ return const_cast<ArchetypeCmptTraits*>(this)->GetTrait(ID); }

		void Register(const RTDCmptTraits&, TypeID);

		void Deregister(TypeID) noexcept;

	private:
		small_vector<CmptTrait, 16> cmpt_traits;
	};
}
