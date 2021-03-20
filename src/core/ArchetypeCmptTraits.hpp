#pragma once

#include <UTemplate/Type.hpp>

#include <USmallFlat/small_flat_set.hpp>

#include <functional>
#include <span>

namespace Ubpa::UECS {
	class RTDCmptTraits;
	struct EntityQuery;

	// run-time static component traits
	class ArchetypeCmptTraits {
	public:
		struct CmptTrait {
			bool trivial;
			std::size_t size;
			std::size_t alignment;

			std::function<void(void*)> default_ctor;
			std::function<void(void*, void*)> copy_ctor; // dst <- src
			std::function<void(void*, void*)> move_ctor; // dst <- src
			std::function<void(void*, void*)> move_assign; // dst <- src
			std::function<void(void*)> dtor;

			void DefaultConstruct(void* cmpt) const;
			void CopyConstruct(void* dst, void* src) const;
			void MoveConstruct(void* dst, void* src) const;
			void MoveAssign(void* dst, void* src) const;
			void Destruct(void* cmpt) const;
		};

		bool IsTrivial() const noexcept { return trivial; }
		small_flat_set<TypeID> GetTypes() noexcept { return types; }
		const small_flat_set<TypeID>& GetTypes() const noexcept { return types; }
		std::span<CmptTrait> GetTraits() noexcept { return { cmpt_traits.data(), cmpt_traits.size() }; }
		std::span<const CmptTrait> GetTraits() const noexcept { return { cmpt_traits.data(), cmpt_traits.size() }; }

		std::size_t GetTypeIndex(TypeID ID) const noexcept;

		CmptTrait& GetTrait(TypeID ID) noexcept;
		const CmptTrait& GetTrait(TypeID ID) const noexcept
		{ return const_cast<ArchetypeCmptTraits*>(this)->GetTrait(ID); }

		void Register(const RTDCmptTraits&, TypeID);

		void Deregister(TypeID) noexcept;

	private:
		bool trivial{ true };
		small_flat_set<TypeID> types;
		small_vector<CmptTrait> cmpt_traits;
	};
}
