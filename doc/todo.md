# TODO

## core

### important

- [x] `Entity` += `version` 
- [x] `CmptTypeSet` hash
- [x] `CmptType` 
- [x] `EntityQuery` 
- [x] Query Entity
- [x] alignment
- [x] `Entity` alias table: `Entity::idx -> (Archetype, idx)` 
- [x] `Entity` as special `Component` stored in `Chunk` 
- [x] `None` Parallel
- [x] System Overload s(dynamic `None`)
- [x] `Archetype` += `RuntimeCmptTraits` 
- [x] empty argument `SystemFunc` as job
- [x] index in `Query` 
  - [x] `SystemFunc` support
  - [x] get entity num of query
- [x] instantiate
- [x] string/`CmptType`-driven API
  - [x] `RTDCmptTraits` : **R**un-**T**ime **D**ynamic **C**omponent **T**raits
  - [x] run-time dynamic `SystemFunc` 
  - [x] C-style API
- [x] chunk job
- [x] serialize (`IListner`)
- [x] system base -> `System` 

### unimportant

- [x] exception: invalid `Entity` 
- [ ] batch create/instantiate (need benchmark)
- [x] lock `FilterChange` 
- [ ] `EntityMngr` `Query`-driven API
- [x] `CmptsView` = `const EntityLocator* locator + void** cmpts` 
- [ ] Filter Builder

### maybe deprecate

- [ ] shared component
- [ ] system group

## tool

- [x] SysFuncGraph dump
- [x] doc

## detial

- [x] remove `EntityMngr::ai2ei` 
- [x] Archetype share `Pool<Chunk>` 
- [x] simplify `Schedule` 
- [x] `constexpr SystemFunc::HashCode()` 
- [x] cache `CmptTypeSet`'s hashcode
- [x] store `EntityMngr` and `SystemMngr` instead `World` in `Schedule` 

