#pragma once

#include "CmptType.h"

#include <unordered_map>
#include <functional>

namespace Ubpa::UECS {
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
	class RTDCmptTraits {
	public:
		static constexpr size_t DefaultAlignment() noexcept { return alignof(std::max_align_t); }

		RTDCmptTraits& Clear();

		RTDCmptTraits& RegisterSize(CmptType, size_t size);
		RTDCmptTraits& RegisterAlignment(CmptType, size_t alignment);
		RTDCmptTraits& RegisterDefaultConstructor(CmptType, std::function<void(void*)>);
		RTDCmptTraits& RegisterCopyConstructor(CmptType, std::function<void(void*,void*)>);
		RTDCmptTraits& RegisterMoveConstructor(CmptType, std::function<void(void*,void*)>);
		RTDCmptTraits& RegisterMoveAssignment(CmptType, std::function<void(void*,void*)>);
		RTDCmptTraits& RegisterDestructor(CmptType, std::function<void(void*)>);
		RTDCmptTraits& RegisterName(CmptType, std::string name);

		const auto& GetSizeofs() const noexcept { return sizeofs; }
		const auto& GetAlignments() const noexcept { return alignments; };
		const auto& GetDefaultConstructors() const noexcept { return default_constructors; }
		const auto& GetCopyConstructors() const noexcept { return copy_constructors; }
		const auto& GetMoveConstructors() const noexcept { return move_constructors; }
		const auto& GetMoveAssignments() const noexcept { return move_assignments; }
		const auto& GetDestructors() const noexcept { return destructors; }
		const auto& GetNames() const noexcept { return names; }

		size_t Sizeof(CmptType) const;
		size_t Alignof(CmptType) const;
		void DefaultConstruct(CmptType, void* cmpt) const;
		void CopyConstruct(CmptType, void* dst, void* src) const;
		void MoveConstruct(CmptType, void* dst, void* src) const;
		void MoveAssign(CmptType, void* dst, void* src) const;
		void Destruct(CmptType, void* cmpt) const;
		std::string_view Nameof(CmptType) const;

		RTDCmptTraits& Deregister(CmptType) noexcept;

		template<typename... Cmpts>
		void Register();

		template<typename Cmpt>
		void Deregister();

	private:
		// register all for Cmpt
		// static_assert
		// - is_default_constructible_v
		// - is_copy_constructible_v
		// - is_move_constructible_v
		// - is_move_assignable_v
		// - is_destructible_v
		template<typename Cmpt>
		void RegisterOne();

		std::unordered_map<CmptType, size_t> sizeofs;
		std::unordered_map<CmptType, size_t> alignments;
		std::unordered_map<CmptType, std::function<void(void*)>> default_constructors; // dst <- src
		std::unordered_map<CmptType, std::function<void(void*,void*)>> copy_constructors; // dst <- src
		std::unordered_map<CmptType, std::function<void(void*,void*)>> move_constructors; // dst <- src
		std::unordered_map<CmptType, std::function<void(void*,void*)>> move_assignments; // dst <- src
		std::unordered_map<CmptType, std::function<void(void*)>> destructors;
		std::unordered_map<CmptType, std::string> names;
	};
}

#include "detail/RTDCmptTraits.inl"
