#pragma once

#include "../AccessTypeID.h"
#include "../RTDCmptTraits.h"

#include <small_vector.h>

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

		const std::function<void(void*, void*)>* GetCopyConstruct(TypeID) const;
		const std::function<void(void*, void*)>* GetMoveConstruct(TypeID) const;
		const std::function<void(void*, void*)>* GetMoveAssign(TypeID) const;
		const std::function<void(void*)>* GetDestruct(TypeID) const;

		template<typename Cmpt>
		void Register();

		void Register(const RTDCmptTraits&, TypeID);

		template<typename Cmpt>
		void Deregister() noexcept;
		void Deregister(TypeID) noexcept;

	private:
		small_vector<std::tuple<TypeID, size_t>, 16> sizeofs;
		small_vector<std::tuple<TypeID, size_t>, 16> alignments;
		small_vector<std::tuple<TypeID, std::function<void(void*, void*)>>, 16> copy_constructors; // dst <- src
		small_vector<std::tuple<TypeID, std::function<void(void*, void*)>>, 16> move_constructors; // dst <- src
		small_vector<std::tuple<TypeID, std::function<void(void*, void*)>>, 16> move_assignments; // dst <- src
		small_vector<std::tuple<TypeID, std::function<void(void*)>>, 16> destructors;
	};
}

#include "ArchetypeCmptTraits.inl"
