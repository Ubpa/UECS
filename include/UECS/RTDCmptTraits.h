#pragma once

#include "CmptType.h"

#include <unordered_map>
#include <functional>

namespace Ubpa {
	// run-time dynamic component traits, singleton
	// size (> 0) is neccessary
	// optional
	// - alignment: alignof(std::max_align_t) as default, 8 / 16 in most cases
	// - default constructor: do nothing as default
	// - copy constructor: memcpy as default
	// - move constructor: memcpy as default
	// - destructor: do nothing as default
	class RTDCmptTraits {
	public:
		static constexpr size_t default_alignment = alignof(std::max_align_t);

		inline static RTDCmptTraits& Instance() noexcept;

		// neccessary
		inline RTDCmptTraits& RegisterSize(CmptType type, size_t size);

		// optional
		inline RTDCmptTraits& RegisterAlignment(CmptType type, size_t alignment);

		// optional
		inline RTDCmptTraits& RegisterDefaultConstructor(CmptType type, std::function<void(void*)> f);

		// optional
		inline RTDCmptTraits& RegisterCopyConstructor(CmptType type, std::function<void(void*, void*)> f);

		// optional
		inline RTDCmptTraits& RegisterMoveConstructor(CmptType type, std::function<void(void*, void*)> f);

		// optional
		inline RTDCmptTraits& RegisterDestructor(CmptType type, std::function<void(void*)> f);

		inline RTDCmptTraits& Deregister(CmptType type) noexcept;

		// register all for Cmpt
		// static_assert
		// - is_default_constructible_v
		// - is_copy_constructible_v
		// - is_move_constructible_v
		// - is_destructible_v
		template<typename Cmpt>
		void Register();

		template<typename Cmpt>
		void Deregister();

	private:
		friend class RTSCmptTraits;
		friend class Archetype;
		friend class EntityMngr;

		RTDCmptTraits() = default;

		std::unordered_map<CmptType, size_t> sizeofs;
		std::unordered_map<CmptType, size_t> alignments;
		std::unordered_map<CmptType, std::function<void(void*)>> default_constructors; // dst <- src
		std::unordered_map<CmptType, std::function<void(void*, void*)>> copy_constructors; // dst <- src
		std::unordered_map<CmptType, std::function<void(void*, void*)>> move_constructors; // dst <- src
		std::unordered_map<CmptType, std::function<void(void*)>> destructors;
	};
}

#include "detail/RTDCmptTraits.inl"
