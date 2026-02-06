/* 全局标号 */
.global _start

_start:
  # enable all clock
  ldr r0, =0x020C4068
  ldr r1, =0xFFFFFFFF
  str r1, [r0]
  
  ldr r0, =0x020C406C
  ldr r1, =0xFFFFFFFF
  str r1, [r0]

  ldr r0, =0x020C4070
  ldr r1, =0xFFFFFFFF
  str r1, [r0]

  ldr r0, =0x020C4074
  ldr r1, =0xFFFFFFFF
  str r1, [r0]

  ldr r0, =0x020C4078
  ldr r1, =0xFFFFFFFF
  str r1, [r0]

  ldr r0, =0x020C407C
  ldr r1, =0xFFFFFFFF
  str r1, [r0]

  ldr r0, =0x020C4080
  ldr r1, =0xFFFFFFFF
  str r1, [r0]

  # 然后到了设置MUX寄存器的时候了
  ldr r0, =0x020E0068
  ldr r1, =0x5
  str r1, [r0]

  # 设置IO属性
  ldr r0, =0x020E02F4
  ldr r1, =0x10B0
  str r1, [r0]

  # 首先设置输出方向
  # 找到GPIO1_GDIR寄存器的内容
  ldr r0, =0x0209C004
  # 因为是GPIO1_IO03所以需要设置bit3为1，表示输出
  ldr r1, =0x8
  str r1, [r0]

  # 然后找到GPIO1_DR寄存器
  ldr r0, =0x0209C000
  # 这样设置有点粗暴
  ldr r1, =0x0
  str r1, [r0]

loop:
   b loop

