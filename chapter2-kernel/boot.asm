	org	0x7c00	

BaseOfStack	equ	0x7c00

BaseOfLoader	equ	0x1000
OffsetOfLoader	equ	0x00

RootDirSectors	equ	14
SectorNumOfRootDirStart	equ	19
SectorNumOfFAT1Start	equ	1
SectorBalance	equ	17	

	jmp	short Label_Start
	nop
	BS_OEMName	db	'MINEboot'
	BPB_BytesPerSec	dw	512
	BPB_SecPerClus	db	1
	BPB_RsvdSecCnt	dw	1
	BPB_NumFATs	db	2
	BPB_RootEntCnt	dw	224
	BPB_TotSec16	dw	2880
	BPB_Media	db	0xf0
	BPB_FATSz16	dw	9
	BPB_SecPerTrk	dw	18
	BPB_NumHeads	dw	2
	BPB_HiddSec	dd	0
	BPB_TotSec32	dd	0
	BS_DrvNum	db	0
	BS_Reserved1	db	0
	BS_BootSig	db	0x29
	BS_VolID	dd	0
	BS_VolLab	db	'boot loader'
	BS_FileSysType	db	'FAT12   '

Label_Start:

	mov	ax,	cs
	mov	ds,	ax
	mov	es,	ax
	mov	ss,	ax
	mov	sp,	BaseOfStack

;=======	clear screen

	mov	ax,	0600h
	mov	bx,	0700h
	mov	cx,	0
	mov	dx,	0184fh
	int	10h

;=======	set focus

	mov	ax,	0200h
	mov	bx,	0000h
	mov	dx,	0000h
	int	10h

;=======	display on screen : Start Booting......

	mov	ax,	1301h
	mov	bx,	000fh
	mov	dx,	0000h
	mov	cx,	10
	push	ax
	mov	ax,	ds
	mov	es,	ax
	pop	ax
	mov	bp,	StartBootMessage
	int	10h

;=======	reset floppy

	xor	ah,	ah
	xor	dl,	dl
	int	13h

;=======	search loader.bin
	mov	word	[SectorNo],	SectorNumOfRootDirStart

;==== 搜索出引导加载程序，文件名为loader.bin
Lable_Search_In_Root_Dir_Begin:
	
	cmp	word	[RootDirSizeForLoop],	0
	jz	Label_No_LoaderBin
	dec	word	[RootDirSizeForLoop]
	;==== ax清零，同时清理es寄存器，es扩展段寄存器；
	mov	ax,	00h
	mov	es,	ax
	mov	bx,	8000h
	;==== 保存根目录的起始扇区号
	mov	ax,	[SectorNo]
	mov	cl,	1
	;==== 从根目录中读取一个扇区的数据到缓冲区
	call	Func_ReadOneSector
	mov	si,	LoaderFileName
	mov	di,	8000h
	cld
	mov	dx,	10h
	
Label_Search_For_LoaderBin:

	cmp	dx,	0
	jz	Label_Goto_Next_Sector_In_Root_Dir
	dec	dx
	mov	cx,	11

Label_Cmp_FileName:

	cmp	cx,	0
	jz	Label_FileName_Found
	dec	cx
	lodsb	
	cmp	al,	byte	[es:di]
	jz	Label_Go_On
	jmp	Label_Different

Label_Go_On:
	
	inc	di
	jmp	Label_Cmp_FileName

Label_Different:

	and	di,	0ffe0h
	add	di,	20h
	mov	si,	LoaderFileName
	jmp	Label_Search_For_LoaderBin

Label_Goto_Next_Sector_In_Root_Dir:
	
	add	word	[SectorNo],	1
	jmp	Lable_Search_In_Root_Dir_Begin
	
;=======	display on screen : ERROR:No LOADER Found

Label_No_LoaderBin:

	mov	ax,	1301h
	mov	bx,	008ch
	mov	dx,	0100h
	mov	cx,	21
	push	ax
	mov	ax,	ds
	mov	es,	ax
	pop	ax
	mov	bp,	NoLoaderMessage
	int	10h
	jmp	$

;=======	found loader.bin name in root director struct

