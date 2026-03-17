.global _start

// 函数从这里开始运行，主要是设置C的运行环境
_start:
    ldr pc, =Reset_Handler        /* 复位中断 */
    ldr pc, =Undefined_Handler    /* 未定义指令中断 */
    ldr pc, =SVC_Handler          // Supervisor 中断
    ldr pc, =PrefAbort_Handler    // 预取终止中断
    ldr pc, =DataAbort_Handler    // 数据终止中断
    ldr pc, =NotUsed_Handler      // 未使用中断
    ldr pc, =IRQ_Handler          // IRQ中断
    ldr pc, =FIQ_Handler          // FIQ中断（快速中断）

Reset_Handler:
    cpsid i                       // 关闭全局中断
    /*
        关闭I/D cache和MMU，为什么要关呢？
        采取读-改-写的方式
    */
    mrc p15, 0, r0, c1, c0, 0    // 看下C1寄存器的读取方式
    bic r0, r0, #(0x1 << 12)
    bic r0, r0, #(0x1 << 2)
    bic r0, r0, #0x2
    bic r0, r0, #(0x1 << 11)
    bic r0, r0, #0x1
    mcr p15, 0, r0, c1, c0, 0    // 再写回去

#if 0
    // 设置中断向量表偏移
    // 设置中断向量表重映射
    ldr r0, =0x87800000
    isb
    mcr p15, 0, r0, c12, c0, 0
    dsb
    isb
#endif

    // 设置各个模式下的栈指针
    // IMX6UL的堆栈是“向下增长”的
    // 堆栈指针地址一定要是4字节地址对齐的
    // DDR范围：0x80000000~0x9FFFFFFF
    // 进入IRQ模式
    mrs r0, cpsr
    bic r0, r0, #0x1f
    orr r0, r0, #0x12
    msr cpsr, r0
    ldr sp, =0x80600000

    // 进入SYS模式
    mrs r0, cpsr
    bic r0, r0, #0x1f
    orr r0, r0, #0x1f
    msr cpsr, r0
    ldr sp, =0x80400000

    // 进入SVC模式
    mrs r0, cpsr
    // 将r0的低5位清零，也就是cpsr的M0~M4
    bic r0, r0, #0x1f
    // r0或上0x13，表示使用SVC模式
    orr r0, r0, #0x13
    // 将r0的值写回cpsr_c寄存器
    msr cpsr, r0
    ldr sp, =0x80200000 // 设置栈指针

    cpsie i        // 打开全局中断

#if 0
    // 使能IRQ中断
    mrs r0, cpsr                  // 读取cpsr到r0中
    bic r0, r0, #0x80             // 将bit 7清零，也就是cpsr寄存器中的I位清零，表示允许IRQ中断
    msr cpsr, r0                  // 将r0重新写入到cpsr当中
#endif
    b main

// 未定义中断
Undefined_Handler:
    ldr r0, =Undefined_Handler
    bx r0

// SVC中断
SVC_Handler:
    ldr r0, =SVC_Handler
    bx r0

// 预取终止中断
PrefAbort_Handler:
    ldr r0, =PrefAbort_Handler
    bx r0

// 数据终止中断
DataAbort_Handler:
    ldr r0, =DataAbort_Handler
    bx r0

// 未使用的中断
NotUsed_Handler:
    ldr r0, =NotUsed_Handler
    bx r0

// 这些中断向量后面会不会在C语言中重写？

// IRQ中断！
IRQ_Handler:
    push {lr}               // 保存lr地址
    push {r0-r3, r12}       // 保存r0-r3，r12寄存器

    mrs r0, spsr            // 读取spsr寄存器
    push {r0}               // 保存spsr寄存器

    mrc p15, 4, r1, c15, c0, 0  // 将cp15的c15内的值存入r1寄存器中
    // 需要细看一下这里的cp15的操作都是什么含义

    add r1, r1, #0x2000     // GIC基地址加0x2000，得到CPU接口端基地址
    ldr r0, [r1, #0xC]      // CPU接口端基地址加0x0C就是GICC_IAR寄存器，GICC_IAR保存着当前发生中断的中断号，要根据这个中断号来决定调用哪个中断服务函数
    push {r0, r1}

    cps #0x13               // 进入SVC模式，允许其他中断再次进入

    push {lr}               // 保存SVC模式下的lr寄存器
    ldr r2, =system_irqhandler // 加载c语言中断处理函数到r2寄存器中
    blx r2                  // 运行C语言的中断处理函数，带有一个参数
    // 所以上面是进入SVC模式后再去运行这个中断处理函数
    // r0作为参数传入，r0当中存储着中断号，那么就是根据这个中断号在C语言中决定使用哪个
    // 中断处理函数

    pop {lr}                // 执行完中断处理函数，出栈
    cps #0x12               // 进入IRQ模式
    pop {r0, r1}            // r0 r1出栈，成对

    str r0, [r1, #0x10]     // 中断执行完成，写EOIR

    pop {r0}
    msr spsr_cxsf, r0       // 恢复spsr

    pop {r0-r3, r12}        // r0-r3，r12出栈
    pop {lr}
    subs pc, lr, #4         // 将lr-4赋值给pc

FIQ_Handler:
    ldr r0, =FIQ_Handler
    bx r0
