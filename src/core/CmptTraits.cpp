#include <UECS/CmptTraits.hpp>

using namespace Ubpa;
using namespace Ubpa::UECS;

struct CmptTraits::Impl {
	Impl(std::pmr::unsynchronized_pool_resource* rsrc) :
		rsrc{ rsrc },
		trivials{rsrc},
		names{rsrc},
		sizeofs{ rsrc },
		alignments{ rsrc },
		default_constructors{ rsrc },
		copy_constructors{rsrc},
		move_constructors{ rsrc },
		move_assignments{ rsrc },
		destructors{ rsrc } {}

	Impl(const Impl& other, std::pmr::unsynchronized_pool_resource* rsrc) :
		rsrc{ rsrc },
		trivials{ other.trivials, rsrc },
		names{ rsrc },
		sizeofs{ other.sizeofs, rsrc },
		alignments{ other.alignments, rsrc },
		default_constructors{ other.default_constructors, rsrc },
		copy_constructors{ other.copy_constructors, rsrc },
		move_constructors{ other.move_constructors, rsrc },
		move_assignments{ other.move_assignments, rsrc },
		destructors{ other.destructors, rsrc }
	{
		for (const auto& [id, name] : other.names) {
			char* buffer = (char*)rsrc->allocate((name.size() + 1) * sizeof(char), alignof(char));
			std::memcpy(buffer, name.data(), name.size() * sizeof(char));
			buffer[name.size()] = 0;
			names.emplace(id, std::string_view{ buffer, name.size() });
			buffer += name.size() + 1;
		}
	}

	std::pmr::unsynchronized_pool_resource* rsrc;

	std::pmr::unordered_set<TypeID> trivials;
	std::pmr::unordered_map<TypeID, std::string_view> names;
	std::pmr::unordered_map<TypeID, std::size_t> sizeofs;
	std::pmr::unordered_map<TypeID, std::size_t> alignments;
	std::pmr::unordered_map<TypeID, std::function<void(void*, std::pmr::memory_resource*)>> default_constructors; // dst <- src
	std::pmr::unordered_map<TypeID, std::function<void(void*, const void*, std::pmr::memory_resource*)>> copy_constructors; // dst <- src
	std::pmr::unordered_map<TypeID, std::function<void(void*, void*, std::pmr::memory_resource*)>> move_constructors; // dst <- src
	std::pmr::unordered_map<TypeID, std::function<void(void*, void*)>> move_assignments; // dst <- src
	std::pmr::unordered_map<TypeID, std::function<void(void*)>> destructors;
};

CmptTraits::CmptTraits(std::pmr::unsynchronized_pool_resource* r) :
	impl{ std::make_unique<CmptTraits::Impl>(r) }
{
	Register<Entity>();
}

CmptTraits::CmptTraits(const CmptTraits& other, std::pmr::unsynchronized_pool_resource* r) :
	impl{ std::make_unique<CmptTraits::Impl>(*other.impl, r) } {}

CmptTraits::CmptTraits(CmptTraits&&) noexcept = default;

CmptTraits::~CmptTraits() = default;

bool CmptTraits::IsTrivial(TypeID type) const {
	return impl->trivials.contains(type);
}

std::size_t CmptTraits::Sizeof(TypeID type) const {
	auto target = impl->sizeofs.find(type);
	assert(target != impl->sizeofs.end());
	return target->second;
}

std::size_t CmptTraits::Alignof(TypeID type) const {
	auto target = impl->alignments.find(type);

	return target != impl->alignments.end() ? target->second : default_alignment;
}

std::string_view CmptTraits::Nameof(TypeID type) const {
	auto target = impl->names.find(type);
	if (target != impl->names.end())
		return target->second;
	else
		return {};
}

const std::pmr::unordered_set<TypeID>& CmptTraits::GetTrivials() const noexcept {
	return impl->trivials;
}

const std::pmr::unordered_map<TypeID, std::string_view>& CmptTraits::GetNames() const noexcept {
	return impl->names;
}

const std::pmr::unordered_map<TypeID, std::size_t>& CmptTraits::GetSizeofs() const noexcept {
	return impl->sizeofs;
}

