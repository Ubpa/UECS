# `Entity` 

Entity = `index` + `version` 

## Fields

### `size_t index` 

The ID of an entity.

Entity indexes are recycled when an entity is destroyed. When an entity is destroyed, the EntityManager increments the version identifier. To represent the same entity, both the Index and the Version fields of the Entity object must match. If the Index is the same, but the Version is different, then the entity has been recycled.

`version` 

The generational version of the entity.

The Version number can, theoretically, overflow and wrap around within the lifetime of an application. For this reason, you cannot assume that an Entity instance with a larger Version is a more recent incarnation of the entity than one with a smaller Version (and the same Index).

## Methods

- `size_t Idx() const`: entity's index.
- `size_t Version() const`: entity's version.

# `CmptType` 

an ID to identify component's type, use `TypeID<Component>`(a **compile-time** hash code of `Component`) as default.

## Fields

### `size_t hashcode` 

ID, it's unique in global.

## Methods

- `constexpr CmptType(size_t id)`: custom ID
- `constexpr CmptType(std::string_view)`: use `RuntimeTypeID(std::string_view)` (use [fnv1a](https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function)) to hash the string
- `template<typename Cmpt> static constexpr CmptType Of()`: use `TypeID<Cmpt>` as component's ID
- `constexpr size_t HashCode()`: ID
- `template<typename Cmpt> static constexpr CmptType HashCodeOf()`: component's ID, equal to `CmptType::Of<Cmpt>().HashCode()` 
- `template<typename Cmpt> bool Is() const`: if ID is same with `TypeID<Cmpt>`, return `true`, else return `false` 

# `CmptPtr` 

`CmptPtr` = `CmptType` + `void*` 

## Fields

###  `CmptType type` 

`CmptType` use to identify `void*` pointer's actual type.

### `void* ptr` 

a no-type pointer of a component.

## Methods

- `CmptPtr(CmptType, void*)`: constructor
- `template<typename Cmpt> CmptPtr(Cmpt*)`: use `CmptType::Of<Cmpt>` as `type` 
- `CmptType Type() const`: get `type` 
- `void* Ptr() const`: get `ptr` 
- `template<typename Cmpt> Cmpt* As() const:`: reinterpret `ptr` 

# `RTDCmptTraits` 

TODO

`EntityMngr` 

The `EntityMngr` manages entities and components in a `World`.

## Fields

### `std::vector<EntityInfo> entityTable` 

an array to store entities's infomation.

> ```c++
> struct EntityInfo {
>   Archetype* archetype{ nullptr };
>   size_t idxInArchetype{ size_t_invalid };
>   size_t version{ 0 }; // version
> };
> ```

### `std::vector<std::function<void()>> commandBuffer` 

a buffer to store commands added in `World::Update()` for methods which can not run in worker threads.

## Methods

**Entity**  

- `template<typename... Cmpts> std::tuple<Entity, Cmpts*...> Create()`: create entity with components `Cmpts...`, call `Cmpts...`'s default constructor

  > The EntityManager creates the entity in the first available chunk with the matching archetype that has enough space.

- `Entity Create(const CmptType* types, size_t num)`: create entity with the array `types` (length `num`), you should register those types in `RTDCmptTraits` 

-  `Entity Create(CmptTypes...)`: a convenient interface for `Entity Create(const CmptType* types, size_t num)` 

- `Entity Instantiate(Entity e)`: copy `Entity`. if `e` is invalid, throw `std::invalid_argument`.

- `bool Exist(Entity) const`: reports whether an Entity object is still valid.

  > An Entity object does not contain a reference to its entity. Instead, the Entity struct contains an index and a generational version number. When an entity is destroyed, the EntityManager increments the version of the entity within the internal array of entities. The index of a destroyed entity is recycled when a new entity is created.
  >
  > After an entity is destroyed, any existing Entity objects will still contain the older version number. This function compares the version numbers of the specified Entity object and the current version of the entity recorded in the entities array. If the versions are different, the Entity object no longer refers to an existing entity and cannot be used.

**Component** 

if `e` is invalid, throw `std::invalid_argument`.

- `template<typename Cmpts> std::tuple<Cmpts*...> Attach(Entity)`: adds components `Cmpts...` to an entity, call components `Cmpts...`'s default constructor.

  > move the entity from an archetype to another archetype, call all original components' move constructor, and the original archetype will shrink the chunk by moving the back entity to the original place of the entity

- `void Attach(Entity, const CmptType* types, size_t num)`: adds components `types`(length `num`) to the entity.

  > construct: call `RTDCmptTraits`' default constructor if user has registered customed default constructor.
  >
  > move: call `RTDCmptTraits`' move constructor or memcpy if user hasn't registered customed move constructor.

- `template<typename... CmptTypes> std::array<CmptPtr, sizeof...(CmptTypes)> Attach(Entity, CmptTypes...)`: a convenient interface for `void Attach(Entity, const CmptType* types, size_t num)`.

- `template<typename Cmpt, typename... Args> Cmpt* Emplace(Entity, Args&&...)`: add a component `Cmpt` to the entity, using the constructor `Cmpt(Args...)`.

- `template<typename Cmpts> std::tuple<Cmpts*...> Detach(Entity)`: removes components `Cmpts...` to an entity, call components `Cmpts...`'s destructor.

  > move the entity from an archetype to another archetype, call all original components' move constructor, and the original archetype will shrink the chunk by moving the back entity to the original place of the entity

- `void Attach(Entity, const CmptType* types, size_t num)`: removes components `types`(length `num`) to the entity.

  > destruct: call `RTDCmptTraits`' destructor if user has registered customed destructor.
  >
  > move: call `RTDCmptTraits`' move constructor or memcpy if user hasn't registered customed move constructor.

- `template<typename... CmptTypes> std::array<CmptPtr, sizeof...(CmptTypes)> Attach(Entity, CmptTypes...)`: a convenient interface for `void Attach(Entity, const CmptType* types, size_t num)`.

- `template<typename Cmpt> bool Have(Entity) const`: checks whether an entity has a specific type of component.

- `bool Have(Entity, CmptType) const`: checks whether an entity has a specific type of component.

- `template<typename Cmpt> Cmpt* Get(Entity) const`: get the component of an entity, if the entity hasn't the component. return `nullptr`.

- `template<typename Cmpt> Cmpt* Get(Entity) const`: get the component of an entity, if the entity hasn't the component. return `CmptPtr` with `nullptr`.

- `std::vector<CmptPtr> Components(Entity) const`: get all components of an entity.

**Other** 

- `size_t EntityNum(const EntityQuery&) const`: get the number of entities which match the query.
- `void AddCommand(const std::function<void()>& command)`: add a command in updating and run at the end of `World::Update` in the main thread.

# `CmptTag` 

TODO

# `EntityLocator` 

TODO

# `EntityFilter` 

TODO

# `EntityQuery` 

TODO

# `SystemFunc` 

TODO

# `Schedule` 

TODO

# `SystemMngr` 

TODO

# `World` 

TODO

