```

 __    __   _______   ______     _______.
|  |  |  | |   ____| /      |   /       |
|  |  |  | |  |__   |  ,----'  |   (----`
|  |  |  | |   __|  |  |        \   \    
|  `--'  | |  |____ |  `----.----)   |   
 \______/  |_______| \______|_______/    
                                         

```

⭐ Star us on GitHub — it helps!

[![repo-size](https://img.shields.io/github/languages/code-size/Ubpa/UECS?style=flat)](https://github.com/Ubpa/UECS/archive/master.zip) [![tag](https://img.shields.io/github/v/tag/Ubpa/UECS)](https://github.com/Ubpa/UECS/tags) [![license](https://img.shields.io/github/license/Ubpa/UECS)](LICENSE) 

# UECS

**U**bpa **E**ntity-**C**omponent-**S**ystem in Unity3D-style

## Environment

- VS 2019
- C++ 17
- CMake 16.3 +

## Documentation

- [API](doc/API.md) 
- [Comparison with Unity3D ECS](doc/comparison.md) 
- [TODO](doc/todo.md) 

## Example

```c++
#include <UECS/World.h>

using namespace Ubpa::UECS;

struct Position { float val; };
struct Velocity { float val; };

class MoverSystem : public System {
public:
  using System::System;

  virtual void OnUpdate(Schedule& schedule) override {
    schedule.Register(
      [](const Velocity* v, Position* p) {
        p->val += v->val;
      },
      "Mover"
    );
  }
};

int main() {
  World w;
  w.systemMngr.Register<MoverSystem>();
  w.entityMngr.CreateEntity<Position, Velocity>();
  w.Update();
}
```

**other examples** 

- [read/write tag](src/test/01_tag/main.cpp) 
- [system update order](src/test/02_order/main.cpp) 
- system function with [`Entity`](src/test/03_query_entity/main.cpp), [`indexInQuery`](src/test/09_idx_in_query/main.cpp) 
- [job function](src/test/08_job/main.cpp) 
- [chunk layout optimization with alignment](src/test/05_alignment/main.cpp) 
- [parrallel with `None` filter](src/test/06_none_parallel/main.cpp) 
- [system **overload**](src/test/07_overload/main.cpp) 
- [runtime dynamic component](src/test/11_runtime_cmpt/main.cpp) 
- [generate **frame graph** in **Graphviz**](src/test/12_framegraph/main.cpp) 
- [performance test](src/test/13_performance/main.cpp) 
- [serialize](src/test/14_serialize/main.cpp) 
- [chunk job](src/test/15_chunk_job/main.cpp) 
- [singleton](src/test/16_singleton/main.cpp) 

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

