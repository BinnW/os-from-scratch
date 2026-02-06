.global _start

// 函数从这里开始运行，主要是设置C的运行环境
_start:
    // 进入SVC模式
    mrs r0, cpsr
    // 将r0的低5位清零，也就是cpsr的M0~M4
    bic r0, r0, #0x1f
    // r0或上0x13，表示使用SVC模式
    orr r0, r0, #0x13
    // 将r0的值写回cpsr_c寄存器
    msr cpsr, r0

    ldr sp, =0x80200000 // 设置栈指针
    b main
