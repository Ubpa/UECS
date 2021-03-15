#include <UECS/World.h>

using namespace Ubpa;
using namespace Ubpa::UECS;

struct Translation { float value{ 0.f }; };
struct LocalToParent { float value{ 0.f }; };
struct LocalToWorld { float value{ 0.f }; };
struct Children { std::vector<Entity> value; };
struct Parent { Entity value; };

struct TranslationSystem {
	static void SetTreeL2W(World* w, const Entity& entity, const LocalToWorld* l2w) {
		if(!w->entityMngr.Have(entity, TypeID_of<LocalToParent>) || !w->entityMngr.Have(entity, TypeID_of<LocalToWorld>))
			return;
		auto* el2p = w->entityMngr.Get<LocalToParent>(entity);
		auto* el2w = w->entityMngr.Get<LocalToWorld>(entity);
		el2w->value = l2w->value + el2p->value;

		if(w->entityMngr.Have(entity, TypeID_of<Children>)) {
			auto* children = w->entityMngr.Get<Children>(entity);
			for (const auto& child : children->value)
				SetTreeL2W(w, child, el2w);
		}
	}
	
	static void OnUpdate(Schedule& schedule) {
		{
			ArchetypeFilter filter;
			filter.all.insert(TypeID_of<Parent>);
			schedule.RegisterEntityJob([](LocalToParent* l2p, const Translation* t) {
				l2p->value = t->value;
			}, "T2LocalToParent", true, filter);
		}
		{
			ArchetypeFilter filter;
			filter.none.insert(TypeID_of<Parent>);
			schedule.RegisterEntityJob([](LocalToWorld* l2w, const Translation* t) {
				l2w->value = t->value;
			}, "T2LocalToWorld", true, filter);
		}
		{
			ArchetypeFilter filter;
			RandomAccessor randomAccessor;
			randomAccessor.types = {
				AccessTypeID_of<Write<LocalToWorld>>,
				AccessTypeID_of<Latest<LocalToParent>>,
				AccessTypeID_of<Latest<Children>>
			};
			filter.none.insert(TypeID_of<Parent>);
			schedule.RegisterEntityJob([](World* w, Children* children, const LocalToWorld* l2w) {
				for(const auto& child : children->value)
					SetTreeL2W(w, child, l2w);
			}, "LocalToWorld", true, filter, {}, {}, randomAccessor);
		}
		{
			schedule.RegisterEntityJob([](const LocalToWorld* l2w) {
				std::cout << l2w->value << std::endl;
			}, "PrintL2W");
		}
		schedule.Order("T2LocalToWorld", "LocalToWorld");
	}
};

int main() {
	World w;
	w.entityMngr.cmptTraits.Register<Translation, LocalToWorld, LocalToParent, Children, Parent>();
	w.systemMngr.RegisterAndActivate<TranslationSystem>();

	auto e1 = w.entityMngr.Create(Ubpa::TypeIDs_of<Children, Translation, LocalToWorld>);
	auto e2 = w.entityMngr.Create(Ubpa::TypeIDs_of<Parent, Translation, LocalToParent, LocalToWorld>);
	auto e3 = w.entityMngr.Create(Ubpa::TypeIDs_of<Parent, Translation, LocalToParent, LocalToWorld>);
	
	w.entityMngr.Get<Children>(e1)->value = { e2, e3 };
	w.entityMngr.Get<Parent>(e2)->value = e1;
	w.entityMngr.Get<Parent>(e3)->value = e1;

	w.entityMngr.Get<Translation>(e1)->value = 1;
	w.entityMngr.Get<Translation>(e2)->value = 2;
	w.entityMngr.Get<Translation>(e3)->value = 3;
	
	w.Update();

	std::cout << w.GenUpdateFrameGraph().Dump() << std::endl;
	std::cout << w.DumpUpdateJobGraph() << std::endl;

	return 0;
}
