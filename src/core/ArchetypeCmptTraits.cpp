#pragma once

#include "ArchetypeCmptTraits.h"

#include <UECS/RTDCmptTraits.h>

#include <stdexcept>

using namespace Ubpa::UECS;

const ArchetypeCmptTraits::CmptTraits* ArchetypeCmptTraits::GetTraits(TypeID ID) const noexcept {
	for (auto& trait : cmpt_traits) {
		if (trait.ID == ID)
			return &trait;
	}
	assert(false);
	return nullptr;
}

bool ArchetypeCmptTraits::IsTrivial(TypeID type) const {
	return GetTraits(type)->trivial;
}

std::size_t ArchetypeCmptTraits::Sizeof(TypeID type) const {
	return GetTraits(type)->size;
}

std::size_t ArchetypeCmptTraits::Alignof(TypeID type) const {
	return GetTraits(type)->alignment;
}

void ArchetypeCmptTraits::CopyConstruct(TypeID type, void* dst, void* src) const {
	auto* trait = GetTraits(type);
	if (trait->copy_ctor)
		trait->copy_ctor(dst, src);
	else {
		assert(trait->trivial);
		std::memcpy(dst, src, trait->size);
	}
}

void ArchetypeCmptTraits::MoveConstruct(TypeID type, void* dst, void* src) const {
	auto* trait = GetTraits(type);
	if (trait->move_ctor)
		trait->move_ctor(dst, src);
	else {
		assert(trait->trivial);
		std::memcpy(dst, src, trait->size);
	}
}

void ArchetypeCmptTraits::MoveAssign(TypeID type, void* dst, void* src) const {
	auto* trait = GetTraits(type);
	if (trait->move_assign)
		trait->move_assign(dst, src);
	else {
		assert(trait->trivial);
		std::memcpy(dst, src, trait->size);
	}
}

void ArchetypeCmptTraits::Destruct(TypeID type, void* cmpt) const {
	auto* trait = GetTraits(type);
	if (trait->dtor)
		trait->dtor(cmpt);
	else
		assert(trait->trivial);
}

void ArchetypeCmptTraits::Register(const RTDCmptTraits& rtdct, TypeID type) {
	auto size_target = rtdct.GetSizeofs().find(type);
	if (size_target == rtdct.GetSizeofs().end())
		throw std::logic_error("ArchetypeCmptTraits::Register: RTDCmptTraits hasn't registered <TypeID>");

	auto copy_constructor_target = rtdct.GetCopyConstructors().find(type);
	auto move_constructor_target = rtdct.GetMoveConstructors().find(type);
	auto move_assignments_target = rtdct.GetMoveAssignments().find(type);
	auto destructor_target = rtdct.GetDestructors().find(type);

	CmptTraits trait{
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
