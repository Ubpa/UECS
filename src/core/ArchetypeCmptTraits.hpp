#pragma once

#include <UTemplate/Type.hpp>

#include <USmallFlat/small_flat_set.hpp>

#include <functional>
#include <span>
#include <memory_resource>

namespace Ubpa::UECS {
	class CmptTraits;
	struct EntityQuery;

	// run-time static component traits
	class ArchetypeCmptTraits {
	public:
		struct CmptTrait {
			bool trivial;
			std::size_t size;
			std::size_t alignment;

			std::function<void(void*, std::pmr::memory_resource*)> default_ctor;
			std::function<void(void*, const void*, std::pmr::memory_resource*)> copy_ctor; // dst <- src
			std::function<void(void*, void*, std::pmr::memory_resource*)> move_ctor; // dst <- src
			std::function<void(void*, void*)> move_assign; // dst <- src
			std::function<void(void*)> dtor;

			void DefaultConstruct(void* cmpt, std::pmr::memory_resource*) const;
			void CopyConstruct(void* dst, const void* src, std::pmr::memory_resource*) const;
			void MoveConstruct(void* dst, void* src, std::pmr::memory_resource*) const;
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

		void Register(const CmptTraits&, TypeID);

		void Deregister(TypeID) noexcept;

	private:
		bool trivial{ true };
		small_flat_set<TypeID> types;
		small_vector<CmptTrait> cmpt_traits;
	};
}
