# UECS
Ubpa Entity-Component-System in Unity-style

## Example

```c++
#include <UECS/World.h>

struct Position { float val; };
struct Velocity { float val; };

struct MoverSystem {
    static void OnUpdate(Ubpa::Schedule& schedule) {
        schedule.Request(
            [](const Velocity* v, Position* p) {
                p->val += v->val;
            }, "MoverSystem");
    }
};

int main() {
    Ubpa::World w;
    w.systemMngr.Register<MoverSystem>();

    for (size_t i = 0; i < 10; i++)
        w.entityMngr.CreateEntity<>();

    w.Update();
}
```

## Compare with Unity ECS

UECS's primary reference project is Unity's ECS.

Read [compare.md](compare.md) for details.

## TODO

### core

- [x] ~~EntityData: version~~ 
- [x] CmptIDSet hash
- [x] CmptType
- [x] EntityQuery
- [x] Query Entity
- [x] ~~SystemMngr += ScheduleRegistrar~~ 
- [x] alignment
- [x] `Entity` alias table: `Entity::idx -> (Archetype, idx)` 
- [x] `Entity` as special `Component` stored in `Chunk` 
- [x] `None` Parallel
- [x] System Override (dynamic `None`)
- [x] Archetype += RuntimeCmptTraits
- [ ] index in Query
- [ ] Shared Component
- [ ] system group

### non-core

- [ ] exception
- [ ] return
- [x] remove `EntityMngr::ai2ei` 
- [ ] copy
- [ ] batch create
- [ ] Archetype share `Pool<Chunk>` 

### tool

- [ ] SysFuncGraph dump

