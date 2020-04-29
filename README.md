# UECS
Ubpa Entity-Component-System

## 概念

- 系统 `System`：函数，需要关联一个 ID（字符串）
- 任务 `Job`：给系统准备好数据后得到的无参可执行函数，内部并行或串行
- 调度表 `Schedule`：储存了所有的任务，以及任务之间的约束关系，完成所有声明后可组装出一个大任务
- 调度表注册器 `ScheduleRegistrar`：用户通过它来注册该帧的系统

