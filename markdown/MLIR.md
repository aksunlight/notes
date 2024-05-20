# MLIR

HACC（Agenda Leveraging MLIR to Design for AI Engines）：异构加速计算引擎

ISA-base Vector Programmable：基于指令集架构的矢量可编程



IR（Intermediate Representation）：中间表示

MLIR（Multi-Level Intermediate Representation）：多层级中间表示

MLIR-AIE（Open-Source Multi-level IR toolflow）：开源多级IR工具流，基于MLIR的AI引擎工具链



AMD Versal ACAP 设备的 AI 引擎阵列是一组具有自适应互连的 VLIW 矢量处理器（The AI Engine array of the AMD Versal ACAP device is a set of VLIW vector processors with adaptable interconnect.）

VLIW（Very Long Instruction Word）：超长指令字，一种非常长的指令组合，它把许多条指令连接在一起，增加了运算的速度

我们将首先深入了解 Versal ACAP 架构，更具体地说是 AI 引擎计算和数据移动功能。通过用 MLIR-AIE 方言表达并在 ACAP 设备上执行的小型设计示例，我们将利用 AI 引擎功能来优化日益复杂的设计的性能。









![img](D:\notes\markdown\MLIR.assets\bd4d573f-5cad-43ea-bb78-b4e9ea6bdde3.png)

![img](D:\notes\markdown\MLIR.assets\7089f1c8-e57b-4876-9365-6901e1f0443b.png)

The two major components of an AI Engine tile are

* VLIW processor core declared as `AIE.core(tileName) { body }`

* Local memory buffer declared as `AIE.buffer(tileName) : memref { body }`

方言声明与物理 AI 引擎组件的关系

![img](D:\notes\markdown\MLIR.assets\2898c169-c763-495d-a319-ab2bc1833d14.png)

![img](D:\notes\markdown\MLIR.assets\1b55506f-3496-416d-bcdf-31a4585d7799.png)

%symbolic name



Tile：

For the tile, we simply declare its coordinates by column and row. Tile declaration is mainly needed so other sub components can be associated to the tile by name. The type of tiles and orientation of its associated local memory is architecture dependent.

对于tile，我们只需按列和行声明其坐标。主要需要tile声明，以便其他子组件可以按名称与tile相关联。tile的类型及其关联的本地内存的方向取决于体系结构。



Buffer:

When declaring a buffer, we pass in the associated AIE tile and declare the buffer parameters. Those parameters are the depth and data type. 

声明缓冲区时，我们传入关联的 AIE Tile并声明缓冲区参数。这些参数是深度和数据类型宽度。

One important note about buffers is that one buffer is not strictly mapped to the entire local memory. You can declare multiple buffers that are associated with the local memory of a tile and they would, by default, be allocated sequentially in that tile's local memory.
关于缓冲区的一个重要注意事项是，一个缓冲区不会严格映射到整个本地内存。您可以声明与tile的本地内存关联的多个缓冲区，默认情况下，这些缓冲区将在该tile的本地内存中按顺序分配。

* odd row (1, 3, ...) - memory on right
* even row (2, 4, ...) - memory on left

![img](D:\notes\markdown\MLIR.assets\7699e43e-c819-4b47-99d1-129a1aa74ac8.png)



Core:

The AIE core functionality is defined within the core body. This functionality is a combination of AIE dialect specific operations as well as other dialects that are supported by the MLIR compiler. 

AIE 核心功能在Core中定义。此功能是特定于 AIE 方言的操作以及 MLIR 编译器支持的其他方言的组合。

The `constant` operation produces an SSA value equal to some integer or floating-point constant specified by an attribute. This is the way MLIR forms simple integer and floating point constants.

The `store` operation store a value to a memref location given by indices. The value stored should have the same type as the elemental type of the memref.



![img](D:\notes\markdown\MLIR.assets\217ecff7-1e19-4cdb-a13e-a4ecd8483a79.png)

Lock:

Locks are components used by other components to arbitrate access to a shared resource. They are not tied to any particular resource and thus do not physically prevent the resource that they are associated with from being accessed. Instead, they facilitate a contractual agreement among users of a resource that a lock must be successfully acquired before the resource can be used. A common resource they are associated with are buffers where two users of that buffer (e.g. core and DMA) may wish to access the buffer at the same time.

锁是其他组件用来仲裁对共享资源的访问的组件。它们不绑定到任何特定资源，因此不会在物理上阻止访问与它们关联的资源。相反，它们促进了资源用户之间的相互配合，即必须先成功获取锁，然后才能使用资源。它们关联的公共资源是缓冲区，其中该缓冲区的两个用户（例如Core 和 DMA）可能希望同时访问缓冲区。

The `lock` operation is actually a physical sub component of the `AIE.tile`, but is declared within the `module` block. The syntax for declaring a lock is `AIE.lock(tileName, lockID)`. An example would be:

~~~
%lockName = AIE.lock(%tileName, %lockID) { body }
~~~

To use the lock, we call the `use_lock` operation either inside a `core` operation or `mem`/`shim_dma` operation.

~~~
AIE.use_lock(%lockName, "Acquire|Release", 0|1)
~~~

Each tile has 16 locks and each lock is in one of two states (acquired, released) and one of two values (0, 1). By default, we tend to assume (value=0 is a "write", value=1 is "read"). But there is no real definition of these values. The only key thing to remember is that the lock value starts at `val=0`, and is reset into the release `val=0` state. This means an `acquire=0` will always succeed first, while an `acquire=1` needs the lock state to be `release=1` to succeed. Once acquired, a lock can be released to the 0 or 1 state.

The 16locks in a tile are accessible by its same 3 cardinal neighbors that can access the tile's local memory. This is to ensure all neighbors that can access the local memory can also access the locks.