Label_FileName_Found:
	;====== 开始准备加载文件到内存指定位置的前期工作
	;====== 计算文件起始扇区号
	;====== 设置好用于加载文件的内存段地址和偏移地址
	
	;====== RootDirSectors是根目录占用的扇区数，后续用于计算文件的
	mov	ax,	RootDirSectors
	;====== di寄存器指向根目录中目标文件的目录项起始位置，与0ffe0h将目标寄存器的低5位清零
	;====== 目的是将di对齐到32字节，确保di指向目标文件的目录项起始位置
	and	di,	0ffe0h
	;====== FAT12文件系统中，每个目录项占用32字节，所以需要将di向后移32字节
	add	di,	01ah
	;====== es是段寄存器，di是偏移的内存单元
	;====== 将目标文件的起始簇号（一个16位的值）读取到寄存器cx中；
	mov	cx,	word	[es:di]
	;====== 起始簇号压栈保存，在Label_Go_On_Loading_File中pop ax取出起始的簇号
	push	cx
	;====== 起始簇号+根目录占据的扇区数，是计算文件起始扇区号的第一步；
	add	cx,	ax
	;====== SectorBalance一个预先定义的常量，代表一个平衡扇区数，通常用于调整扇区号的计算。
	;====== cx最终得到起始扇区号
	add	cx,	SectorBalance
	;====== BaseOfLoader 是一个预先定义的常量，表示loader.bin要被加载到的内存段基地址。
	mov	ax,	BaseOfLoader
	;====== 将ax的值赋值给es，后续在使用 es 作为段基址进行内存访问时，就可以将数据加载到指定的内存段中。
	mov	es,	ax
	;====== OffsetOfLoader 是一个预先定义的常量（0x00），表示loader.bin在内存段中的偏移地址。
	mov	bx,	OffsetOfLoader
	;====== 最后将得到的起始扇区号存放至ax寄存器中
	mov	ax,	cx

Label_Go_On_Loading_File:
	;====== 实现文件循环加载，通常用于从磁盘中读取文件内容
	;====== 每次循环时打印一个 . ，用来表示进度
	push	ax
	push	bx
	;====== int 10h, ah=0eh，显示一个字符
	mov	ah,	0eh
	mov	al,	'.'
	;====== bl=0fh，设置颜色为白色
	mov	bl,	0fh
	int	10h
	pop	bx
	pop	ax

	;====== cl设置为1，表示要读取的扇区个数为1
	mov	cl,	1
	;====== 从磁盘读取一个扇区的数据到内存中，这个内存位置由 es:bx 指定
	call	Func_ReadOneSector
	;====== 从栈中弹出之前保存的文件起始扇区号，恢复ax的值
	pop	ax
	;====== 根据当前的簇号计算并获取文件的下一个簇号
	call	Func_GetFATEntry
	;====== 当下一个簇号为0x0fffh，在FAT12文件系统中表示文件的最后一个簇
	cmp	ax,	0fffh
	;====== 如果ax=0fff，表示文件已经加载完成
	jz	Label_File_Loaded
	;====== 如果没有加载完成的话，将当前的簇号压栈保存
	push	ax
	;====== 将根目录占用的扇区数存放至dx中
	mov	dx,	RootDirSectors
	;====== 当前簇号 + 根目录占据的扇区数 + 平衡扇区数，得到下一个扇区号
	add	ax,	dx
	add	ax,	SectorBalance
	;====== bx表示当前的内存偏移地址，加上每个扇区的字节数，得到下一个内存偏移地址
	;====== bx在函数Func_ReadOneSector中使用
	add	bx,	[BPB_BytesPerSec]
	jmp	Label_Go_On_Loading_File

Label_File_Loaded:
	
	jmp	BaseOfLoader:OffsetOfLoader

;=======	read one sector from floppy

Func_ReadOneSector:
	;====== bp寄存器保存了什么？
	push	bp
	;====== 设置bp为当前栈帧的基址，方便访问局部变量和参数
	mov	bp,	sp
	;====== esp和sp有什么区别？
	;====== esp是栈指针寄存器，指向栈顶，sp是堆栈指针寄存器，指向栈底。
	;====== 在栈上分配2字节的空间，用于存储局部变量
	sub	esp,	2
	;====== 将cl的值存入局部变量，cl通常表示要读取的扇区数
	mov	byte	[bp - 2],	cl
	;====== 压栈，以便在函数结束时恢复
	push	bx
	;====== SecPerTrk表示每个磁道的扇区数，用于计算磁道和扇区号
	mov	bl,	[BPB_SecPerTrk]
	;====== 将寄存器ax的值除以bl，商存储在al中（磁头号），余数存储在ah中
	;====== 计算当前逻辑扇区号对应的磁道号和扇区号
	div	bl
	;====== 扇区号从1开始，余数从0开始，所以需要加1（扇区号）
	inc	ah
	;====== cl用于存储扇区号，供BIOS中断使用
	mov	cl,	ah
	;====== dh用于存储磁头号，供BIOS中断使用
	mov	dh,	al
	;====== 将al寄存器的值右移一位，计算柱面号（磁道号的高位部分）
	shr	al,	1
	;====== ch用于存储柱面号，供BIOS中断使用
	mov	ch,	al
	;====== dh与1按位与，得到磁道号的低位部分，提取磁头号的低位部分，因为磁头号只有0和1两种可能
	and	dh,	1
	;====== 恢复栈，但bp还未恢复
	pop	bx
	;====== 将内存地址[BS_DrvNum]处的值赋值给dl，BS_DrvNum表示驱动器号(0表示软盘A，1表示软盘B)，供BIOS中断使用
	mov	dl,	[BS_DrvNum]
