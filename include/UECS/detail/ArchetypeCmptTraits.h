#pragma once

#include "../AccessTypeID.h"
#include "../RTDCmptTraits.h"

#include <unordered_map>
#include <functional>

namespace Ubpa::UECS {
	// run-time static component traits
	class ArchetypeCmptTraits {
	public:
		std::size_t Sizeof(TypeID) const;
		std::size_t Alignof(TypeID) const;
		void CopyConstruct(TypeID, void* dst, void* src) const;
		void MoveConstruct(TypeID, void* dst, void* src) const;
		void MoveAssign(TypeID, void* dst, void* src) const;
		void Destruct(TypeID, void* cmpt) const;

		template<typename Cmpt>
		void Register();

		void Register(const RTDCmptTraits&, TypeID);

		template<typename Cmpt>
		void Deregister() noexcept;
		void Deregister(TypeID) noexcept;

	private:
		friend class Archetype;

		std::unordered_map<TypeID, std::size_t> sizeofs;
		std::unordered_map<TypeID, std::size_t> alignments;
		std::unordered_map<TypeID, std::function<void(void*,void*)>> copy_constructors; // dst <- src
		std::unordered_map<TypeID, std::function<void(void*,void*)>> move_constructors; // dst <- src
		std::unordered_map<TypeID, std::function<void(void*,void*)>> move_assignments; // dst <- src
		std::unordered_map<TypeID, std::function<void(void*)>> destructors;
	};
}

#include "ArchetypeCmptTraits.inl"
