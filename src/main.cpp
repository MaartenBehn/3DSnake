#include <Arduino.h>

typedef struct vec3 {
    int x, y, z;
} vec3;

#define X_Pins_Size 9 // Amount of X Dimention Pins
int X_Pins[X_Pins_Size] = {4,5,6,7,8,9,12,11,10}; // The pin ids of X Dimention

#define Y_Pins_Size 3 // Amount of Y Dimention Pins
int Y_Pins[Y_Pins_Size] = {A1,A0,A2}; // The pin ids of Y Dimention

#define Button_Pins_Size 7 // Amount of button pins
int Button_Pins[Button_Pins_Size] = {A5,A4,2,A6,3,A7,A3}; // The button pins

#define Board_Size 3 // Size of the bord amount of leds is this ^3
int board[3][3][3]; // The bord. 0 = empti cell, -1 = apple, 1 = head, >1 = body
vec3 head; // position of head
int aktivButton = 0; // 0 x+, 1 x-, 2 y+, 3 y-, 4 z+, 5 z-,
int snakeLenght; // the length of the snake 
bool newApple; // flag if new apple needs to be placed.
bool death; // flag if the player is death and the game can be restarted. Starts with true.

vec3 emptiCells[Board_Size * Board_Size * Board_Size]; // A list of all empti cells for apple placement.
int emptiCellPointer; // A random index of the empti cell list for apple placement.

// blink timer
#define Matrix_Delay 1
#define Max_Head_Counter 50
#define Max_Apple_Counter 100
#define Max_Death_Counter 300
// The bord game as timer version for draw call.
int matrix[3][3][3];
int matrixCounter = 0; // Counter for delay timer.
#define Max_Matrix_Counter 10000

// Rests the game
void newGame(){
  // Clears varibles
  head = {1,1,1};
  snakeLenght = 1;
  aktivButton = 1;
  newApple = 1;

  // Clears board
  for (size_t x = 0; x < Board_Size; x++)
  {
    for (size_t y = 0; y < Board_Size; y++)
    {
      for (size_t z = 0; z < Board_Size; z++)
      {
        board[x][y][z] = 0;
      }
    }
  }
  // Places head
  board[head.x][head.y][head.z] = 1;
  death = 0; // Sets death to 0 so the game is running.
}

// The main game loop
void gameLoop(){

  // Sets aktivButton a varible which says what button was pressed last.
  for (size_t i = 0; i < Button_Pins_Size; i++)
  {
    bool button;
    if(Button_Pins[i] == A6 || Button_Pins[i] == A7){
      button = analogRead(Button_Pins[i]) > 100 ? 1 : 0;
    }else{
      button = digitalRead(Button_Pins[i]);
    }

    if(button)
    {
      aktivButton = i;
    }
  }

  // Button to restart game.
  if(aktivButton = 6){
    newGame();
    return;
  }
  else if(death){ // Stop here if the palyer is death
    return;
  }

  // Move the head in the direction aktivButton says..
  switch (aktivButton){ 
  case 0:
    head.x++;
    if(head.x > Board_Size - 1)
    {
      head.x = 0;
    }
    break;
  case 1:
    head.x--;
    if(head.x < 0)
    {
      head.x = Board_Size - 1;
    }
    break;
  case 2:
    head.y++;
    if(head.y > Board_Size - 1)
    {
      head.y = 0;
    }
    break;
  case 3:
    head.y--;
    if(head.y < 0)
    {
      head.y = Board_Size - 1;
    }
    break;
  case 4:
    head.z++;
    if(head.z > Board_Size - 1)
    {
      head.z = 0;
    }
    break;
  case 5:
    head.z--;
    if(head.z < 0)
    {
      head.z = Board_Size - 1;
    }
    break;
  }
  
  if(board[head.x][head.y][head.z] < 0) // Check if head moven on apple cell.
  { 
    snakeLenght++;
    newApple = 1;
  }
  else if(board[head.x][head.y][head.z] > 0) // Check if head moven on body cell.
  { 
    death = 1;
  }
  
  // Check all Cells and update them
  emptiCellPointer = 0;
  for (size_t x = 0; x < Board_Size; x++)
  {
    for (size_t y = 0; y < Board_Size; y++)
    {
      for (size_t z = 0; z < Board_Size; z++)
      {
        int cell = board[x][y][z];

        if(cell == 0 ) // Its an empti cell.
        { 
          emptiCells[emptiCellPointer] = vec3{x,y,z}; // Save all empti cells for apple placement.
          emptiCellPointer++;
        }
        else if(cell > 0) // Its a body part.
        {
          board[x][y][z]++;
          if (board[x][y][z] > snakeLenght) // Check if body part is older than the snake is long.
          { 
            board[x][y][z] = 0;
          }
        }
        else if(cell < 0 && newApple) // Its the apple.
        {
          board[x][y][z] = 0;
        }
      }
    }
  }
  board[head.x][head.y][head.z] = 1; // Save head pos as fist body part on board.

  if(newApple)
  {
    vec3 applePos = emptiCells[rand() % emptiCellPointer]; // Get random empti cell
    board[applePos.x][applePos.y][applePos.z] = -1; // Save apple on board for next loop.
    newApple = 0;
  }
}

