#pragma once

#include "ArchetypeCmptTraits.hpp"

#include <UECS/CmptTraits.hpp>

#include <UECS/EntityQuery.hpp>

#include <stdexcept>

using namespace Ubpa::UECS;

void ArchetypeCmptTraits::CmptTrait::DefaultConstruct(void* cmpt, std::pmr::memory_resource* world_rsrc) const {
	if(default_ctor)
		default_ctor(cmpt, world_rsrc);
}

void ArchetypeCmptTraits::CmptTrait::CopyConstruct(void* dst, const void* src, std::pmr::memory_resource* world_rsrc) const {
	if (copy_ctor)
		copy_ctor(dst, src, world_rsrc);
	else
		std::memcpy(dst, src, size);
}

void ArchetypeCmptTraits::CmptTrait::MoveConstruct(void* dst, void* src) const {
	if (move_ctor)
		move_ctor(dst, src);
	else
		std::memcpy(dst, src, size);
}

void ArchetypeCmptTraits::CmptTrait::MoveAssign(void* dst, void* src) const {
	if (move_assign)
		move_assign(dst, src);
	else
		std::memcpy(dst, src, size);
}

void ArchetypeCmptTraits::CmptTrait::Destruct(void* cmpt) const {
	if (dtor)
		dtor(cmpt);
}

std::size_t ArchetypeCmptTraits::GetTypeIndex(TypeID ID) const noexcept {
	return static_cast<std::size_t>(std::distance(types.begin(), types.find(ID)));
}

ArchetypeCmptTraits::CmptTrait& ArchetypeCmptTraits::GetTrait(TypeID ID) noexcept {
	return cmpt_traits[GetTypeIndex(ID)];
}

void ArchetypeCmptTraits::Register(const CmptTraits& rtdct, TypeID type) {
	auto size_target = rtdct.GetSizeofs().find(type);
	if (size_target == rtdct.GetSizeofs().end())
		throw std::logic_error("ArchetypeCmptTraits::Register: RTDCmptTrait hasn't registered <TypeID>");

	auto default_constructor_target = rtdct.GetDefaultConstructors().find(type);
	auto copy_constructor_target = rtdct.GetCopyConstructors().find(type);
	auto move_constructor_target = rtdct.GetMoveConstructors().find(type);
	auto move_assignments_target = rtdct.GetMoveAssignments().find(type);
	auto destructor_target = rtdct.GetDestructors().find(type);

	CmptTrait trait{
		.trivial = rtdct.IsTrivial(type),
		.size = size_target->second,
		.alignment = rtdct.Alignof(type)
	};
	if (default_constructor_target != rtdct.GetDefaultConstructors().end())
		trait.default_ctor = default_constructor_target->second;
	if (copy_constructor_target != rtdct.GetCopyConstructors().end())
		trait.copy_ctor = copy_constructor_target->second;
	if (move_constructor_target != rtdct.GetMoveConstructors().end())
		trait.move_ctor = move_constructor_target->second;
	if (move_assignments_target != rtdct.GetMoveAssignments().end())
		trait.move_assign = move_assignments_target->second;
	if (destructor_target != rtdct.GetDestructors().end())
		trait.dtor = destructor_target->second;

	if (!trait.trivial)
		trivial = false;

	auto [iter, success] = types.insert(type);
	std::size_t idx = static_cast<std::size_t>(std::distance(types.begin(), iter));
	if (success)
		cmpt_traits.insert(cmpt_traits.begin() + idx, std::move(trait));
	else
		cmpt_traits[idx] = std::move(trait);
}

void ArchetypeCmptTraits::Deregister(TypeID type) noexcept {
	auto ttarget = types.find(type);
	if (ttarget == types.end())
		return;

	std::size_t idx = static_cast<std::size_t>(std::distance(types.begin(), ttarget));
	auto target = cmpt_traits.begin() + idx;
	if (!target->trivial) {
		cmpt_traits.erase(target);
		for (const auto& trait : cmpt_traits) {
			if (!trait.trivial)
				return;
		}
		// all cmpt is trivial
		trivial = true;
	}
	else
		cmpt_traits.erase(target);
}
