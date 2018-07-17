#include <8051.h>
#include "..\ev8031.lib\bitdef.h"
#include "..\ev8031.lib\ev8031.c"
//
// signal levels
//
#define SYNC_LEVEL	255
#define ZERO_LEVEL	130
#define ONE_PLUS_LEVEL  150
#define TWO_PLUS_LEVEL  230
#define ONE_MINUS_LEVEL 110
#define TWO_MINUS_LEVEL 25
#define GROUND_LEVEL    0
//
// signal durations
//
#define SYNC_DURATION	20
#define BIT_DURATION 	10
//
// signal sequence parameters
//
#define SEQUENCE_LEN    32
//
// keyboard scan codes and usefull constants 
//
#define KEY_SCAN_DELAY  10000
#define ENTER_KEY       3 //*
#define CONTINUE_KEY    11 //#
#define KEY_1		0
#define KEY_2		4
#define KEY_3           8
#define KEY_4           1
#define KEY_5           5
#define KEY_6           9
#define KEY_7           2
#define KEY_8           6
#define KEY_9           10
#define KEY_0           7
#define KEY_STAR        3
#define KEY_SHARP       11

//
//signal data
//


xdata static unsigned char inp_sequence[SEQUENCE_LEN] = {'0','1','0','1','0','1','0','1',
                                                         '0','1','0','1','0','1','0','1',
                                                         '0','1','0','1','0','1','0','1',
                                                         '0','1','0','1','0','1','0','1'};

xdata static unsigned int  line_sequence[SEQUENCE_LEN*2] ;

//
// Signal output routines
//



void out_sync_pulse()
{
  pDAC = SYNC_LEVEL;
  delay16(SYNC_DURATION);
  pDAC = GROUND_LEVEL;
  delay16(SYNC_DURATION);
  pDAC = ZERO_LEVEL;
  delay16(SYNC_DURATION);

}

void out_pulse(unsigned int p_level)
{
 pDAC=p_level;
 delay16(BIT_DURATION);
}


//
// LCD routines
//


void char_to_hd(unsigned char x, unsigned char y, char *string)
{
  set_cursor(x,y);
  y = *string;
    LCD_DATA = y;
}



void array_tohd()
{
  unsigned int i;

  for(i=0;i<SEQUENCE_LEN;i++)
  {
    char_to_hd( i%8, i/8, &inp_sequence[i]);
  }
}



#define SHIFT_CMD      _00010000
#define DISPLAY_SHIFT  _00001000
#define CURSOR_SHIFT   _00000000
#define LEFT_SHIFT     _00000000
#define RIGHT_SHIFT    _00000100

#define DISPLAY_CTRL   _00001000
#define DISPLAY_ON     _00000100
#define CURSOR_ON      _00000010
#define BLINKING_ON    _00000001

#define DISPLAY_FORMAT _00100000
#define DATA_8_BITS    _00010000
#define DATA_4_BITS    _00000000
#define LINES_2        _00001000
#define LINES_1        _00000000
#define FONT_5X10      _00000100
#define FONT_5X8       _00000000

#define CLEAR_CMD      _00000001

void prepare_lcd()
{
  LCD_CMD = (DISPLAY_CTRL|DISPLAY_ON|CURSOR_ON|BLINKING_ON); // display on, cursor on , blinking on
  delay16(300);
  LCD_CMD = (DISPLAY_FORMAT|DATA_8_BITS|LINES_2|FONT_5X10); // 8 bit x 2 lines , 5x10 font	
  delay16(300);
  LCD_CMD = (CLEAR_CMD);	
  delay16(300);
}


void display_welcome_screen()
{
  LCD_CMD = (CLEAR_CMD);	
  delay16(300);
  string_to_hd(0,0,"AMI CODE");
  string_to_hd(0,1,"GENERATOR");
  string_to_hd(0,2,"press * to");
  string_to_hd(0,3,"input");
} 

void enter_data()
{
  unsigned int pressed_coninue_key=0;
  char pressed_key;
  unsigned int position=0;
  
  // prepare LCD
  prepare_lcd();
  set_cursor(0,0);
  array_tohd();
  set_cursor(0,0);
  
  while (!pressed_coninue_key)
  {
  	// keyboard scanning
  	
  	do{
  	  pressed_key=kb_scan();
  	  if (pressed_key!=12) righti = pressed_key;
  	  }
  	while(pressed_key==12);
        
        // reaction on pressed keys
        
  	switch(pressed_key)
  	{
  	  case KEY_0: // 0 pressed - save into memory and display at LCD
  	           inp_sequence[position]='0';
                   char_to_hd(position % 8,position / 8,&inp_sequence[position]);
                   if (position<SEQUENCE_LEN) position+=1;
                   break;
  	  case KEY_1:// 1 pressed - save into memory and display at LCD
  	    	   inp_sequence[position]='1';
                   char_to_hd(position % 8,position / 8,&inp_sequence[position]);
                   if (position<SEQUENCE_LEN) position+=1;
                   break;
  	           
  	  case KEY_4: // 4 pressed - shift cursor left <-
  	           if (position!=0) position-=1;
  	           set_cursor(position % 8,position / 8);  	  
  	           break;
  	  case KEY_6: // 6 pressed  -shift cursor right ->
  	           if (position<SEQUENCE_LEN) position+=1;
  	           set_cursor(position % 8,position / 8 );  	  
  	           break;
  	  case KEY_SHARP:// # pressed - end input procedure 
  	           pressed_coninue_key=1;
  	           break;
  	  default: // any key pressed - nothing to do
  	           break;
  	  
  	}
  	delay16(KEY_SCAN_DELAY);  
  }
  
  display_welcome_screen();
}

void encode_ami()
{
 unsigned int i ;             // counter
 unsigned int y = ONE_MINUS_LEVEL; // temporary value
 unsigned int z = ONE_MINUS_LEVEL;
 
 
 for (i=0;i<SEQUENCE_LEN;i++)
 {
   switch (inp_sequence[i])
   {               
    case '1': 
               if (z=ONE_PLUS_LEVEL) 
                   {
                     y=ONE_MINUS_LEVEL;
                     z=ONE_MINUS_LEVEL;
                   }
               else 
                  {
                     y=ONE_PLUS_LEVEL;
                     z=ONE_PLUS_LEVEL;
                   };
               break;              
    case '0':
               y=ZERO_LEVEL;
               break;             
   }
   line_sequence[2*i]=y;
   line_sequence[2*i+1]=ZERO_LEVEL;
    
 }

}

int main()
{
  unsigned int i; //cycle counter

  // initialize leds
  
  lefti=0xcc;
  righti=0xcc;
  new_dotsi=0;
  
  // initialize LCD
  
  hd_init();
  hd_turn_on();
  prepare_lcd();
  display_welcome_screen();
  encode_ami();
  // working cycle
  
  while(1)
  {
  // check keyboard 
  
  if ( key_scan()== ENTER_KEY )
  
      // key pressed 
      {
         // enter input code combination
         enter_data();         
         // encode inut data
         encode_ami();
     }
     else
     // out data
     {  
  	// out sync pulse
	out_sync_pulse();
	// cycle foy out line code
	for (i=0;i<SEQUENCE_LEN*2;i++)
	{	
	   out_pulse(line_sequence[i]) ; 
  	  
  	}
     }
  }

}
        