// Converts the bord data to the matrix wiith the corect blink timers.
void pushMatrix(){
  for (size_t x = 0; x < Board_Size; x++)
  {
    for (size_t y = 0; y < Board_Size; y++)
    {
      for (size_t z = 0; z < Board_Size; z++)
      {
        matrix[x][y][z] = 0;

        if(death){
           matrix[x][y][z] = Max_Death_Counter;
        }
        else if(board[x][y][z] == -1){
          matrix[x][y][z] = Max_Apple_Counter;
        }
        else if(board[x][y][z] == 1){
          matrix[x][y][z] = Max_Head_Counter;
        }
        else if(board[x][y][z] > 1){
          matrix[x][y][z] = 1;
        }
      }
    }
  }
}

// Draws the matrix
void drawMatrix(){
  matrixCounter++;

  for (size_t x = 0; x < Board_Size; x++)
  {
    for (size_t y = 0; y < Board_Size; y++)
    {
      for (size_t z = 0; z < Board_Size; z++)
      {
        bool data = matrix[x][y][z] == 1;
        if(matrix[x][y][z] > 1){
          data = (matrixCounter / matrix[x][y][z]) % 2 == 0;
        }
        int i = (y*3) +z;
        digitalWrite(X_Pins[i], !data);
      } 
    }

    digitalWrite(Y_Pins[x], 1);
    delay(Matrix_Delay);
    digitalWrite(Y_Pins[x], 0);
  }

  if(matrixCounter >= Max_Matrix_Counter){
    matrixCounter = 0;
  }
}

// The interupt function for the game loop
ISR(TIMER1_COMPA_vect){
  gameLoop();
  pushMatrix();
}

void setup() {
  cli(); // disable interrupts
  
  // reset
  TCCR1A = 0; // set TCCR1A register to 0
  TCCR1B = 0; // set TCCR1B register to 0
  TCNT1  = 0; // reset counter value
  
  OCR1A = 15624; // compare match register

  // set prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);   
  
  TCCR1B |= (1 << WGM12); // turn on CTC mode
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
  
  sei(); // allow interrupts

  Serial.begin(9600);

  // Set pinmodes
  for (size_t i = 0; i < X_Pins_Size; i++)
  {
    pinMode(X_Pins[i], OUTPUT);
  }
  for (size_t i = 0; i < Y_Pins_Size; i++)
  {
    pinMode(Y_Pins[i], OUTPUT);
  }
  for (size_t i = 0; i < Button_Pins_Size; i++)
  {
    pinMode(Button_Pins[i], INPUT);
  }

  // Start the game as death.
  death = 1;
}

// The loop only calls drawMartix the gameloop is called in the interupt loop.
void loop() {
  drawMatrix();
}