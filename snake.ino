#include <FastLED.h>
#include <math.h>

// ==========================================================
// DEFINE CONSTANTS
// ==========================================================

// Enable debug output over serial port
#define DEBUG true

// Output for data pin for WS2811 led matrix
#define DATA_PIN 3

// Dimensions of led matrix
#define NCOLS 16
#define NROWS 16

// Define maximum length of tail
#define TAIL_MAX 50

// ==========================================================
// DEFINE GLOBAL VARIABLES
// ==========================================================

// Array for color values of leds
CRGB leds[NCOLS*NROWS];

// Head position
int head;
// Array for tail
int tail[TAIL_MAX];
// Tail length
int tail_len;
// Direction
//   0: up
//   1: right
//   2: down
//   3: left
int direction;
// Food position
int food;
// level (1 ... 9)
//   1 => tick time 1000 ms
//   2 => tick time  900 ms
//   ...
//   9 => tick time  100 ms
int level;
boolean gameover;

// Timer value
unsigned long tick;

// ==========================================================
// SETUP
//
void setup() {

  // Initialize serial port
  Serial.begin(9600);
  while (!Serial) {
    ;
  }

  //Initialize led matrix
  delay(2000);
  //FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NCOLS*NROWS);
  FastLED.addLeds<WS2811, DATA_PIN, GRB>(leds, NCOLS*NROWS);

  // Initialize some values
  head = 136;
  tail_len = 0;
  direction = 0; 

  // Initialize random number generator
  randomSeed(analogRead(0));

  // Initialize level (1 ... 10)
  level = 1;

  // Initialize timer. After a specific amount of time has passed,
  // move the snake one position further. The amount of time depends
  // on the level. 
  tick = millis();

  // Initialy place food
  food = place_food();

  // Draw initial screen
  update_screen(false);
}

// ==========================================================
// print_movement
//
// Description
//   For debuggin only
//
// Parameter
//   int old_pos        old position
//   int new_pos        new position
//
// Returns
//   none
//
void print_movement(int old_pos, int new_pos) {
  if (DEBUG) {
    Serial.print(old_pos);
    Serial.print(" --> ");
    Serial.println(new_pos);
  }
}

// ==========================================================
// move_left
//
// Description
//   Moves given position to left. Returns old_pos, if the
//   given position is already at the left border.
//
// Parameter
//   int old_pos        old position to move left
//
// Returns
//   int new_pos
int move_left(int old_pos) {
  int new_pos;

  if ((old_pos + 1) % (2 * NCOLS) == 0 || old_pos % (2 * NCOLS) == 0) {
    new_pos = old_pos;
  } else {
    if ((int)floor(old_pos/NCOLS) % 2 == 0) {
      new_pos = old_pos - 1;
    } else {
      new_pos = old_pos + 1;
    }
  }    

  print_movement(old_pos, new_pos);

  return new_pos;
}

// ==========================================================
// move_right
//
// Description
//   Moves given position to right. Returns old_pos, if the
//   given position is already at the right border.
//
// Parameter
//   int old_pos        old position to move right
//
// Returns
//   int new_pos
//
int move_right(int old_pos) {
  int new_pos;

  if ((old_pos + 1) % (2 * NCOLS) == 16 || old_pos % (2 * NCOLS) == 16) {
    new_pos = old_pos;  
  } else {
    if ((int)floor(old_pos/NCOLS) % 2 == 0) {
      new_pos = old_pos + 1;
    } else {
      new_pos = old_pos - 1;
    }
  }
  
  print_movement(old_pos, new_pos);
  
  return new_pos;
}

// ==========================================================
// move_up
//
// Description
//   Moves given position up. Returns old_pos, if the
//   given position is already at the top border.
//
// Parameter
//   int old_pos        old position to up
//
// Returns
//   int new_pos
//
int move_up(int old_pos) {
  int new_pos;

  if (old_pos > (NROWS * (NCOLS - 1) -1)) {
    new_pos = old_pos;
  } else {
    new_pos = 2 * NCOLS - old_pos - 1 + 2 * (int)floor(old_pos / NCOLS) * NCOLS;
  }
  
  print_movement(old_pos, new_pos);
  
  return new_pos;
}

