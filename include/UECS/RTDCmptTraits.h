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

		static RTDCmptTraits& Instance() noexcept {
			static RTDCmptTraits instance;
			return instance;
		}

		// neccessary
		RTDCmptTraits& RegisterSize(CmptType type, size_t size) {
			sizeofs[type] = size;
			return *this;
		}

		// optional
		RTDCmptTraits& RegisterAlignment(CmptType type, size_t alignment) {
			alignments[type] = alignment;
			return *this;
		}

		// optional
		RTDCmptTraits& RegisterDefaultConstructor(CmptType type, std::function<void(void*)> f) {
			default_constructors[type] = std::move(f);
			return *this;
		}

		// optional
		RTDCmptTraits& RegisterCopyConstructor(CmptType type, std::function<void(void*, void*)> f) {
			copy_constructors[type] = std::move(f);
			return *this;
		}

		// optional
		RTDCmptTraits& RegisterMoveConstructor(CmptType type, std::function<void(void*, void*)> f) {
			move_constructors[type] = std::move(f);
			return *this;
		}

		// optional
		RTDCmptTraits& RegisterDestructor(CmptType type, std::function<void(void*)> f) {
			destructors[type] = std::move(f);
			return *this;
		}

		RTDCmptTraits& Deregister(CmptType type) {
			sizeofs.erase(type);
			alignments.erase(type);
			default_constructors.erase(type);
			copy_constructors.erase(type);
			move_constructors.erase(type);
			destructors.erase(type);
			return *this;
		}

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
