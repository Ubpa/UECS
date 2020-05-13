# TODO

## core

### important

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
- [x] empty argument `SystemFunc` as job
- [x] index in Query
  - [x] SystemFunc support
  - [x] get entity num of query
- [ ] Shared Component
- [ ] system group

### unimportant

- [x] exception: invalid `Entity` 
- [ ] copy
- [ ] batch create
- [ ] lock filter change

## tool

- [ ] SysFuncGraph dump
- [ ] serialize

## detial

- [x] remove `EntityMngr::ai2ei` 
- [x] Archetype share `Pool<Chunk>` 
- [ ] simplify `Schedule` 
- [ ] parallel `Schedule` 
- [x] `constexpr SystemFunc::HashCode( )` 
- [x] cache `CmptIDSet`'s hashcode
- [ ] store `EntityMngr` instead `World` in `Schedule` 

