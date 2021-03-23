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
			RegisterTrivial(type.GetID());

		RegisterSize(type.GetID(), sizeof(Cmpt));
		RegisterAlignment(type.GetID(), alignof(Cmpt));
		RegisterName(type);

		if constexpr (details::DefaultCtorWithAlloc<Cmpt>) {
			RegisterDefaultConstructor(type.GetID(), [](void* cmpt, std::pmr::memory_resource* world_rsrc) {
				using Alloc = typename Cmpt::allocator_type;
				Alloc alloc(world_rsrc);
				std::allocator_traits<Alloc>::template construct(alloc, reinterpret_cast<Cmpt*>(cmpt));
			});
		}
		else if constexpr (std::is_default_constructible_v<Cmpt>) {
			RegisterDefaultConstructor(type.GetID(), [](void* cmpt, std::pmr::memory_resource*) {
				new(cmpt)Cmpt();
			});
		}

		if constexpr (std::is_destructible_v<Cmpt> && !std::is_trivially_destructible_v<Cmpt>) {
			RegisterDestructor(type.GetID(), [](void* cmpt) {
				static_cast<Cmpt*>(cmpt)->~Cmpt();
			});
		}

		if constexpr (details::MoveCtorWithAlloc<Cmpt>) {
			RegisterMoveConstructor(type.GetID(), [](void* dst, void* src, std::pmr::memory_resource* world_rsrc) {
				using Alloc = typename Cmpt::allocator_type;
				Alloc alloc(world_rsrc);
				std::allocator_traits<Alloc>::template construct(alloc, reinterpret_cast<Cmpt*>(dst), std::move(*static_cast<const Cmpt*>(src)));
			});
		}
		else if constexpr (std::is_move_constructible_v<Cmpt> && !std::is_trivially_move_constructible_v<Cmpt>) {
			RegisterMoveConstructor(type.GetID(), [](void* dst, void* src, std::pmr::memory_resource*) {
				new(dst)Cmpt(std::move(*static_cast<Cmpt*>(src)));
			});
		}

		if constexpr (std::is_move_assignable_v<Cmpt> && !std::is_trivially_move_assignable_v<Cmpt>) {
			RegisterMoveAssignment(type.GetID(), [](void* dst, void* src) {
				*static_cast<Cmpt*>(dst) = std::move(*static_cast<Cmpt*>(src));
			});
		}

		if constexpr (details::CopyCtorWithAlloc<Cmpt>) {
			RegisterCopyConstructor(type.GetID(), [](void* dst, const void* src, std::pmr::memory_resource* world_rsrc) {
				using Alloc = typename Cmpt::allocator_type;
				Alloc alloc(world_rsrc);
				std::allocator_traits<Alloc>::template construct(alloc, reinterpret_cast<Cmpt*>(dst), *static_cast<const Cmpt*>(src));
			});
		}
		else if constexpr (std::is_copy_constructible_v<Cmpt> && !std::is_trivially_copy_constructible_v<Cmpt>) {
			RegisterCopyConstructor(type.GetID(), [](void* dst, const void* src, std::pmr::memory_resource*) {
				new(dst)Cmpt(*static_cast<const Cmpt*>(src));
			});
		}
	}

	template<typename Cmpt>
	void CmptTraits::RegisterOne() {
		static_assert(!IsTaggedCmpt_v<Cmpt>, "<Cmpt> should not be tagged");
		if constexpr (details::ContainPmrAlloc<Cmpt>) {
			static_assert(details::DefaultCtorWithAlloc<Cmpt>, "alloc-awared <Cmpt> must be default-constructible with alloc");
			static_assert(details::CopyCtorWithAlloc<Cmpt>, "alloc-awared <Cmpt> must be copy-constructible with alloc");
			static_assert(details::MoveCtorWithAlloc<Cmpt>, "alloc-awared <Cmpt> must be move-constructible with alloc");
		}
		else {
			static_assert(std::is_default_constructible_v<Cmpt>, "<Cmpt> must be default-constructible");
			static_assert(std::is_copy_constructible_v<Cmpt>, "<Cmpt> must be copy-constructible");
			static_assert(std::is_move_constructible_v<Cmpt>, "<Cmpt> must be move-constructible");
		}
		static_assert(std::is_move_assignable_v<Cmpt>, "<Cmpt> must be move-assignable");
		static_assert(std::is_destructible_v<Cmpt>, "<Cmpt> must be destructible");

		UnsafeRegisterOne<Cmpt>();
	}

	template<typename Cmpt>
	void CmptTraits::Deregister() {
		constexpr TypeID type = TypeID_of<Cmpt>;
		Deregister(type);
	}
}
