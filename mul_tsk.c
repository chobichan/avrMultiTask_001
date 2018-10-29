/****************************************************************************/
/* マルチタスク実行                                                         */
/*                                                                          */
/* designed by hamayan                                                      */
/*                         Copyright (C) 2012 hamayan All Rights Reserved.  */
/****************************************************************************/
#include "mul_tsk.h"  //必要
#include "delivertive.h"

/****************************************************************************/
/* 大域変数                                                                 */
/****************************************************************************/
extern SYSTIM systim;  //1msでインクリメントする変数
ID cur_tid;  //現行タスクID
unsigned char disable_dispatch = 0;  //
unsigned char notStartTaskContext = 1;
//static unsigned char disable_dispatch = 0;  //
//static unsigned char notStartTaskContext = 1;

/****************************************************************************/
/* サービスコール                                                           */
/****************************************************************************/

/****************************************************************************/
/* タスクコンテキスト                                                       */
/****************************************************************************/
TASK_CONTROL_BLOCK tcb[MAX_TASK_NUMBER];   //タスクの環境変数(コンテキスト)を保存する領域

/****************************************************************************/
/* タスク登録                                                               */
/* タスクID、エントリーポイント、スタック領域、スタックサイズ、             */
/* 起動時引数を4つまで登録できる。                           、             */
/****************************************************************************/
ER reg_tsk( ID tid, void *tsk, void *stk, int stk_sz,
  VP_INT exinf1, VP_INT exinf2, VP_INT exinf3, VP_INT exinf4 )
{
  if( tid < 0 || tid >= MAX_TASK_NUMBER ) return E_ID;

  /*タスクのスタックの初期化*/
  unsigned char *ptr = (unsigned char *)((unsigned char *)stk + stk_sz - (18 + 1 + 1 + 2));

  /* r2-r17,r28-r29,SREG,PCまでこの領域に保存される */
  tcb[ tid ].sp = (void *)ptr;
  unsigned char *pc = (unsigned char *)&ptr[20];
  unsigned short exe = (unsigned short)tsk;
  *pc++ = (unsigned char)(exe >> 8);  /*pc upper*/
  *pc = (unsigned char)(exe >> 0);  /*pc lower*/

  tcb[tid].exinf[0] = exinf1;
  tcb[tid].exinf[1] = exinf2;
  tcb[tid].exinf[2] = exinf3;
  tcb[tid].exinf[3] = exinf4;

  tcb[tid].rdy_flg = TTS_DMT;

  return E_OK;
}

/****************************************************************************/
/* タスク開始                                                               */
/* 実際はスケジューラーに登録                                               */
/****************************************************************************/
ER sta_tsk( ID tid )
{
  if( tid < 0 || tid >= MAX_TASK_NUMBER ) return E_ID;

  tcb[tid].rdy_flg = TTS_RDY;  //タスク登録

  return E_OK;
}

/****************************************************************************/
/* ラウンドロビン開始                                                       */
/****************************************************************************/
ER sta_rdq( ID tid )
{
  if( tid < 0 || tid >= MAX_TASK_NUMBER ) return E_ID;
  if( tcb[ tid ].rdy_flg != TTS_RDY ) return E_CTX;
  notStartTaskContext = 0;

  cur_tid = tid;
  sta_ctx( (void *)&tcb[ tid ].sp );

  return E_OK;
}

/****************************************************************************/
/* Rotate Round Que                                                         */
/* タスクQUEを回す                                                          */
/****************************************************************************/
ER rot_rdq( void )
{
  int i;
  ID tid = cur_tid;
  ID old_tid = cur_tid;

  if( disable_dispatch != 0 ) return E_OK;

  for( i = 0; i < MAX_TASK_NUMBER; i++ )
  {
    if( ++tid >= MAX_TASK_NUMBER ) tid = 0;  //次のタスクに切り替え
    if( tcb[tid].rdy_flg & TTS_SUS ) continue;
    loc_cpu();  //一時的に割込みを禁止
    if( tcb[tid].rdy_flg & (TTS_RDY | TTS_WAI) )
    {
      cur_tid = tid;  //コンテキスト切替え後のタスクIDの保存
      swi_ctx( &tcb[old_tid].sp, &tcb[tid].sp );
      unl_cpu();  //割込みを再開
      return E_OK;
    }
    unl_cpu();  //割込みを再開
  }

  return E_PAR;
}

