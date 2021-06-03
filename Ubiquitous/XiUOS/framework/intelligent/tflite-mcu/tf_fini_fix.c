// fix '_fini' not found on gcc risvc 8.2.0
#if (__riscv == 1) && (__GNUC__ == 8) && (__GNUC_MINOR__ == 2)
void _fini (void) {}
#endif