.global reload_segments
.type reload_segments, %function
reload_segments:
	ljmp $0x8, $reload_data_segments

.global reload_data_segments
.type reload_data_segments, %function
reload_data_segments:
	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %ss
	mov $0x28, %ax
	mov %ax, %fs
	mov $0x30, %ax
	mov %ax, %gs
	ret
