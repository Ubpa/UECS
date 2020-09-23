#pragma once

#include "../CmptType.h"
#include "../RTDCmptTraits.h"

#include <unordered_map>
#include <functional>

namespace Ubpa::UECS {
	// run-time static component traits
	class ArchetypeCmptTraits {
	public:
		size_t Sizeof(CmptType) const;
		size_t Alignof(CmptType) const;
		void CopyConstruct(CmptType, void* dst, void* src) const;
		void MoveConstruct(CmptType, void* dst, void* src) const;
		void MoveAssign(CmptType, void* dst, void* src) const;
		void Destruct(CmptType, void* cmpt) const;

		template<typename Cmpt>
		void Register();

		void Register(const RTDCmptTraits&, CmptType);

		template<typename Cmpt>
		void Deregister();
		void Deregister(CmptType) noexcept;

	private:
		friend class Archetype;

		std::unordered_map<CmptType, size_t> sizeofs;
		std::unordered_map<CmptType, size_t> alignments;
		std::unordered_map<CmptType, std::function<void(void*,void*)>> copy_constructors; // dst <- src
		std::unordered_map<CmptType, std::function<void(void*,void*)>> move_constructors; // dst <- src
		std::unordered_map<CmptType, std::function<void(void*,void*)>> move_assignments; // dst <- src
		std::unordered_map<CmptType, std::function<void(void*)>> destructors;
	};
}

#include "ArchetypeCmptTraits.inl"