// ==========================================================
// move_down
//
// Description
//   Moves given position down. Returns old_pos, if the
//   given position is already at the bottom border.
//
// Parameter
//   int old_pos        old position to move down
//
// Returns
//   int new_pos
//
int move_down(int old_pos) {
  int new_pos;

  if (old_pos < NROWS) {
    new_pos = old_pos;
  } else {
    new_pos = old_pos - 2 * (old_pos - NCOLS) - 1 + 2 * ((int)floor(old_pos / NCOLS) - 1) * NCOLS;
  }
  
  print_movement(old_pos, new_pos);
  
  return new_pos;
}

int mmove(int old_pos, int direction) {
  int new_pos;
  
  if (direction == 3) {
    new_pos = move_left(old_pos);
  } else if (direction == 2) {
    new_pos = move_down(old_pos);
  } else if (direction == 0) {
    new_pos = move_up(old_pos);
  } else {
    new_pos = move_right(old_pos);
  }

  return new_pos;
}

// ==========================================================
// place_food
//
// Description
//   Places food at random position and takes care, that
//   food is not places inside snake.
//
// Parameter
//   none
//
//   (Processes global variables head, tail and tail_len)
//
// Returns
//   int food           food position
//
int place_food() {
  int food;
  int last, next;
  boolean in_head = true;
  boolean in_tail = true;
  int iter = 0;

  // Do not place food in head or tail of snake
  while (in_head || in_tail) {
    iter++;
    food = (int)random(NCOLS*NROWS + 1);

    // Food in head?
    if (food != head) {
      in_head = false;
    }
    // Food in tail?
    // Before iterating through tail, set variable in_tail to false
    in_tail = false;
    for (int l = 0; l < tail_len; l++) {
      if (l == 0) {
        last = head;
      } else {
        last = next;
      }
      
      if (tail[l] == 3) {
        next = move_right(last);
      } else if (tail[l] == 2) {
        next = move_up(last);
      } else if (tail[l] == 0) {
        next = move_down(last);
      } else if (tail[l] == 1) {
        next = move_left(last);
      }
      
      // If we found the food to be somewhere in the tail, we can stop the
      // for loop
      if (food == next) {
        in_tail = true;
        break;
      }
    }
  }

  if (DEBUG) {
    Serial.print("Placed food[");
    Serial.print(iter);
    Serial.print("]: ");
    Serial.println(food);
  }
  
  return food;
}

void print_tail() {
  if (DEBUG) {
    Serial.print("Tail[");
    Serial.print(tail_len);
    Serial.print("]: ");
    for (int l = 0; l < tail_len; l++) {
      Serial.print(tail[l]);
      Serial.print(" ");
    }
    Serial.println("");
  }
}

// ==========================================================
// update_screen
//
// Description
//   Updates/prints the led matrix
//
// Parameter
//   boolean failed             indicater wheather (not) to
//                              set failed indicator on
//                              led matrix in debug mode.
//
//   (Processes global variables head, tail, tail_len, food
//    and leds)
//
// Returns
//   none
//
void update_screen(boolean failed) {
  // first, set all leds to black
  for (int i = 0; i < NCOLS*NROWS; i++) {
    leds[i] = CRGB::Black;
  }

  // Only for debugging
  if (DEBUG && failed) {
    leds[0] = CRGB::White;
  }

  // Set the head to blue
  leds[head] = CRGB::Blue;

  // Set the food to red
  leds[food] = CRGB::Red;

  // Draw the tail in green
  int next;
  for (int l = 0; l < tail_len; l++) {
    int last;
    if (l == 0) {
      last = head;
    } else {
      last = next;
    }
    
    if (tail[l] == 3) {
      next = move_right(last);
    } else if (tail[l] == 2) {
      next = move_up(last);
    } else if (tail[l] == 0) {
      next = move_down(last);
    } else if (tail[l] == 1) {
      next = move_left(last);
    }
    
    leds[next] = CRGB::Green;
  }
  
  FastLED.show();
}

