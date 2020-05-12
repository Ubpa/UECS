# Compare with Unity ECS

> ref: [Unity Entities 0.10.0-preview.6](https://docs.unity3d.com/Packages/com.unity.entities@0.10/manual/index.html) 

[TOC]

## Core ECS

**ECS concepts** 

**Archetypes** 

**Memory Chunks** 

(TODO) Shared Component 

**Entity queries** 

`Query` = `Filter`(`All`, `Any`, `None`) + `Locator` 

**Jobs** 

cpp-Taskflow

only support "ForEach" 

**System organization** 

(TODO) System Group

### Entities

**Creating entities** 

(TODO) copy

(TODO) batch create

#### Entity Queries

`Query` = `Filter`(`All`, `Any`, `None`) + `Locator`, for `SystemFunc` 

(TODO) shared component filter

(TODO) change filter

#### Worlds

### Components

#### General purpose components

#### Shared components

(TODO)

#### System state components

needless, you can use components constructor, destructor and move comstructor (copy constructor)

#### Dynamic buffer components

needless, you can use `std::vector` in Component

#### Chunk components

(TODO)

### Systems

no instantiation, just use static function `OnUpdate(Schedule&)`，you can regist `SystemFunc`，set update order, dynamic change filter, etc.

#### Creating systems

lifecycle: only support `OnUpdate`, because of no instantiation

(TODO) shared component filter

(TODO) change filter

**special parameters** 

- `[const] Entity` 
- (TODO) `size_t int entityInQueryIndex` 
- (TODO) `size_t nativeThreadIndex` 

#### System update order

(TODO) System Group

#### Job dependencies

#### Looking up data

#### Entity command buffers

`World::AddCommand()`，run all commands after Update Graph

### Sync points and structural changes

（TODO）

currently, UECS only has a sync point `World::AddCommand()` 

### Component WriteGroups

use `Schedule::{Insert|Erase}{All|Any|None}` to dynamic change a system's filter

### Versions and generations

only support `Entity::Version` 

### C# Job System extensions

cpp-Taskflow

## Creating gameplay



### 