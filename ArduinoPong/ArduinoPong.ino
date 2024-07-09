#include <UTFT.h>

#include "joystick.cpp"

// SPI pins Nano

#define TFT_RST  41 // Reset line for TFT (or connect to +5V)
#define TFT_CS  40  // Chip select line for TFT display
#define TFT_WR   39  // Write/Read
#define TFT_RS  38  // RS 

//TFT resolution 320*240
#define MIN_X 0
#define MIN_Y 0
#define MAX_X 240
#define MAX_Y 320

#define PADDLESIZE  22  //size of the paddle: 32 EASY  16 HARD
#define BALLSIZE     4  //radius size of the ball
#define PADDLEREGION 3  //pixels around paddle that still hit ball

#define slowdownspeed 5 //game too fast, increase this to slow it down. A delay in ms

extern uint8_t SmallFont[];

UTFT myGLCD(ILI9325D_8,TFT_RS,TFT_WR,TFT_CS,TFT_RST);  

//obtain a joystick
Joystick js;
  
float BPX, BPX_OLD;  // Ball Position X
float BPY, BPY_OLD;  // Ball Position Y
int byx; // Ball position in y after receiving input from A
int bx; // Ball direction in x
int A;      // Ball Direction in y (1 = left to right, -1 = right to left)
bool oneplayer = true;
int playerScore;    // These variables hold the player & computer score.
int computerScore;
int sendScore;
#define WINSCORE 5  // When you reach this value you win - suggest you keep <=9 for display alignment, increase > 9 if you tweek

int R, L, D, U;     // These values adjust the paddles L & R from player buttons, U & D from computer algorithm
int playerPaddle;   // Player Paddle Position
int computerPaddle; // Computer's Paddle Position
int Random;

void DrawCourt(boolean onlycenter) // Draw the play court lines (pass 1 to onlu have the center line drawn for speed)
{
  if(!onlycenter) {
       myGLCD.setColor(0, 255, 255);
       myGLCD.drawHLine(0,0,319);
       myGLCD.drawHLine(0,239,319);
   }
  myGLCD.drawVLine(160, 0, 239);  // Center Line 
}

void DisplayScore(int playerScore, int computerScore)  // display score with numbers.  Tested for WINSCORE <= 9 (ot greater # digits than 1)
{ 
  myGLCD.setColor(0, 255, 255);
  myGLCD.setBackColor(VGA_TRANSPARENT);
  myGLCD.print( String(playerScore) + " " + String(computerScore), CENTER, 5); //String myString = String(n);
}

void EraseScore(int playerScore, int computerScore){
  myGLCD.setColor(255, 0, 0);
  myGLCD.setBackColor(VGA_TRANSPARENT);
  myGLCD.print( String(playerScore) + " " + String(computerScore), CENTER, 5); //String myString = String(n);
}

void erasePaddle() {
  myGLCD.setColor(255, 0, 0);
  myGLCD.drawVLine(3, playerPaddle-1, PADDLESIZE+2);
  myGLCD.drawVLine(4, playerPaddle-1, PADDLESIZE+2);
  myGLCD.drawVLine(5, playerPaddle-1, PADDLESIZE+2);
  myGLCD.drawVLine(6, playerPaddle-1, PADDLESIZE+2);

  myGLCD.drawVLine(314, computerPaddle-1, PADDLESIZE+2);
  myGLCD.drawVLine(315, computerPaddle-1, PADDLESIZE+2);
  myGLCD.drawVLine(316, computerPaddle-1, PADDLESIZE+2);
  myGLCD.drawVLine(317, computerPaddle-1, PADDLESIZE+2);
}

void drawPaddle() {
  
  //tft.fillRect(playerPaddle-1, 3, 2, 3, ST7735_BLACK);  //These two parts are so the paddle erase themselves as they move, they could have been black pixels but i just used 
  myGLCD.setColor(255, 0, 0);
  myGLCD.drawHLine(3, playerPaddle-1, 3);
  myGLCD.drawHLine(3, playerPaddle, 3);
  //tft.fillRect(playerPaddle+33, 3, 2, 3, ST7735_BLACK);
  myGLCD.drawHLine(3, playerPaddle+PADDLESIZE+1, 3);
  myGLCD.drawHLine(3, playerPaddle+PADDLESIZE+2, 3);
  myGLCD.setColor(0, 255, 255);
  myGLCD.fillRect(3, playerPaddle, 6, playerPaddle + PADDLESIZE);

  if (playerPaddle==1)  // This is so the paddle does not move off the screen on the bottom side of the screen
    playerPaddle=2;
  if (playerPaddle==MAX_X-PADDLESIZE-1) // This is so the paddle does not move off the screen on the top side of the screen
    playerPaddle=MAX_X-PADDLESIZE-2;

  myGLCD.fillRect(314, computerPaddle, 317, computerPaddle + PADDLESIZE);
  myGLCD.setColor(255, 0, 0);
  myGLCD.drawHLine(314, computerPaddle-1, 3);
  myGLCD.drawHLine(314, computerPaddle, 3);
  //tft.fillRect(computerPaddle-1, 154, 2, 3, ST7735_BLACK);
  //tft.fillRect(computerPaddle+33, 154, 2, 3, ST7735_BLACK);
  myGLCD.drawHLine(314, computerPaddle+PADDLESIZE+1, 3);
  myGLCD.drawHLine(314, computerPaddle+PADDLESIZE+2, 3);

  if (computerPaddle==1)
    computerPaddle=2;
  if (computerPaddle==MAX_X-PADDLESIZE-1)
    (computerPaddle=MAX_X-PADDLESIZE-2);
}

