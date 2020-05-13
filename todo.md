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
- [ ] index in Query
- [ ] Shared Component
- [ ] system group

### unimportant

- [ ] exception
- [ ] return
- [ ] copy
- [ ] batch create

## tool

- [ ] SysFuncGraph dump
- [ ] serialize

## detial

- [x] remove `EntityMngr::ai2ei` 
- [x] Archetype share `Pool<Chunk>` 
- [ ] simplify `Schedule` 
- [ ] parallel `Schedule` 
- [ ] `constexpr SystemFunc::HashCode()` 