// ==========================================================
// check_if_gameover
//
// Description
//   Check if game is over. It's the case, if we want to
//   move outside the boundaries of the led matrix or if
//   the head of the snake touches its tail.
//
// Parameter
//   none
//
//   (Processes global variables head, tail, tail_len, food))
//
// Returns
//   boolean failed             true if failed, otherwise
//                              false
//
boolean check_if_gameover() {
  boolean gameover = false;
  int dir, head_next, last, next;

  // Game over, if in the upper border and moving up
  if ((head > (NROWS * (NCOLS - 1) -1)) && direction == 0) {
    gameover = true;
  }

  // Game over, if on the right border and moving right
  if (((head + 1) % (2 * NCOLS) == 16 || head % (2 * NCOLS) == 16) && direction == 1) {
    gameover = true;
  }
  
  // Game over, if on the bottom border an moving down
  if ((head < NROWS) && direction == 2) {
    gameover = true;
  }

  // Game over, if on the left border and moving left
  if (((head + 1) % (2 * NCOLS) == 0 || head % (2 * NCOLS) == 0) && direction == 3) {
    gameover = true;
  }

  // Game over, if we suddenly change the direction by 180 degrees.
  // This only applies, if the snake has a tail. Then, we can compare the current
  // direction with the first element of the tail array.
  if (tail_len > 0 && tail[0] == (direction + 2) % 4) {
    gameover = true;
  }

  // Failed, if head would touch the tail if moving further
  // Therefore, we first calculate the following head position and check,
  // if it touches any position in the tail.
  head_next = mmove(head, direction);

  // Now, loop over the snake
  for (int l = 0; l < tail_len; l++) {
    // The first tail element of the snake is calculated relative to the head position.
    // The following tail elements are calculated relative to the previous tail element
    // position.
    if (l == 0) {
      last = head;
    } else {
      last = next;
    }

    // The array tail does not contain the "coordinates" of the tail elements, but
    // directions, from which the tail coordintes have to be calculated. This is
    // done in the following line by updating the variable next.
    // IMPORTANT: The directions in tail have to be rotated by 180 degrees, first!
    next = mmove(last, (tail[l] + 2) % 4 );

    // Now, check if head_next would touch the current computed snake element (next)
    if (head_next == next) {
      gameover = true;
      break;
    }
  }

  if (DEBUG) {
    Serial.print("Game over? ");
    Serial.println(gameover);
  }

  return gameover;
}

// ==========================================================
// MAIN LOOP
//
void loop() {

  // See, if enough time has elapsed to move the snake one position
  // further
  if (millis() - tick >= 1000 - 100 * (level -1)) {
    if (DEBUG) {
      Serial.println("=== tick ===");
    }

    // Set tick to current time
    tick = millis();

  // Read serial port and update direction, if input available
  if (Serial.available() > 0) {
    int inchar=Serial.read();
    //Serial.print("I received: ");
    //Serial.println(inchar);
    
    // h => left
    if (inchar == 104) {
      direction = 3;
    // j => down
    } else if (inchar == 106) {
      direction = 2;
    // k => up
    } else if (inchar == 107) {
      direction = 0;
    // l => right
    } else if (inchar == 108) {
      direction = 1;
    }
  }

    // Check if game is over
    gameover = check_if_gameover();

    // Move head
    head = mmove(head, direction);
    
    if (head == food) {
      food = place_food();
      tail_len++;
    }

    // Update tail. Forget the last tail element and shift the tail one
    // element "tail-wards". Then, set first element in tail with
    // current direction
    for (int l = tail_len - 1; l >= 1; l--) {
      tail[l] = tail[l - 1];
    }
    tail[0] = direction;  

    print_tail();
    
    update_screen(gameover);
  } 
}
