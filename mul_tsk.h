/****************************************************************************/
/* マルチタスク実行                                                         */
/*                                                                          */
/* designed by hamayan                                                      */
/*                         Copyright (C) 2012 hamayan All Rights Reserved.  */
/****************************************************************************/
#ifndef _MULTITASK_H_
#define _MULTITASK_H_

/****************************************************************************/
/* サービスコールデータ型                                                   */
/****************************************************************************/
typedef  int            ID;
typedef  int            ER;
typedef  unsigned char  STAT;
typedef  unsigned long long  SYSTIM;
//typedef  unsigned long  SYSTIM;
typedef  unsigned long  RELTIM;
typedef  unsigned long  TMO;

typedef  long           VP_INT;  /* VP または INT */
typedef struct
{
  void *sp;
  VP_INT exinf[4];
  unsigned char rdy_flg;
} TASK_CONTROL_BLOCK;

enum TSK_ID {TASK1,TASK2,TASK3,TASK4,TASK5,TASK6,TASK7,TASK8,TASK9,TASK10,};

/****************************************************************************/
/* セマフォ管理                                                             */
/****************************************************************************/
typedef struct
{
  unsigned int sigCount;  /*sig_semでカウントアップ*/
  unsigned int waiCount;  /*wai_semでカウントアップ*/
} SEM_OBJECT;

/****************************************************************************/
/* パケット形式                                                             */
/****************************************************************************/
typedef  struct t_rtst{ STAT tskstat; STAT tskwait; } T_RTST;

/****************************************************************************/
/* 定数とマクロ                                                             */
/****************************************************************************/
#define  E_OK        0
#define  TMO_POL     0
#define  TMO_FEVR    (-1)
#define  TMO_NBLK    (-2)

#define  TTS_RUN     0x01
#define  TTS_RDY     0x02
#define  TTS_WAI     0x04
#define  TTS_SUS     0x08
#define  TTS_WAS     0x0C
#define  TTS_DMT     0x10

#define  TSK_SELF    0
#define  TSK_NONE    0

/****************************************************************************/
/* エラーコード                                                             */
/****************************************************************************/
#define  E_SYS    (-5)
#define  E_NOSPT  (-9)
#define  E_RSFN   (-10)
#define  E_RSATR  (-11)
#define  E_PAR    (-17)
#define  E_ID     (-18)
#define  E_CTX    (-25)
#define  E_MACV   (-26)
#define  E_OACV   (-27)
#define  E_ILUSE  (-28)
#define  E_NOMEM  (-33)
#define  E_NOID   (-34)
#define  E_OBJ    (-41)
#define  E_NOEXS  (-42)
#define  E_QOVR   (-43)
#define  E_RLWAI  (-49)
#define  E_TMOUT  (-50)
#define  E_DLT    (-51)
#define  E_CLS    (-52)
#define  E_WBLK   (-57)
#define  E_BOVR   (-58)

/****************************************************************************/
/* サービスコール                                                           */
/****************************************************************************/
//ER   reg_tsk( ID tid, void (*task)(void), void *stk, unsigned int sz );
ER reg_tsk( ID tid, void *tsk, void *stk, int stk_sz, VP_INT exinf1, VP_INT exinf2, VP_INT exinf3, VP_INT exinf4 );
ER   sta_tsk( ID tid );
void ext_tsk( void );
ER   sus_tsk( ID tid );
ER   rsm_tsk( ID tid );
ER   dly_tsk( RELTIM dly );
ER   rot_rdq( void );
ER   sta_rdq( ID tid );
ER   ref_tst( ID tid, T_RTST *pk_rtst );
ER   rel_wai( ID tid );
ER   get_par( VP_INT *exinf, int size );
ER   dis_dsp( void );
ER   ena_dsp( void );
ER   sig_sem( ID semid );
ER   wai_sem( ID semid );

#define  loc_cpu()  __asm__("cli\n\t")
#define  unl_cpu()  __asm__("sei\n\t")

/****************************************************************************/
/* ディスパッチャー                                                         */
/****************************************************************************/
void sta_ctx( void *exe );             /*コンテキストをロードし、タスク実行*/
void swi_ctx( void *pre, void *post);  /*コンテキストの切り替え*/

#endif  /*_MULTITASK_H_*/
