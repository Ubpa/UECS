#pragma once

#include "AccessTypeID.hpp"

#include <memory_resource>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <string>

namespace Ubpa::UECS {
	class EntityMngr;

	// run-time dynamic component traits
	// size (> 0) is neccessary
	// optional
	// - alignment: alignof(std::max_align_t) as default, 8 / 16 in most cases
	// - default constructor: do nothing as default
	// - copy constructor: memcpy as default
	// - move constructor: memcpy as default
	// - move assignment: memcpy as default
	// - destructor: do nothing as default
	// - name
	class CmptTraits {
	public:
		static constexpr std::size_t default_alignment = alignof(std::max_align_t);

		CmptTraits& Clear();

		CmptTraits& RegisterName(Type);
		CmptTraits& RegisterTrivial(TypeID);
		CmptTraits& RegisterSize(TypeID, std::size_t size);
		CmptTraits& RegisterAlignment(TypeID, std::size_t alignment);
		CmptTraits& RegisterDefaultConstructor(TypeID, std::function<void(void*, std::pmr::memory_resource*)>);
		CmptTraits& RegisterCopyConstructor(TypeID, std::function<void(void*,const void*, std::pmr::memory_resource*)>);
		CmptTraits& RegisterMoveConstructor(TypeID, std::function<void(void*,void*)>);
		CmptTraits& RegisterMoveAssignment(TypeID, std::function<void(void*,void*)>);
		CmptTraits& RegisterDestructor(TypeID, std::function<void(void*)>);

		const auto& GetSizeofs() const noexcept { return sizeofs; }
		const auto& GetAlignments() const noexcept { return alignments; };
		const auto& GetDefaultConstructors() const noexcept { return default_constructors; }
		const auto& GetCopyConstructors() const noexcept { return copy_constructors; }
		const auto& GetMoveConstructors() const noexcept { return move_constructors; }
		const auto& GetMoveAssignments() const noexcept { return move_assignments; }
		const auto& GetDestructors() const noexcept { return destructors; }
		const auto& GetNames() const noexcept { return names; }

		bool IsTrivial(TypeID) const;
		std::size_t Sizeof(TypeID) const;
		std::size_t Alignof(TypeID) const;
		void DefaultConstruct(TypeID, void* cmpt) const;
		void CopyConstruct(TypeID, void* dst, const void* src) const;
		void MoveConstruct(TypeID, void* dst, void* src) const;
		void MoveAssign(TypeID, void* dst, void* src) const;
		void Destruct(TypeID, void* cmpt) const;
		std::string_view Nameof(TypeID) const;

		CmptTraits& Deregister(TypeID) noexcept;

		template<typename... Cmpts>
		void Register();

		template<typename... Cmpts>
		void UnsafeRegister();

		template<typename Cmpt>
		void Deregister();

	private:
		friend class EntityMngr;
		CmptTraits(std::pmr::memory_resource* world_rsrc);
		CmptTraits(const CmptTraits& other, std::pmr::memory_resource* world_rsrc);
		CmptTraits(CmptTraits&& other) noexcept;

		// register all for Cmpt
		// static_assert
		// - is_default_constructible_v
		// - is_copy_constructible_v || std::is_constructible_v<Cmpt, Cmpt&>
		// - is_move_constructible_v
		// - is_move_assignable_v
		// - is_destructible_v
		template<typename Cmpt>
		void RegisterOne();

		template<typename Cmpt>
		void UnsafeRegisterOne();

		std::unique_ptr<std::pmr::unsynchronized_pool_resource> rsrc;
		std::unordered_set<TypeID> trivials;
		std::unordered_map<TypeID, std::string_view> names;
		std::unordered_map<TypeID, std::size_t> sizeofs;
		std::unordered_map<TypeID, std::size_t> alignments;
		std::unordered_map<TypeID, std::function<void(void*, std::pmr::memory_resource*)>> default_constructors; // dst <- src
		std::unordered_map<TypeID, std::function<void(void*, const void*, std::pmr::memory_resource*)>> copy_constructors; // dst <- src
		std::unordered_map<TypeID, std::function<void(void*, void*)>> move_constructors; // dst <- src
		std::unordered_map<TypeID, std::function<void(void*, void*)>> move_assignments; // dst <- src
		std::unordered_map<TypeID, std::function<void(void*)>> destructors;

		std::pmr::memory_resource* world_rsrc;
	};
}

#include "details/CmptTraits.inl"
