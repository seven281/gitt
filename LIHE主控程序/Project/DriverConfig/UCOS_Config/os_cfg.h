/*
*********************************************************************************************************
*                                               uC/OS-III
*                                          The Real-Time Kernel
*
*                         (c) Copyright 2009-2018; Silicon Laboratories Inc.,
*                                400 W. Cesar Chavez, Austin, TX 78701
*
*                   All rights reserved. Protected by international copyright laws.
*
*                  Your use of this software is subject to your acceptance of the terms
*                  of a Silicon Labs Micrium software license, which can be obtained by
*                  contacting info@micrium.com. If you do not agree to the terms of this
*                  license, you may not use this software.
*
*                  Please help us continue to provide the Embedded community with the finest
*                  software available. Your honesty is greatly appreciated.
*
*                    You can find our product's documentation at: doc.micrium.com
*
*                          For more information visit us at: www.micrium.com
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                          CONFIGURATION FILE
*
* Filename : os_cfg.h
* Version  : V3.07.03
*********************************************************************************************************
*/

#ifndef OS_CFG_H
#define OS_CFG_H


/*使能APP回调函数*/
#define OS_CFG_APP_HOOKS_EN             DEF_DISABLED             /* Enable (DEF_ENABLED) application specific hooks                       */

/*使能参数检查*/
#define OS_CFG_ARG_CHK_EN               DEF_ENABLED             /* Enable (DEF_ENABLED) argument checking                                */

/*使能中断中调用系统API检查*/
#define OS_CFG_CALLED_FROM_ISR_CHK_EN   DEF_ENABLED             /* Enable (DEF_ENABLED) check for called from ISR                        */

/*使能操作系统调试*/
#define OS_CFG_DBG_EN                   DEF_ENABLED             /* Enable (DEF_ENABLED) debug code/variables                             */

/*使能Tick时钟*/
#define OS_CFG_TICK_EN                  DEF_ENABLED             /* Enable (DEF_ENABLED) the kernel tick                                  */

/*使能动态时钟*/
#define OS_CFG_DYN_TICK_EN              DEF_DISABLED             /* Enable (DEF_ENABLED) the Dynamic Tick                                 */

/*使能不支持的系统调用检查*/
#define OS_CFG_INVALID_OS_CALLS_CHK_EN  DEF_ENABLED             /* Enable (DEF_ENABLED) checks for invalid kernel calls                  */

/*使能对象类型检查*/
#define OS_CFG_OBJ_TYPE_CHK_EN          DEF_ENABLED             /* Enable (DEF_ENABLED) object type checking                             */

/*使能时间帧计数器*/
#define OS_CFG_TS_EN                    DEF_ENABLED             /* Enable (DEF_ENABLED) time stamping                                    */

/*设置最大系统优先级*/
#define OS_CFG_PRIO_MAX                 32u                     /* Defines the maximum number of task priorities (see OS_PRIO data type) */

/*系统调度关断时间测量*/
#define OS_CFG_SCHED_LOCK_TIME_MEAS_EN  DEF_DISABLED            /* Include (DEF_ENABLED) code to measure scheduler lock time             */

/*同级优先级调度*/
#define OS_CFG_SCHED_ROUND_ROBIN_EN     DEF_DISABLED            /* Include (DEF_ENABLED) code for Round-Robin scheduling                 */

/*最小堆栈大小*/
#define OS_CFG_STK_SIZE_MIN             128u                     /* Minimum allowable task stack size                                     */


/*标志位*/
#define OS_CFG_FLAG_EN                  DEF_ENABLED             /* Enable (DEF_ENABLED) code generation for EVENT FLAGS                  */
#define OS_CFG_FLAG_DEL_EN              DEF_ENABLED             /*     Include (DEF_ENABLED) code for OSFlagDel()                        */
#define OS_CFG_FLAG_MODE_CLR_EN         DEF_ENABLED             /*     Include (DEF_ENABLED) code for Wait on Clear EVENT FLAGS          */
#define OS_CFG_FLAG_PEND_ABORT_EN       DEF_ENABLED             /*     Include (DEF_ENABLED) code for OSFlagPendAbort()                  */


/*内存管理*/
#define OS_CFG_MEM_EN                   DEF_ENABLED             /* Enable (DEF_ENABLED) code generation for the MEMORY MANAGER           */


/*互斥信号量*/
#define OS_CFG_MUTEX_EN                 DEF_ENABLED             /* Enable (DEF_ENABLED) code generation for MUTEX                        */
#define OS_CFG_MUTEX_DEL_EN             DEF_DISABLED             /*     Include (DEF_ENABLED) code for OSMutexDel()                       */
#define OS_CFG_MUTEX_PEND_ABORT_EN      DEF_DISABLED             /*     Include (DEF_ENABLED) code for OSMutexPendAbort()                 */


