#include <UECS/UECS.hpp>

#include "MyResource.hpp"

#include <utility>
#include <algorithm>

using namespace Ubpa;
using namespace Ubpa::UECS;

struct Buffer {
	using allocator_type = std::pmr::vector<std::size_t>::allocator_type;
	allocator_type get_allocator() const noexcept { return value.get_allocator(); }

	Buffer(const allocator_type& alloc) : value(alloc) {}
	Buffer(const Buffer& other, const allocator_type& alloc) : value(other.value, alloc) {}
	Buffer(Buffer&& other, const allocator_type& alloc) : value(std::move(other.value), alloc) {}

	std::pmr::vector<std::size_t> value;
};

struct C {};

struct PrintSystem {
	static void OnUpdate(Schedule& schedule) {
		schedule.RegisterEntityJob(
			[](const Buffer* buffer) {
				std::cout << buffer->get_allocator().resource() << std::endl;
			},
			"Print"
		);
	}
};

struct FillSystem {
	static void OnUpdate(Schedule& schedule) {
		schedule.RegisterEntityJob(
			[](Buffer* buffer) {
				for (std::size_t j = 0; j < 100; j++)
					buffer->value.push_back(j);
			},
			"Fill"
		);
	}
};

int main() {
	MyResource rsrc;
	{
		World w(&rsrc);
		w.entityMngr.cmptTraits.Register<Buffer, C>();
		w.systemMngr.RegisterAndCreate<PrintSystem>();
		w.systemMngr.Activate<PrintSystem>();

		auto e0 = w.entityMngr.Create(TypeIDs_of<Buffer>);
		auto e1 = w.entityMngr.Create(TypeIDs_of<Buffer>);

		w.Update();

		w.entityMngr.Destroy(e0); // move in chunk

		w.Update();

		w.entityMngr.Attach(e1, TypeIDs_of<C>); // move between chunk

		w.Update();

		{
			World w2(w, &rsrc);
			w.Update();
			w2.Update();
		}

		{
			World w3(std::move(w));
			w3.Update();
		}
	}
	//rsrc.verbose = false;
	{
		World w(&rsrc);
		w.entityMngr.cmptTraits.Register<Buffer, C>();
		w.systemMngr.RegisterAndCreate<FillSystem>();
		w.systemMngr.Activate<FillSystem>();
		
		for (std::size_t i = 0; i < 100000; i++)
			w.entityMngr.Create(TypeIDs_of<Buffer>);

		w.Update();
	}

	rsrc.release();

	return 0;
}
