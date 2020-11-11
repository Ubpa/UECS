#include <UECS/World.h>

using namespace Ubpa::UECS;

struct Translation { float value{ 0.f }; };
struct LocalToParent { float value{ 0.f }; };
struct LocalToWorld { float value{ 0.f }; };
struct Children { std::vector<Entity> value; };
struct Parent { Entity value; };

struct TranslationSystem {
	static void SetTreeL2W(World* w, const Entity& entity, const LocalToWorld* l2w) {
		if(!w->entityMngr.Have<LocalToParent>(entity) || !w->entityMngr.Have<LocalToWorld>(entity))
			return;
		auto* el2p = w->entityMngr.Get<LocalToParent>(entity);
		auto* el2w = w->entityMngr.Get<LocalToWorld>(entity);
		el2w->value = l2w->value + el2p->value;

		if(w->entityMngr.Have<Children>(entity)) {
			auto* children = w->entityMngr.Get<Children>(entity);
			for (const auto& child : children->value)
				SetTreeL2W(w, child, el2w);
		}
	}
	
	static void OnUpdate(Schedule& schedule) {
		{
			ArchetypeFilter filter;
			filter.all.insert(CmptType::Of<Parent>);
			schedule.RegisterEntityJob([](LocalToParent* l2p, const Translation* t) {
				l2p->value = t->value;
			}, "T2LocalToParent", true, filter);
		}
		{
			ArchetypeFilter filter;
			filter.none.insert(CmptType::Of<Parent>);
			schedule.RegisterEntityJob([](LocalToWorld* l2w, const Translation* t) {
				l2w->value = t->value;
			}, "T2LocalToWorld", true, filter);
		}
		{
			ArchetypeFilter filter;
			RandomAccessor randomAccessor;
			randomAccessor.types = {
				CmptAccessType::Of<Write<LocalToWorld>>,
				CmptAccessType::Of<Latest<LocalToParent>>,
				CmptAccessType::Of<Latest<Children>>
			};
			filter.none.insert(CmptType::Of<Parent>);
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
	auto [sys] = w.systemMngr.systemTraits.Register<TranslationSystem>();
	auto [e1, c, t1, l2w1] = w.entityMngr.Create<Children, Translation, LocalToWorld>();
	auto [e2, p2, t2, l2p2, l2w2] = w.entityMngr.Create<Parent, Translation, LocalToParent, LocalToWorld>();
	auto [e3, p3, t3, l2p3, l2w3] = w.entityMngr.Create<Parent, Translation, LocalToParent, LocalToWorld>();
	
	c->value = { e2, e3 };
	p2->value = e1;
	p3->value = e1;
	
	t1->value = 1.f;
	t2->value = 2.f;
	t3->value = 3.f;
	
	w.systemMngr.Activate(sys);
	w.Update();

	std::cout << w.GenUpdateFrameGraph().Dump() << std::endl;
	std::cout << w.DumpUpdateJobGraph() << std::endl;
	return 0;
}