const std::pmr::unordered_map<TypeID, std::size_t>& CmptTraits::GetAlignments() const noexcept {
	return impl->alignments;
}

const std::pmr::unordered_map<TypeID, std::function<void(void*, std::pmr::memory_resource*)>>& CmptTraits::GetDefaultConstructors() const noexcept {
	return impl->default_constructors;
}

const std::pmr::unordered_map<TypeID, std::function<void(void*, const void*, std::pmr::memory_resource*)>>& CmptTraits::GetCopyConstructors() const noexcept {
	return impl->copy_constructors;
}

const std::pmr::unordered_map<TypeID, std::function<void(void*, void*, std::pmr::memory_resource*)>>& CmptTraits::GetMoveConstructors() const noexcept {
	return impl->move_constructors;
}

const std::pmr::unordered_map<TypeID, std::function<void(void*, void*)>>& CmptTraits::GetMoveAssignments() const noexcept {
	return impl->move_assignments;
}

const std::pmr::unordered_map<TypeID, std::function<void(void*)>>& CmptTraits::GetDestructors() const noexcept {
	return impl->destructors;
}

CmptTraits& CmptTraits::RegisterSize(TypeID type, std::size_t size) {
	impl->sizeofs.emplace(type, size);
	return *this;
}

CmptTraits& CmptTraits::RegisterAlignment(TypeID type, std::size_t alignment) {
	impl->alignments.emplace(type, alignment);
	return *this;
}

CmptTraits& CmptTraits::RegisterDefaultConstructor(TypeID type, std::function<void(void*, std::pmr::memory_resource*)> f) {
	impl->default_constructors.emplace(type, std::move(f));
	return *this;
}

CmptTraits& CmptTraits::RegisterCopyConstructor(TypeID type, std::function<void(void*, const void*, std::pmr::memory_resource*)> f) {
	impl->copy_constructors.emplace(type, std::move(f));
	return *this;
}

CmptTraits& CmptTraits::RegisterMoveConstructor(TypeID type, std::function<void(void*, void*, std::pmr::memory_resource*)> f) {
	impl->move_constructors.emplace(type, std::move(f));
	return *this;
}

CmptTraits& CmptTraits::RegisterMoveAssignment(TypeID type, std::function<void(void*, void*)> f) {
	impl->move_assignments.emplace(type, std::move(f));
	return *this;
}

CmptTraits& CmptTraits::RegisterDestructor(TypeID type, std::function<void(void*)> f) {
	impl->destructors.emplace(type, std::move(f));
	return *this;
}

CmptTraits& CmptTraits::RegisterName(Type type) {
	auto target = impl->names.find(type);
	if (target != impl->names.end()) {
		assert(type.Is(target->second));
		return *this;
	}

	auto* buffer = (char*)impl->rsrc->allocate((type.GetName().size() + 1) * sizeof(char), alignof(char));
	std::memcpy(buffer, type.GetName().data(), type.GetName().size() * sizeof(char));
	buffer[type.GetName().size()] = 0;
	impl->names.emplace(type.GetID(), std::string_view{ buffer, type.GetName().size() });

	return *this;
}

CmptTraits& CmptTraits::RegisterTrivial(TypeID type) {
	impl->trivials.insert(type);
	return *this;
}

CmptTraits& CmptTraits::Deregister(TypeID type) noexcept {
	impl->names.erase(type);
	impl->trivials.erase(type);
	impl->sizeofs.erase(type);
	impl->alignments.erase(type);
	impl->default_constructors.erase(type);
	impl->copy_constructors.erase(type);
	impl->move_constructors.erase(type);
	impl->move_assignments.erase(type);
	impl->destructors.erase(type);
	return *this;
}

CmptTraits& CmptTraits::Clear() {
	impl->names.clear();
	impl->trivials.clear();
	impl->sizeofs.clear();
	impl->alignments.clear();
	impl->default_constructors.clear();
	impl->copy_constructors.clear();
	impl->move_constructors.clear();
	impl->move_assignments.clear();
	impl->destructors.clear();
	return *this;
}
