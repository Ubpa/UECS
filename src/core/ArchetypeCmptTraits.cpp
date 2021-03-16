#pragma once

#include "ArchetypeCmptTraits.h"

#include <UECS/RTDCmptTraits.h>

#include <stdexcept>

using namespace Ubpa::UECS;

ArchetypeCmptTraits::CmptTrait* ArchetypeCmptTraits::GetTrait(TypeID ID) noexcept {
	for (auto& trait : cmpt_traits) {
		if (trait.ID == ID)
			return &trait;
	}
	assert(false);
	return nullptr;
}

void ArchetypeCmptTraits::CmptTrait::CopyConstruct(void* dst, void* src) const {
	if (copy_ctor)
		copy_ctor(dst, src);
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

void ArchetypeCmptTraits::Register(const RTDCmptTraits& rtdct, TypeID type) {
	auto size_target = rtdct.GetSizeofs().find(type);
	if (size_target == rtdct.GetSizeofs().end())
		throw std::logic_error("ArchetypeCmptTraits::Register: RTDCmptTrait hasn't registered <TypeID>");

	auto copy_constructor_target = rtdct.GetCopyConstructors().find(type);
	auto move_constructor_target = rtdct.GetMoveConstructors().find(type);
	auto move_assignments_target = rtdct.GetMoveAssignments().find(type);
	auto destructor_target = rtdct.GetDestructors().find(type);

	CmptTrait trait{
		.ID = type,
		.trivial = rtdct.IsTrivial(type),
		.size = size_target->second,
		.alignment = rtdct.Alignof(type)
	};
	if (copy_constructor_target != rtdct.GetCopyConstructors().end())
		trait.copy_ctor = copy_constructor_target->second;
	if (move_constructor_target != rtdct.GetMoveConstructors().end())
		trait.move_ctor = move_constructor_target->second;
	if (move_assignments_target != rtdct.GetMoveAssignments().end())
		trait.move_assign = move_assignments_target->second;
	if (destructor_target != rtdct.GetDestructors().end())
		trait.dtor = destructor_target->second;

	cmpt_traits.push_back(std::move(trait));
}

void ArchetypeCmptTraits::Deregister(TypeID type) noexcept {
	for (auto cursor = cmpt_traits.begin(); cursor != cmpt_traits.end(); ++cursor) {
		if (cursor->ID == type) {
			cmpt_traits.erase(cursor);
			return;
		}
	}
	assert(false);
}
