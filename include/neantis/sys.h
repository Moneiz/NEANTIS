extern int sys_setup();
extern int sys_fork();
extern int sys_pause();
extern int sys_sync();

fn_ptr sys_call_table[] = {
    sys_setup, sys_fork, sys_pause, sys_sync
}