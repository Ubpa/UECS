#pragma once

#include "../CmptType.h"

#include <unordered_map>
#include <functional>

namespace Ubpa {
	// run-time static component traits
	class RTSCmptTraits {
	public:
		size_t Sizeof(CmptType type) const {
			assert(sizeofs.find(type) != sizeofs.end());
			return sizeofs.find(type)->second;
		}

		size_t Alignof(CmptType type) const {
			assert(alignments.find(type) != alignments.end());
			return alignments.find(type)->second;
		}

		void CopyConstruct(CmptType type, void* dst, void* src) const {
			auto target = copy_constructors.find(type);

			if (target != copy_constructors.end())
				target->second(dst, src);
			else
				memcpy(dst, src, Sizeof(type));
		}

		void MoveConstruct(CmptType type, void* dst, void* src) const {
			auto target = move_constructors.find(type);

			if (target != move_constructors.end())
				target->second(dst, src);
			else
				memcpy(dst, src, Sizeof(type));
		}

		void Destruct(CmptType type, void* cmpt) const {
			auto target = destructors.find(type);
			if (target != destructors.end())
				target->second(cmpt);
		}

		template<typename Cmpt>
		void Register();
		inline void Register(CmptType type);

		template<typename Cmpt>
		void Deregister();
		inline void Deregister(CmptType type);

	private:
		std::unordered_map<CmptType, size_t> sizeofs;
		std::unordered_map<CmptType, size_t> alignments;
		std::unordered_map<CmptType, std::function<void(void*, void*)>> copy_constructors; // dst <- src
		std::unordered_map<CmptType, std::function<void(void*, void*)>> move_constructors; // dst <- src
		std::unordered_map<CmptType, std::function<void(void*)>> destructors;
	};
}

#include "RTSCmptTraits.inl"
