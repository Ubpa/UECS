#pragma once

#include "CmptType.h"

#include <unordered_map>
#include <functional>

namespace Ubpa::UECS {
	// run-time dynamic component traits, singleton
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
		static constexpr size_t default_alignment = alignof(std::max_align_t);

		static RTDCmptTraits& Instance() noexcept;

		// neccessary
		RTDCmptTraits& RegisterSize(CmptType type, size_t size);

		// optional
		RTDCmptTraits& RegisterAlignment(CmptType type, size_t alignment);

		// optional
		RTDCmptTraits& RegisterDefaultConstructor(CmptType type, std::function<void(void*)> f);

		// optional
		RTDCmptTraits& RegisterCopyConstructor(CmptType type, std::function<void(void*, void*)> f);

		// optional
		RTDCmptTraits& RegisterMoveConstructor(CmptType type, std::function<void(void*, void*)> f);

		// optional
		RTDCmptTraits& RegisterMoveAssignment(CmptType type, std::function<void(void*, void*)> f);

		// optional
		RTDCmptTraits& RegisterDestructor(CmptType type, std::function<void(void*)> f);

		// optional
		RTDCmptTraits& RegisterName(CmptType type, std::string name);

		size_t Sizeof(CmptType type) const;
		size_t Alignof(CmptType type) const;
		void CopyConstruct(CmptType type, void* dst, void* src) const;
		void MoveConstruct(CmptType type, void* dst, void* src) const;
		void MoveAssign(CmptType type, void* dst, void* src) const;
		void Destruct(CmptType type, void* cmpt) const;
		std::string_view Nameof(CmptType type) const;

		RTDCmptTraits& Deregister(CmptType type) noexcept;

		template<typename... Cmpts>
		void Register();

		template<typename Cmpt>
		void Deregister();

	private:
		friend class RTSCmptTraits;
		friend class Archetype;
		friend class EntityMngr;

		RTDCmptTraits() = default;

		// register all for Cmpt
		// static_assert
		// - is_default_constructible_v
		// - is_copy_constructible_v
		// - is_move_constructible_v
		// - is_destructible_v
		template<typename Cmpt>
		void RegisterOne();

		std::unordered_map<CmptType, size_t> sizeofs;
		std::unordered_map<CmptType, size_t> alignments;
		std::unordered_map<CmptType, std::function<void(void*)>> default_constructors; // dst <- src
		std::unordered_map<CmptType, std::function<void(void*, void*)>> copy_constructors; // dst <- src
		std::unordered_map<CmptType, std::function<void(void*, void*)>> move_constructors; // dst <- src
		std::unordered_map<CmptType, std::function<void(void*, void*)>> move_assignments; // dst <- src
		std::unordered_map<CmptType, std::function<void(void*)>> destructors;
		std::unordered_map<CmptType, std::string> names;
	};
}

#include "detail/RTDCmptTraits.inl"
