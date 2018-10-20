/**********************************************************/
/* マルチタスクやってみる                                 */
/**********************************************************/
#include <MsTimer2.h>
#include "delivertive.h"

extern "C"
{
  #include "mul_tsk.h"
}

/**********************************************************/
/* prototypes                                             */
/**********************************************************/
void tsk_ini( void );
void stackMonitor( void );
void task1( void );
void task2( void );

/**********************************************************/
/* valiables                                              */
/**********************************************************/
SYSTIM systim;  // 1msでインクリメントする変数
uint8_t tsk0_stk[ 128 * 1 ];
uint8_t tsk1_stk[ 128 * 1 ];
uint8_t tsk2_stk[ 128 * 1 ];

/**********************************************************/
/* system timer update                                    */
/**********************************************************/
void systemTimerUpdate()
{
  systim += 10UL;
}

/**********************************************************/
/* setup                                                  */
/**********************************************************/
void setup()
{
  Serial.begin( 115200UL );
  Serial.println( "AVR Multi Task Demo." );
  delay( 100UL );
  MsTimer2::set( 10, systemTimerUpdate ); // 500ms period
  MsTimer2::start();

  tsk_ini();  //タスクの初期化
  sta_rdq( ID_monitor );  //ラウンドロビン開始。ここからタスクが開始される

  /* Infinite loop */
  while(1) {}
}

/**********************************************************/
/* loop ※使われない                                      */
/**********************************************************/
void loop()
{
}

/**********************************************************/
/* タスク初期化                                           */
/**********************************************************/
void tsk_ini( void )
{
  reg_tsk( ID_task1, (void *)task1, (void *)tsk1_stk, sizeof(tsk1_stk), 0,0,0,0 );
  reg_tsk( ID_task2, (void *)task2, (void *)tsk2_stk, sizeof(tsk2_stk), 0,0,0,0 );
  reg_tsk( ID_monitor, (void *)stackMonitor, (void *)tsk0_stk, sizeof(tsk0_stk), 0,0,0,0 );

  sta_tsk( ID_task1 );
  sta_tsk( ID_task2 );
  sta_tsk( ID_monitor );
}

/**********************************************************/
/* stack monitor task                                     */
/**********************************************************/
static unsigned int RemainStack( void *stk, unsigned int sz );
static void stackPrint( const char *msg, void *stk, unsigned int sz );

void stackMonitor( void )
{
  while( 1 )
  {
    dly_tsk( 10 * 1000UL );
    stackPrint( "task1 stack : ", tsk1_stk, sizeof(tsk1_stk) );
    stackPrint( "task2 stack : ", tsk2_stk, sizeof(tsk2_stk) );
    stackPrint( "monitor stack : ", tsk0_stk, sizeof(tsk0_stk) );
  }
}

static void stackPrint( const char *msg, void *stk, unsigned int sz )
{
  Serial.print( msg );
  Serial.print( RemainStack( stk, sz ), DEC );
  Serial.print( "/" );
  Serial.println( sz, DEC );
}

static unsigned int RemainStack( void *stk, unsigned int sz )
{
  unsigned int i;
  char *ptr = (char *)stk;

  for( i = 0; i < sz; i++ )
  {
    if( *ptr++ != 0 ) break;
  }

  return sz - i;
//  return i;
}

/**********************************************************/
/* task1                                                  */
/**********************************************************/
void task1( void )
{
  while( 1 )
  {
    Serial.println( "this is task1." );
    dly_tsk( 500UL );
  }
}

/**********************************************************/
/* task2                                                  */
/**********************************************************/
void task2( void )
{
  while( 1 )
  {
    Serial.println( "this is task2." );
    dly_tsk( 777UL );
  }
}