/*消息队列*/
#define OS_CFG_Q_EN                     DEF_ENABLED             /* Enable (DEF_ENABLED) code generation for QUEUES                       */
#define OS_CFG_Q_DEL_EN                 DEF_DISABLED             /*     Include (DEF_ENABLED) code for OSQDel()                           */
#define OS_CFG_Q_FLUSH_EN               DEF_DISABLED             /*     Include (DEF_ENABLED) code for OSQFlush()                         */
#define OS_CFG_Q_PEND_ABORT_EN          DEF_DISABLED             /*     Include (DEF_ENABLED) code for OSQPendAbort()                     */


/*信号量*/
#define OS_CFG_SEM_EN                   DEF_ENABLED             /* Enable (DEF_ENABLED) code generation for SEMAPHORES                   */
#define OS_CFG_SEM_DEL_EN               DEF_DISABLED             /*     Include (DEF_ENABLED) code for OSSemDel()                         */
#define OS_CFG_SEM_PEND_ABORT_EN        DEF_DISABLED             /*     Include (DEF_ENABLED) code for OSSemPendAbort()                   */
#define OS_CFG_SEM_SET_EN               DEF_ENABLED             /*     Include (DEF_ENABLED) code for OSSemSet()                         */


/*系统统计任务*/
#define OS_CFG_STAT_TASK_EN             DEF_ENABLED             /* Enable (DEF_ENABLED) the statistics task                              */
#define OS_CFG_STAT_TASK_STK_CHK_EN     DEF_ENABLED              /*     Check task stacks (DEF_ENABLED) from the statistic task           */

#define OS_CFG_TASK_CHANGE_PRIO_EN      DEF_DISABLED             /* Include (DEF_ENABLED) code for OSTaskChangePrio()                     */
#define OS_CFG_TASK_DEL_EN              DEF_ENABLED             /* Include (DEF_ENABLED) code for OSTaskDel()                            */
#define OS_CFG_TASK_IDLE_EN             DEF_ENABLED             /* Include (DEF_ENABLED) the idle task                                   */
#define OS_CFG_TASK_PROFILE_EN          DEF_ENABLED             /* Include (DEF_ENABLED) variables in OS_TCB for profiling               */
#define OS_CFG_TASK_Q_EN                DEF_ENABLED             /* Include (DEF_ENABLED) code for OSTaskQXXXX()                          */
#define OS_CFG_TASK_Q_PEND_ABORT_EN     DEF_ENABLED             /* Include (DEF_ENABLED) code for OSTaskQPendAbort()                     */
#define OS_CFG_TASK_REG_TBL_SIZE        1u                      /* Number of task specific registers                                     */
#define OS_CFG_TASK_STK_REDZONE_EN      DEF_DISABLED            /* Enable (DEF_ENABLED) stack redzone                                    */
#define OS_CFG_TASK_STK_REDZONE_DEPTH   8u                      /* Depth of the stack redzone                                            */
#define OS_CFG_TASK_SEM_PEND_ABORT_EN   DEF_DISABLED             /* Include (DEF_ENABLED) code for OSTaskSemPendAbort()                   */
#define OS_CFG_TASK_SUSPEND_EN          DEF_ENABLED             /* Include (DEF_ENABLED) code for OSTaskSuspend() and OSTaskResume()     */


/*任务寄存器*/
#define OS_CFG_TLS_TBL_SIZE             0u                      /* Include (DEF_ENABLED) code for Task Local Storage (TLS) registers     */


/*系统时间管理*/
#define OS_CFG_TIME_DLY_HMSM_EN         DEF_ENABLED             /* Include (DEF_ENABLED) code for OSTimeDlyHMSM()                        */
#define OS_CFG_TIME_DLY_RESUME_EN       DEF_ENABLED             /* Include (DEF_ENABLED) code for OSTimeDlyResume()                      */


/*系统定时任务*/
#define OS_CFG_TMR_EN                   DEF_ENABLED             /* Enable (DEF_ENABLED) code generation for TIMERS                       */
#define OS_CFG_TMR_DEL_EN               DEF_ENABLED             /* Enable (DEF_ENABLED) code generation for OSTmrDel()                   */


/*使能系统状态追踪*/
#define OS_CFG_TRACE_EN                 DEF_DISABLED            /* Enable (DEF_ENABLED) uC/OS-III Trace instrumentation                  */
#define OS_CFG_TRACE_API_ENTER_EN       DEF_DISABLED            /* Enable (DEF_ENABLED) uC/OS-III Trace API enter instrumentation        */
#define OS_CFG_TRACE_API_EXIT_EN        DEF_DISABLED            /* Enable (DEF_ENABLED) uC/OS-III Trace API exit  instrumentation        */

#endif
