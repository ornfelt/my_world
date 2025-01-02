.global reload_segments
.type reload_segments, %function
reload_segments:
	pushq $0x8
	pushq $reload_data_segments
	lretq

.global reload_data_segments
.type reload_data_segments, %function
reload_data_segments:
	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %ss
	ret