Label_Go_On_Reading:
	;====== int 13h, ah=2，BIOS功能号，读取扇区
	mov	ah,	2
	;====== al表示需要读取的扇区数，这里是逐个读取
	;====== 读取之前通过cl传入的局部变量，始终为1
	mov	al,	byte	[bp - 2]
	;====== 调用BIOS中断，读取扇区
	int	13h
	;====== 如果读取失败，则跳转到Label_Go_On_Reading继续读取
	;====== jc检查CF，如果CF=1，表示读取失败，这种错误通常是因为磁盘没有找到或磁盘错误
	jc	Label_Go_On_Reading
	;====== 释放之前的局部变量
	add	esp,	2
	pop	bp
	ret

;=======	get FAT Entry

Func_GetFATEntry:
	;====== 保存es和bx、ax数据
	push	es
	push	bx
	push	ax
	;====== es 段地址设置为0，表示使用段地址0x0000
	mov	ax,	00
	mov	es,	ax
	;====== 取出ax，即当前的簇号
	pop	ax
	;====== 初始化这里的变量Odd，标志着是奇数簇号还是偶数簇号
	mov	byte	[Odd],	0
	;====== 先执行乘三的操作，乘的结果存放在ax中
	mov	bx,	3
	mul	bx
	;====== 再执行除2的操作，ax存商，dx存余数
	mov	bx,	2
	div	bx
	;====== 如果余数为0，表示当前簇号是偶数簇号，否则是奇数簇号
	cmp	dx,	0
	;====== 如果是0就直接执行Label_Even，跳过Odd赋值1的操作
	jz	Label_Even
	mov	byte	[Odd],	1

Label_Even:
	;====== 将dx清零
	xor	dx,	dx
	;====== bx赋值每扇区字节数 - 512，用于计算FAT表项的扇区偏移量
	mov	bx,	[BPB_BytesPerSec]
	;====== 计算FAT表项所在的扇区号，以及扇区内偏移量，商是扇区号，余数是扇区内偏移量
	div	bx
	push	dx
	;====== 设置目标内存地址的偏移量
	mov	bx,	8000h
	;====== 将扇区号再加上FAT1起始的扇区号，得到FAT表项所在的扇区号
	add	ax,	SectorNumOfFAT1Start
	;====== 设置变量，调用ReadOneSector时读取两个扇区
	mov	cl,	2
	call	Func_ReadOneSector
	;====== 弹出扇区内偏移量
	pop	dx
	;====== 计算FAT表项在内存中的实际位置
	add	bx,	dx
	;====== 将内存地址赋值给ax
	mov	ax,	[es:bx]
	;====== 如果Odd为1，表示当前簇号是奇数簇号，需要右移4位
	cmp	byte	[Odd],	1
	;====== 如果簇号是偶数，跳转到Label_Even_2
	jnz	Label_Even_2
	;====== 如果簇号是奇数，右移4位
	shr	ax,	4

Label_Even_2:
	;====== 取出低4位，即FAT表项的值，低12位表示有效簇号
	and	ax,	0fffh
	;====== 弹出栈
	pop	bx
	pop	es
	;====== 返回FAT表项的值
	ret

;=======	tmp variable

RootDirSizeForLoop	dw	RootDirSectors
SectorNo		dw	0
Odd			db	0

;=======	display messages

StartBootMessage:	db	"Start Boot"
NoLoaderMessage:	db	"ERROR:No LOADER Found"
LoaderFileName:		db	"LOADER  BIN",0

;=======	fill zero until whole sector

	times	510 - ($ - $$)	db	0
	dw	0xaa55