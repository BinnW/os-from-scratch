//=======     GDT_Table
//======= 将全局描述符表GDT结构、中断描述符表IDT结构、任务状态段TSS结构放在内核程序的数据段内
.section .data

// 伪指令.globl修饰，使标识符可以被外部程序引用或访问
.globl GDT_Table

// 手动配置GDT内的各个段描述符
GDT_Table:
    // 每个.quad都对应一个段描述符
    // 第一个段描述符为空，这是标准做法，通常索引0为空，用于防止未初始化的段寄存器访问非法内存
    .quad     0x0000000000000000     /*0 NULL descriptor 00*/
    // 64位内核代码段，用于运行内核的64位代码，
    .quad     0x0020980000000000     /*1 KERNEL Code 64-bit Segment 08*/
    // 64位内核数据段，用于内核数据访问（栈以及静态数据等）
    .quad     0x0000920000000000     /*2 KERNEL Data 64-bit Segment 10*/
    .quad     0x0020f80000000000     /*3 USER   Code 64-bit Segment 18*/
    .quad     0x0000f20000000000     /*4 USER   Data 64-bit Segment 20*/
    .quad     0x00cf9a000000ffff     /*5 KERNEL Code 32-bit Segment 28*/
    .quad     0x00cf92000000ffff     /*6 KERNEL Data 32-bit Segment 30*/
    // 填充的语法，保留10个8字节的空间，可能用于TSS或其他系统段
    .fill     10,8,0                  /*8～9    TSS (jmp one segment <7>) in long-mode 128-bit 40*/
GDT_END:

// 64位系统下，GDT的设置主要是设置特权级以及段寄存器的属性信息

GDT_POINTER:
// 定义GDT的长度，END-TABLE-1，
GDT_LIMIT:     .word     GDT_END - GDT_Table -1
// 定义GDT的起始地址
GDT_BASE:      .quad     GDT_Table

//=======      IDT_Table

.globl IDT_Table
IDT_Table:
    // 填充512个8字节的空间，主要是为了后续内核程序向IDT设置中断/异常处理程序
    .fill 512,8,0
IDT_END:

IDT_POINTER:
IDT_LIMIT:     .word     IDT_END - IDT_Table -1
IDT_BASE:      .quad     IDT_Table

//=======      TSS64_Table

.globl          TSS64_Table

TSS64_Table:
    // 填充13个8字节空间
    .fill 13,8,0
TSS64_END:

TSS64_POINTER:
TSS64_LIMIT:     .word     TSS64_END - TSS64_Table -1
TSS64_BASE:      .quad     TSS64_Table

// init page
.align 8
// 用.org固定各个页表的起始位置
.org   0x1000
// 页目录 - 顶层页表
__PML4E:
    .quad 0x102007
    .fill 255,8,0
    .quad 0x102007
    .fill 255,8,0

.org   0x2000
__PDPTE:
    .quad 0x103003
    .fill 511,8,0

.org   0x3000
__PDE:
    .quad     0x000083
    .quad     0x200083
    .quad     0x400083
    .quad     0x600083
    .quad     0x800083
    .quad     0xe0000083          /*0x a00000*/
    .quad     0xe0200083
    .quad     0xe0400083
    .quad     0xe0600083          /*0x1000000*/
    .quad     0xe0800083
    .quad     0xe0a00083
    .quad     0xe0c00083
    .quad     0xe0e00083
    .fill     499,8,0


.section .text
.globl _start
// GAS编译器中，_start作为程序的默认起始位置
_start:
    // 还真是AT&T的写法
    // Q：设置这些寄存器的意义是什么？
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %ss
    mov $0x7E00, %esp
    // load GDTR
    lgdt GDT_POINTER(%rip)
    // load IDTR
    lidt IDT_POINTER(%rip)
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    movq $0x7E00, %rsp
    // load cr3，启动IA-32e模式
    movq $0x101000, %rax
    movq %rax, %cr3
    movq switch_seg(%rip), %rax
    pushq $0x08
    pushq %rax
    lretq
    // 64-bit mode code
switch_seg:
    .quad entry64
entry64:
    movq $0x10, %rax
    movq %rax, %ds
    movq %rax, %es
    movq %rax, %gs
    movq %rax, %ss
    movq $0xffff800000007E00, %rsp /* rsp address*/
    movq go_to_kernel(%rip), %rax /* movq address*/
    pushq $0x08
    pushq %rax
    lretq
go_to_kernel:
    .quad Start_Kernel


