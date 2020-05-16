# Comparison with Unity ECS core

> ref: [Unity Entities 0.10.0-preview.6](https://docs.unity3d.com/Packages/com.unity.entities@0.10/manual/index.html) 
>
> ---
>
> [TOC]

**ECS concepts** 

**Archetypes** 

**Memory Chunks** 

(TODO) Shared Component 

**Entity queries** 

`Query` = `Filter`(`All`, `Any`, `None`) + `Locator` 

**Jobs** 

cpp-Taskflow

support

- `Entities.ForEach`: use `schedule.Register([](<Component-Tag>...){...})` 
- `Job.WithCode`: use `schedule.Register([](){...})`(empty argument)

**System organization** 

(TODO) System Group

use `schedule.Order(<system-name>, <system-name>)` to set system update order

## 1. Entities

**Creating entities** 

(TODO) copy

(TODO) batch create

### 1.1 Entity Queries

`Query` = `Filter`(`All`, `Any`, `None`) + `Locator`, for `SystemFunc` 

(TODO) shared component filter

(TODO) change filter

### 1.2 Worlds

## 2. Components

### 2.1 General purpose components

### 2.2 Shared components

(TODO)

### 2.3 System state components

needless, you can use components constructor, destructor and move comstructor (copy constructor)

### 2.4 Dynamic buffer components

needless, you can use any type (e.g. `std::vector`) in Component

### 2.5 Chunk components

(TODO)

## 3. Systems

no instantiation, just use static function `OnUpdate(Schedule&)`，you can register `SystemFunc`，set update order, dynamic change filter, etc.

### 3.1 Creating systems

lifecycle: only support `OnUpdate`, because of no instantiation

(TODO) shared component filter

(TODO) change filter

**Components** 

use `CmptTag::LastFrame<Cmpt> (like const <Cmpt>*)`, `CmptTag::Write<Cmpt> == <Cmpt> *`, `CmptTag::Lastest<Cmpt> == const <Cmpt>* ` to differentiate read/write and timepoint

- `SystemFunc` with `LastFrame<Cmpt>` run before any `SystemFunc` with `CmptTag::Write<Cmpt>` 

- `SystemFunc` with `CmptTag::Write<Cmpt>` run before any `SystemFunc` with `CmptTag::Lastest<Cmpt>` 

**special parameters** 

- `[const] Entity` 
- `size_t entityInQueryIndex` 
- (not-support) `size_t nativeThreadIndex` 
- `const EntityLocator* locator` 
- `void** cmpts` 

**System kind** 

- components (optional: + `Entity`, `size_t entityInQueryIndex`) : system for each entity
- empty : job
- `const EntityLocator* locator` + `void** cmpts`: run-time dynamic system function

### 3.2 System update order

(TODO) System Group

### 3.3 Job dependencies

### 3.4 Looking up data

### 3.5 Entity command buffers

`World::AddCommand()`，run all commands after Update Graph

## 4. Sync points and structural changes

（TODO）

currently, UECS only has a sync point `World::AddCommand()` 

## 5. Component WriteGroups

use `Schedule::{Insert|Erase}{All|Any|None}` to dynamic change a system's filter

## 6. Versions and generations

only support `Entity::Version` 

(TODO) chunk version

## 7. C# Job System extensions

cpp-Taskflow

