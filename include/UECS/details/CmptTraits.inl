#pragma once

#include "../CmptTraits.hpp"

namespace Ubpa::UECS::details {
	template<typename T>
	concept ContainPmrAlloc = requires() {
		typename T::allocator_type;
		std::is_constructible_v<typename T::allocator_type, std::pmr::memory_resource*>;
	};

	template<typename T>
	concept DefaultCtorWithAlloc = ContainPmrAlloc<T> && requires(const typename T::allocator_type & alloc) {
		new T(alloc);
	};

	template<typename T>
	concept CopyCtorWithAlloc = ContainPmrAlloc<T> &&
		(requires(const T & other, const typename T::allocator_type & alloc) { new T(other, alloc); }
	|| requires(const T & other, const typename T::allocator_type & alloc) { new T(std::allocator_arg_t{}, alloc, other); });

	template<typename T>
	concept MoveCtorWithAlloc = ContainPmrAlloc<T> &&
		(requires(T other, const typename T::allocator_type & alloc) { new T(std::move(other), alloc); }
	|| requires(T other, const typename T::allocator_type & alloc) { new T(std::allocator_arg_t{}, alloc, std::move(other)); });
}

namespace Ubpa::UECS {
	template<typename... Cmpts>
	void CmptTraits::Register() {
		(RegisterOne<Cmpts>(), ...);
	}
	template<typename... Cmpts>
	void CmptTraits::UnsafeRegister() {
		(UnsafeRegisterOne<Cmpts>(), ...);
	}

	template<typename Cmpt>
	void CmptTraits::UnsafeRegisterOne() {
		constexpr Type type = Type_of<Cmpt>;

		if constexpr (std::is_trivial_v<Cmpt>)
			trivials.insert(TypeID_of<Cmpt>);

		sizeofs.emplace(type.GetID(), sizeof(Cmpt));
		alignments.emplace(type.GetID(), alignof(Cmpt));
		names.emplace(type.GetID(), type.GetName());

		if constexpr (details::DefaultCtorWithAlloc<Cmpt>) {
			default_constructors.emplace(type.GetID(), [](void* cmpt, std::pmr::memory_resource* world_rsrc) {
				using Alloc = typename Cmpt::allocator_type;
				Alloc alloc(world_rsrc);
				std::allocator_traits<Alloc>::template construct(alloc, reinterpret_cast<Cmpt*>(cmpt));
			});
		}
		else if constexpr (std::is_default_constructible_v<Cmpt>) {
			default_constructors.emplace(type.GetID(), [](void* cmpt, std::pmr::memory_resource*) {
				new(cmpt)Cmpt();
			});
		}

		if constexpr (std::is_destructible_v<Cmpt> && !std::is_trivially_destructible_v<Cmpt>) {
			destructors.emplace(type.GetID(), [](void* cmpt) {
				static_cast<Cmpt*>(cmpt)->~Cmpt();
			});
		}

		if constexpr (details::MoveCtorWithAlloc<Cmpt>) {
			move_constructors.emplace(type.GetID(), [](void* dst, void* src, std::pmr::memory_resource* world_rsrc) {
				using Alloc = typename Cmpt::allocator_type;
				Alloc alloc(world_rsrc);
				std::allocator_traits<Alloc>::template construct(alloc, reinterpret_cast<Cmpt*>(dst), std::move(*static_cast<const Cmpt*>(src)));
			});
		}
		else if constexpr (std::is_move_constructible_v<Cmpt> && !std::is_trivially_move_constructible_v<Cmpt>) {
			move_constructors.emplace(type.GetID(), [](void* dst, void* src, std::pmr::memory_resource*) {
				new(dst)Cmpt(std::move(*static_cast<Cmpt*>(src)));
			});
		}

		if constexpr (std::is_move_assignable_v<Cmpt> && !std::is_trivially_move_assignable_v<Cmpt>) {
			move_assignments.emplace(type.GetID(), [](void* dst, void* src) {
				*static_cast<Cmpt*>(dst) = std::move(*static_cast<Cmpt*>(src));
			});
		}

		if constexpr (details::CopyCtorWithAlloc<Cmpt>) {
			copy_constructors.emplace(type.GetID(), [](void* dst, const void* src, std::pmr::memory_resource* world_rsrc) {
				using Alloc = typename Cmpt::allocator_type;
				Alloc alloc(world_rsrc);
				std::allocator_traits<Alloc>::template construct(alloc, reinterpret_cast<Cmpt*>(dst), *static_cast<const Cmpt*>(src));
			});
		}
		else if constexpr (std::is_copy_constructible_v<Cmpt> && !std::is_trivially_copy_constructible_v<Cmpt>) {
			copy_constructors.emplace(type.GetID(), [](void* dst, const void* src, std::pmr::memory_resource*) {
				new(dst)Cmpt(*static_cast<const Cmpt*>(src));
			});
		}
	}

	template<typename Cmpt>
	void CmptTraits::RegisterOne() {
		static_assert(!IsTaggedCmpt_v<Cmpt>, "<Cmpt> should not be tagged");
		static_assert(std::is_default_constructible_v<Cmpt> || details::DefaultCtorWithAlloc<Cmpt>, "<Cmpt> must be default-constructible or with alloc");
		static_assert(std::is_copy_constructible_v<Cmpt> || details::CopyCtorWithAlloc<Cmpt>, "<Cmpt> must be copy-constructible or with alloc");
		static_assert(std::is_move_constructible_v<Cmpt> || details::MoveCtorWithAlloc<Cmpt>, "<Cmpt> must be move-constructible or with alloc");
		static_assert(std::is_move_assignable_v<Cmpt>, "<Cmpt> must be move-assignable");
		static_assert(std::is_destructible_v<Cmpt>, "<Cmpt> must be destructible");

		UnsafeRegisterOne<Cmpt>();
	}

	template<typename Cmpt>
	void CmptTraits::Deregister() {
		constexpr TypeID type = TypeID_of<Cmpt>;

		sizeofs.erase(type);
		alignments.erase(type);
		names.erase(type);

		if constexpr (std::is_trivial_v<Cmpt>)
			trivials.erase(TypeID_of<Cmpt>);

		default_constructors.erase(type);
		if constexpr (!std::is_trivially_destructible_v<Cmpt>)
			destructors.erase(type);
		if constexpr (!std::is_trivially_move_constructible_v<Cmpt>)
			move_constructors.erase(type);
		if constexpr (!std::is_trivially_move_assignable_v<Cmpt>)
			move_assignments.erase(type);
		if constexpr (!std::is_trivially_copy_constructible_v<Cmpt>)
			copy_constructors.erase(type);
	}
}
