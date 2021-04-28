/*
* Copyright (c) 2020 AIIT XUOS Lab
* XiUOS is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*        http://license.coscl.org.cn/MulanPSL2
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v2 for more details.
*/

#include <xs_base.h>
#include <stdint.h>
#include "svc_handle.h"
#include <xs_service.h>
#include <xs_ktask.h>

static void SvcDispatch(void) __attribute__ ((naked, no_instrument_function));
static void SvcDispatch(void)
{
  __asm__ __volatile__
    (
        " mov r12, sp\n"                /* Calculate (orig_SP - new_SP) */
        " sub r12, r12, #36\n"
        " and r12, r12, #7\n"
        " add r12, r12, #36\n"
        " sub sp,  sp, r12\n"
        " str r0,  [sp, #0]\n" 
        " str r1,  [sp, #4]\n" 
        " str r2,  [sp, #8]\n" 
        " str r3,  [sp, #12]\n"
        " str r4,  [sp, #16]\n"          
        " str r5,  [sp, #20]\n"
        " str r6,  [sp, #24]\n"
        " str lr,  [sp, #28]\n"
        " str r12, [sp, #32]\n"
        " mov r0,  sp\n"
        " ldr r12, =SvcHandle\n"
        " blx r12\n"
        " ldr lr,  [sp, #28]\n"
        " ldr r2,  [sp, #32]\n"
        " add sp, sp, r2\n"
        " svc 1"
    );
}


void __svcall(uintptr_t* contex)
{
    uint32_t svc_number;
    KTaskDescriptorType tid;

    tid = GetKTaskDescriptor();
    svc_number = ((char *)contex[REG_INT_PC ])[-2];
    switch (svc_number) {
    case 0:  //svc handler 
        tid->task_dync_sched_member.svc_return = contex[REG_INT_PC];
        //tid->task_dync_sched_member.exc_return = contex[REG_INT_EXC_RETURN];
        tid->task_dync_sched_member.isolation_status = 1;
        contex[REG_INT_PC] = (uint32_t)SvcDispatch & ~1;
        //contex[REG_INT_EXC_RETURN] = EXC_RETURN_PRIVTHR;
        break;
    case 1:   // svc return 
        contex[REG_INT_PC] = tid->task_dync_sched_member.svc_return;
        //contex[REG_INT_EXC_RETURN] = tid->task_dync_sched_member.exc_return;
        tid->task_dync_sched_member.isolation_status = 0;
        break;
    default:
        KPrintf("unsurport svc call number :%d\n",svc_number);
        break;
    }
}

uintptr_t SvcHandle(uintptr_t *sp)
{
    uint32_t service_num = 0;
    service_num = ((uint32_t) sp[0]);  //r0 
    //KPrintf("SvcHandle service_num :%d\n ",service_num);
    uint8_t param_num =  g_service_table[service_num].param_num;
    uintptr_t *param = sp + 1;
    return g_service_table[service_num].fun(service_num,param,param_num) ;
}


uint32_t GetTaskPrivilege(void){
    uint32_t unprivileg = 0;
    struct TaskDescriptor *task = GetKTaskDescriptor();
    //KPrintf("GetTaskPrivilege : %s\n", task->task_base_info.name);
    if (task->task_dync_sched_member.isolation_flag == 1 && task->task_dync_sched_member.isolation_status == 0) {
       unprivileg = 1;
    } else {
       unprivileg = 0;
    }

    return unprivileg;
}
