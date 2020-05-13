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

Read [todo.md](todo.md) for details.

