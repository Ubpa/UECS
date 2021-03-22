#include <UECS/UECS.hpp>

using namespace Ubpa;
using namespace Ubpa::UECS;

struct Buffer {
	using allocator_type = std::pmr::vector<int>::allocator_type;
	Buffer(const Buffer& other, const allocator_type& alloc)
		: value(alloc) {}
	Buffer(const allocator_type& alloc)
		: value(alloc) {}
	allocator_type get_allocator() const noexcept { return value.get_allocator(); }
	std::pmr::vector<int> value;
};

struct PrintSystem {
	static void OnUpdate(Schedule& schedule) {
		schedule.RegisterEntityJob(
			[](World* w, const Buffer* buffer) {
				std::cout << w->GetSyncResource() << std::endl;
				std::cout << buffer->get_allocator().resource() << std::endl;
			},
			"Print"
		);
	}
};

int main() {
	World w;
	w.entityMngr.cmptTraits.Register<Buffer>();
	w.systemMngr.RegisterAndCreate<PrintSystem>();
	w.systemMngr.Activate<PrintSystem>();

	w.entityMngr.Create(TypeIDs_of<Buffer>); // use w's resource
	w.Update();

	World w2(w); // buffer use w2's resource
	w.Update();
	w2.Update();

	return 0;
}