/****************************************************************************/
/* Rotate Round Que                                                         */
/* 割込みの中でタスクQUEを回す                                              */
/****************************************************************************/
void irot_rdq( void )
{
  int i;
  ID tid = cur_tid;
  ID old_tid = cur_tid;

  if( disable_dispatch != 0 ) return;

  for( i = 0; i < MAX_TASK_NUMBER; i++ )
  {
    if( ++tid >= MAX_TASK_NUMBER ) tid = 0;  //次のタスクに切り替え
    if( tcb[tid].rdy_flg & TTS_SUS ) continue;
    if( tcb[tid].rdy_flg & (TTS_RDY | TTS_WAI) )
    {
      cur_tid = tid;  //コンテキスト切替え後のタスクIDの保存
      swi_ctx( &tcb[old_tid].sp, &tcb[tid].sp );
      return;
    }
  }
}

/****************************************************************************/
/* 遅延処理                                                                 */
/* 条件が一致するまでタスクスイッチを継続                                   */
/****************************************************************************/
ER dly_tsk( RELTIM dly )
{
#if 1
  RELTIM lastTim = systim;
  ID tid = cur_tid;

  if(dly == 0UL)
  {
    rot_rdq();
    return E_OK;
  }

  tcb[tid].rdy_flg = TTS_WAI;  /*待ち状態に遷移*/
  for( ;; )
  {
    if( (systim - lastTim) >= dly )  /*時間経過待ち*/
    {
      tcb[tid].rdy_flg &= ~TTS_WAI;  /*待ち状態解除*/
      tcb[tid].rdy_flg |= TTS_RDY;  /*READYに遷移*/
      return E_OK; 
    }
    (void)rot_rdq();  /*ラウンドロビンの実行*/
    if( !(tcb[tid].rdy_flg & TTS_WAI) ) break;  /*待ち状態が解除されている時、ループ終了*/
  }

  tcb[tid].rdy_flg |= TTS_RDY;
  return E_RLWAI;
#else
  RELTIM oldtim = millis();
  ID tid = cur_tid;

  tcb[tid].rdy_flg = TTS_WAI;  /*待ち状態に遷移*/
  for( ;; )
  {
    if((millis() - oldtim) >= dly)  /*時間経過待ち*/
    {
      tcb[tid].rdy_flg &= ~TTS_WAI;  /*待ち状態*/
      tcb[tid].rdy_flg |= TTS_RDY;  /*READYに遷移*/
      return E_OK; 
    }
    (void)rot_rdq();  /*ラウンドロビンの実行*/
    if( !(tcb[tid].rdy_flg & TTS_WAI) ) break;  /*待ち状態が解除されている時、ループ終了*/
  }

  tcb[tid].rdy_flg |= TTS_RDY;
  return E_RLWAI;
#endif
}

/****************************************************************************/
/* 待ち状態の強制解除                                                       */
/* 対象は該当タスク                                                         */
/****************************************************************************/
ER rel_wai( ID tid )
{
  if( tid < 0 || tid >= MAX_TASK_NUMBER ) return E_ID;     /*不正ID*/
  if( tcb[tid].rdy_flg == 0 )             return E_NOEXS;  /*オブジェクト未生成*/
  if( !(tcb[tid].rdy_flg & TTS_WAI) )     return E_OBJ;    /*対象が待ち状態でない*/

  tcb[tid].rdy_flg &= ~TTS_WAI;  /*待ち状態の解除*/

  return E_OK;
}

/****************************************************************************/
/* タスクの状態取得(簡易版)                                                 */
/****************************************************************************/
ER ref_tst( ID tid, T_RTST *pk_rtst )
{
  if( tid < 0 || tid >= MAX_TASK_NUMBER ) return E_NOEXS;

  if(tid == TSK_SELF)
  {
    pk_rtst->tskstat = tcb[cur_tid].rdy_flg;
  }
  else
  {
    pk_rtst->tskstat = tcb[tid].rdy_flg;
  }

  return E_OK;
}

/****************************************************************************/
/* 強制タスク待ち状態に遷移                                                 */
/****************************************************************************/
ER sus_tsk( ID tid )
{
  if( tid < 0 || tid >= MAX_TASK_NUMBER ) return E_ID;
  if( tcb[tid].rdy_flg == 0 ) return E_NOEXS;  //タスク無効

  tcb[tid].rdy_flg |= TTS_SUS;  //タスク無効

  if( tid == cur_tid ) rot_rdq();  //QUEを回す

  return E_OK;
}

