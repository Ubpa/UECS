# TODO

## core

### important

- [x] Entity += version
- [x] CmptTypeSet hash
- [x] CmptType
- [x] EntityQuery
- [x] Query Entity
- [x] alignment
- [x] `Entity` alias table: `Entity::idx -> (Archetype, idx)` 
- [x] `Entity` as special `Component` stored in `Chunk` 
- [x] `None` Parallel
- [x] System Override (dynamic `None`)
- [x] Archetype += RuntimeCmptTraits
- [x] empty argument `SystemFunc` as job
- [x] index in Query
  - [x] SystemFunc support
  - [x] get entity num of query
- [x] instantiate
- [ ] string/CmptType-driven API
  - [ ] RumtimeCustomCmptTraits

### unimportant

- [x] exception: invalid `Entity` 
- [ ] batch create/instantiate
- [ ] lock `FilterChange` 
- [ ] EntityMngr Query-driven API

### maybe deprecate

- [ ] shared component
- [ ] system group

## tool

- [ ] SysFuncGraph dump
- [ ] serialize

## detial

- [x] remove `EntityMngr::ai2ei` 
- [x] Archetype share `Pool<Chunk>` 
- [x] simplify `Schedule` 
- [ ] parallel `Schedule` 
- [x] `constexpr SystemFunc::HashCode()` 
- [x] cache `CmptIDSet`'s hashcode
- [x] store `EntityMngr` and `SystemMngr` instead `World` in `Schedule` 

