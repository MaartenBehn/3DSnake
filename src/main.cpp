#include <Arduino.h>

typedef struct vec3 {
    int x, y, z;
} vec3;

#define X_Pins_Size 9
int X_Pins[X_Pins_Size] = {4,5,6,7,8,9,12,11,10};

#define Y_Pins_Size 3
int Y_Pins[Y_Pins_Size] = {A1,A0,A2};

#define Button_Pins_Size 7
int Button_Pins[Button_Pins_Size] = {A5,A4,2,A6,3,A7,A3};

#define Board_Size 3
int board[3][3][3];
vec3 head;
int aktiveButton = 0; // 0 x+, 1 x-, 2 y+, 3 y-, 4 z+, 5 z-,
int snakeLenght;
bool newApple;
bool death;

vec3 emptiCells[Board_Size * Board_Size * Board_Size];
int emptiCellPointer;

#define Matrix_Delay 1
#define Max_Head_Counter 50
#define Max_Apple_Counter 100
#define Max_Death_Counter 300
int matrix[3][3][3];
int matrixCounter = 0;
#define Max_Matrix_Counter 10000

void setAllLEDS(bool state){
  for (size_t x = 0; x < Board_Size; x++)
  {
    for (size_t y = 0; y < Board_Size; y++)
    {
      for (size_t z = 0; z < Board_Size; z++)
      {
        matrix[x][y][z] = state;
      }
    }
  }
}

void newGame(){
  head = {1,1,1};
  snakeLenght = 1;
  aktiveButton = 1;
  newApple = 1;

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
  board[head.x][head.y][head.z] = 1;
  death = 0;
}

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

void gameLoop(){

  // Set move Direction from pressed buttons
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
      aktiveButton = i;
    }
  }

  switch (aktiveButton){ // Move head in move direction.
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
  case 6:
    newGame();
    return;
  }

  if(death){
    return;
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

  death = 1;
}

void loop() {
  drawMatrix();
}