/****************************************************************************/
/* 強制待ち状態の強制解除                                                   */
/* 対象は該当タスク                                                         */
/****************************************************************************/
ER rsm_tsk( ID tid )
{
  if( tid < 0 || tid >= MAX_TASK_NUMBER ) return E_ID;     /*不正ID*/
  if( tcb[tid].rdy_flg == 0 )             return E_NOEXS;  /*オブジェクト未生成*/
  if( !(tcb[tid].rdy_flg & (TTS_WAI | TTS_SUS)) ) return E_OBJ;    /*対象が待ち状態でない*/

  tcb[tid].rdy_flg &= ~TTS_SUS;  /*待ち状態の解除*/

  rot_rdq();  //QUEを回す

  return E_OK;
}

/****************************************************************************/
/* 自タスクの終了                                                           */
/* スケジューラーテーブルの状態をDORMANTにする                              */
/****************************************************************************/
void ext_tsk( void )
{
  tcb[cur_tid].rdy_flg = TTS_DMT;  //該当タスクをDORMANTにする

  for( ;; )  //どこかのタスクがRDYではないと、ここでロックしてしまうので注意
  {
    (void)rot_rdq();
  }
}

/****************************************************************************/
/* 起動時引数の引き取り                                                     */
/* exinfは引き取り先、sizeは引き取り数。最大は4個まで。                     */
/****************************************************************************/
ER get_par( VP_INT *exinf, int size )
{
  int i;

  if( size <= 0 || size > 4 ) return E_PAR;

  for( i = 0; i < size; i++ )
  {
    *exinf++ = tcb[cur_tid].exinf[i];
  }
  return E_OK;
}

/****************************************************************************/
/* ディスパッチ禁止                                                         */
/****************************************************************************/
ER dis_dsp( void )
{
  loc_cpu();
  disable_dispatch = 1;  //0以外の時はディスパッチを禁止する
  unl_cpu();
  return E_OK;
}

/****************************************************************************/
/* ディスパッチ許可                                                         */
/****************************************************************************/
ER ena_dsp( void )
{
  loc_cpu();
  disable_dispatch = 0;  //0以外の時はディスパッチを禁止する
  unl_cpu();
  return E_OK;
}

extern volatile SEM_OBJECT sem_obj[];
/****************************************************************************/
/* セマフォ待ち                                                             */
/* 現在のwaiCountを取得後、waiCountのカウントアップを行う。                 */
/* 現在のsigCountと取得したwaiCountが一致するまで待ち状態となる。           */
/* プリエンプティブと言う訳ではないので、loc_cpuは要らない事は要らない。    */
/****************************************************************************/
ER wai_sem( ID semid )
{
  unsigned int presentWaiCount;
  ID tid = cur_tid;

  loc_cpu();
  presentWaiCount = sem_obj[ semid - 1 ].waiCount;
  unl_cpu();

  tcb[tid].rdy_flg = TTS_WAI;  /*待ち状態に遷移*/
  for( ;; )
  {
    //loc_cpu();
    if( presentWaiCount == sem_obj[ semid - 1 ].sigCount )
    {
      tcb[tid].rdy_flg &= ~TTS_WAI;  /*待ち状態の*/
      tcb[tid].rdy_flg |= TTS_RDY;  /*READYに遷移*/
      loc_cpu();
      sem_obj[ semid - 1 ].waiCount = presentWaiCount + 1;
      unl_cpu();
      return E_OK;
    }
    //unl_cpu();
    (void)rot_rdq();  /*ラウンドロビンの実行*/
    if( !(tcb[tid].rdy_flg & TTS_WAI) ) break;  /*待ち状態が解除されている時、ループ終了*/
  }

  return E_RLWAI;
}

/****************************************************************************/
/* セマフォ返却                                                             */
/* 現在のsigCountのカウントアップを行う。                                   */
/* プリエンプティブと言う訳ではないので、loc_cpuは要らない事は要らない。    */
/****************************************************************************/
ER sig_sem( ID semid )
{
  loc_cpu();
  sem_obj[ semid - 1 ].sigCount++;
  //sem_obj[ semid - 1 ].sigCount = sem_obj[ semid - 1 ].waiCount;
  unl_cpu();
  rot_rdq();

  return E_OK;
}
