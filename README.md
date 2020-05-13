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
    w.entityMngr.CreateEntity<Position, Velocity>();
    w.Update();
}
```

## Compare with Unity ECS

UECS's primary reference project is Unity's ECS.

Read [compare.md](compare.md) for details.

## TODO

Read [todo.md](todo.md) for details.

## Licensing

You can copy and paste the license summary from below.

```
MIT License

Copyright (c) 2020 Ubpa

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