void setup(){
  Serial.begin(115200);
  
  myGLCD.InitLCD();
  myGLCD.setFont(SmallFont);
  
  myGLCD.fillScr(0, 0, 0);  // clear display
  
  js.init();
  
  // Draw splash screen text
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("PONG", CENTER, 5);
  myGLCD.print("Press Joy: single player", CENTER, 140);
  myGLCD.print("Press A  : 2 player mode", CENTER, 160);
  
  
  while( ! Joystick::JoystickPressed() && ! js.A());    // New game when swtich 4 is pressed
  if (Joystick::JoystickPressed()) 
    oneplayer = true;
  else
    oneplayer = false;

  
  myGLCD.fillScr(255, 0, 0);  // clear screen again
  DrawCourt(0);   // Draw court lines
  
  playerScore=0;
  computerScore=0;
  DisplayScore(playerScore, computerScore);
  BPX = 160;
  BPY = 120; 
  byx=120;
  bx=1;
  A=1;
  playerPaddle=MAX_X/2-PADDLESIZE/2;    // set paddles in the center of the display TODO
  computerPaddle=MAX_X/2-PADDLESIZE/2;
  
  randomSeed(analogRead(2));
}


void loop() {
  
  if (oneplayer) {
    if ((BPX==200) || (BPX==150) || (BPX==100) || (BPX==50)){    //This is how i made the computer/the probability of the computer making an error
      Random = random(1, 10);
    }
    if (Random<=7){
    
      if (( bx == 1)||((BPX > 100) && ( bx == -1))) {
        if ((A == -1) && (BPY < (computerPaddle+PADDLESIZE/2))) {
          U = 1; 
          D = 0;
        }    //Computer simulation
        if ((A == 1) && (BPY > (computerPaddle+PADDLESIZE/2))) {
          D = 1; 
          U = 0;
        }
      }
      else {
        D = 0; 
        U = 0;
      }
    }
   
   if ((Random>7) && (Random<=9)){ 
    if (( bx == 1)||((BPX > 100) && ( bx == -1))) {
      if ((A == -1) && (BPY < (computerPaddle+PADDLESIZE/2))) {
        U = 0; 
        D = 1;
      }    //Computer simulation
      if ((A == 1) && (BPY > (computerPaddle+PADDLESIZE/2))) {
        D = 0; 
        U = 1;
      }
    }
    else {
      D = 0; 
      U = 0;
    }
   }
    
   if (Random>9){
    
     if (( A == 1)||((BPY > 100) && ( A == -1))) {
      if ((bx == -1) && (BPX < (computerPaddle-PADDLESIZE/2))) {
        U = 1; 
        D = 0;
      }    //Computer simulation
      if ((bx == 1) && (BPX > (computerPaddle-PADDLESIZE/2))) {
        D = 1; 
        U = 0;
      }
     }
     else {
      D = 0; 
      U = 0;
     } 
   }
  } else {
    // twoplayer mode, bottom player with Joystick
    int side = js.getX();
    if (side > 0) {U=0; D=1;}
    else if (side < 0) {U=1; D=0;}
    else {U=0; D=0;}
  }
  
 DrawCourt(0);  // Draw court line(s)
 DisplayScore(playerScore, computerScore);
// see if player is using A & B buttons to signal they wish t move the player paddle
  R = js.B(); 
  L = js.A(); 

  playerPaddle=playerPaddle+R; //These equations are for the movement of the paddle, R, L, D, and U are all boolean.  paddles initially equal 48. This is so
                //at startup the paddles are at center.  
  playerPaddle=playerPaddle-L;
  computerPaddle=computerPaddle+D;  //I used D and U because i use the buttons for other applications but they can be defined as player2 R and L
  computerPaddle=computerPaddle-U;

  drawPaddle();
 
  byx=(A+byx);
  BPX_OLD = BPX;
  BPY_OLD = BPY;
  BPY=((byx));
  BPX=(bx+BPX);
  //choque con pared
  if ((BPY == 236)||(BPY == 2)){
    (A=(-1*A));
  }
  else {
  };
  
  //bounce ball back if we hit it with paddle
  if ((BPY<=(computerPaddle+PADDLESIZE+BALLSIZE+PADDLEREGION)) 
      && (BPY>=(computerPaddle-BALLSIZE-PADDLEREGION)) && (BPX == 314)){
    (bx=(-1*bx));
  }
  else{
  };
  if ((BPY<=(playerPaddle+PADDLESIZE+BALLSIZE+PADDLEREGION)
      && (BPY>=(playerPaddle-BALLSIZE-PADDLEREGION)) && (BPX==6))){
    (bx=(-1*bx));
  }
  else{
  };
  delay(slowdownspeed);
  
  if (BPX >= MAX_Y - 2 || BPX <= 2){    // someone scored!
    EraseScore(playerScore, computerScore);
    if (BPX >= MAX_Y - 2) 
       playerScore = playerScore+1; 
    else 
       computerScore = computerScore+1;
    // Reset:                                   // reset court after score
    DisplayScore(playerScore, computerScore);
    
    DrawCourt(0);
    myGLCD.setColor(255, 0, 0);
    myGLCD.fillCircle(BPX, BPY, 7);  // erase ball in last known location
    BPX=160;                                     // set ball to center of screen
    BPY=120;

    //center the paddles
    erasePaddle();
    playerPaddle=MAX_X/2-PADDLESIZE/2;    // set paddles in the center of the display
    computerPaddle=MAX_X/2-PADDLESIZE/2;
    drawPaddle();

    myGLCD.setColor(0, 255, 255);
    myGLCD.fillCircle(BPX, BPY, 4);  // draw ball  in center
    
    delay(3000);                                // delay 3 seconds
    
    myGLCD.setColor(255, 0, 0);
    myGLCD.fillCircle(BPX, BPY, 7);
    byx=120; 
  }

 //we allow score to disappear as rotating screen + drawing is SLOW !!
 //DisplayScore(playerScore, computerScore);

 if(playerScore==WINSCORE || computerScore==WINSCORE) {  // if someone hit the winning score then game over - print who one and reset game
     myGLCD.fillScr(255, 0, 0);
     myGLCD.setColor(255, 255, 255);
  
     if (playerScore==WINSCORE){  // player wins
       myGLCD.print("PLAYER 1 WINS", CENTER, 20);
       sendScore = playerScore * 100 - computerScore * 50;
     }
     else {
      if (oneplayer){
       myGLCD.print("COMPUTER WINS", CENTER, 20);   // computer wins
       sendScore = 50 + playerScore * 50;
      }
      else{
       myGLCD.print("PLAYER 2 WINS", CENTER, 20);   // player 2 wins
       sendScore = computerScore * 100 - playerScore * 50;
      }
     }
     Serial.print(sendScore);
     Serial.print("_");
    //NEWGAME:           //Resets the screen for a new game

    //center the paddles
    playerPaddle=MAX_X/2-PADDLESIZE/2;    // set paddles in the center of the display
    computerPaddle=MAX_X/2-PADDLESIZE/2;
    
    // Draw splash screen text
    myGLCD.setColor(255, 255, 255);
    myGLCD.print("PONG", CENTER, 5);
    myGLCD.print("Press Joy: single player", CENTER, 140);
    myGLCD.print("Press A  : 2 player mode", CENTER, 160);
    while( !(Joystick::JoystickPressed()) && !(js.A()));     // New game when joystick is pressed
    if (Joystick::JoystickPressed()) 
      oneplayer = true;
    else
      oneplayer = false;
    myGLCD.fillScr(255, 0, 0);
    myGLCD.setColor(255, 0, 0);
    myGLCD.fillCircle(BPX, BPY, 7);
    BPX = 160;
    BPY = 120; 
    byx=120;
    bx=1;
    A=1;
    myGLCD.setColor(0, 255, 255); 
    myGLCD.fillCircle(BPX, BPY, 4);
    myGLCD.setColor(255, 0, 0);
    myGLCD.fillCircle(BPX, BPY, 7);
    myGLCD.setColor(0, 255, 255);
    myGLCD.fillCircle(BPX, BPY, 4);
    myGLCD.setColor(255, 0, 0);
    myGLCD.fillCircle(BPX, BPY, 7);
    computerScore=0;
    playerScore=0;
    DrawCourt(0);
    DisplayScore(playerScore, computerScore);

    delay(2000);     // wait 4 seconds to start new game
  }
//If you want, uncomment to use the new scoring method/output using bars
////////////////////////////////////////////// 
// DisplayScoreTicks(playerScore, computerScore);
////////////////////////////////////////////

  myGLCD.setColor(255, 0, 0);
  myGLCD.fillCircle(BPX_OLD, BPY_OLD, BALLSIZE);  //Erase ball 
                                           // if little green pixels start to light up due to the ball, change the 6 to a 7
  myGLCD.setColor(0, 255, 255);
  myGLCD.fillCircle(BPX, BPY, BALLSIZE);  //This is the actual ball

}
