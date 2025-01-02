.global wakeup_trampoline
.type wakeup_trampoline, %function
wakeup_trampoline:
	b wakeup_trampoline
