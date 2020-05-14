#pragma once

#include "CmptType.h"

#include <unordered_map>
#include <functional>

namespace Ubpa {
	// run-time dynamic component traits
	class RTDCmptTraits {
	public:
		RTDCmptTraits& Instance() noexcept {
			static RTDCmptTraits instance;
			return instance;
		}

		RTDCmptTraits& Register(CmptType type, size_t size, size_t alignment) {
			sizeofs[type] = size;
			alignments[type] = alignment;
			return *this;
		}

		RTDCmptTraits& RegisterCopyConstructor(CmptType type, std::function<void(void*, void*)> f) {
			copy_constructors[type] = std::move(f);
			return *this;
		}

		RTDCmptTraits& RegisterMoveConstructor(CmptType type, std::function<void(void*, void*)> f) {
			move_constructors[type] = std::move(f);
			return *this;
		}

		RTDCmptTraits& RegisterDestructor(CmptType type, std::function<void(void*)> f) {
			destructors[type] = std::move(f);
			return *this;
		}

		RTDCmptTraits& Deregister(CmptType type) {
			sizeofs.erase(type);
			alignments.erase(type);
			copy_constructors.erase(type);
			move_constructors.erase(type);
			destructors.erase(type);
			return *this;
		}

	private:
		friend class RTSCmptTraits;

		RTDCmptTraits() = default;

		std::unordered_map<CmptType, size_t> sizeofs;
		std::unordered_map<CmptType, size_t> alignments;
		std::unordered_map<CmptType, std::function<void(void*, void*)>> copy_constructors; // dst <- src
		std::unordered_map<CmptType, std::function<void(void*, void*)>> move_constructors; // dst <- src
		std::unordered_map<CmptType, std::function<void(void*)>> destructors;
	};
}
