#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Stickman frame (simplified)
byte stickman[8] = {
  B01110,
  B11111,
  B01110,
  B00100,
  B01110,
  B10101,
  B01110,
  B10001
};

// Mountain obstacle (simple version)
byte mountain[8] = {
  B00100,
  B01110,
  B11111,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100
};

// Bird obstacle (improved bird)
byte bird[8] = {
   B00000,
  B00010,
  B01111,
  B10111,
  B10111,
  B01111,
  B00010,
  B00000
};

// Variables
int stickmanY = 1; // Start standing on the second row
bool jumping = false;
int jumpCounter = 0;
int score = 0;
int highScore = 0; // Variable to store the high score
int terrain[16]; // Array to store the terrain: 0=empty, 1=mountain, 2=bird
const int buttonPin = 2; // Pin for the jump button

bool gameOverState = false; // Track if game over state is active
bool highScoreDisplayed = false; // Track if high score screen is displayed

void setup() {
  lcd.init();
  lcd.backlight();
  
  lcd.createChar(0, stickman);
  lcd.createChar(1, mountain);
  lcd.createChar(2, bird);
  
  pinMode(buttonPin, INPUT_PULLUP); // Set button pin with internal pull-up resistor
  
  Serial.begin(9600); // for debug
  
  // Initialize game
  resetGame();
}

void loop() {
  if (gameOverState) {  // If in Game Over state
    if (digitalRead(buttonPin) == LOW) { // Button pressed (LOW means connected to GND)
      delay(200); // Debounce delay
      if (!highScoreDisplayed) {
        displayHighScore();
      } else {
        resetGame();
      }
    }
    return; // Don't do anything else during game over
  }

  if (score > 0) {
    lcd.clear();
  }
  
  // Handle Jump if button is pressed
  if (digitalRead(buttonPin) == LOW) { // Button pressed (LOW means connected to GND)
    if (jumping) {
      // If already jumping, go down immediately
      jumping = false;
      jumpCounter = 0;
      stickmanY = 1; // Return to ground
    } else {
      // Start a jump
      jumping = true;
    }
  }

  if (jumping) {
    stickmanY = 0; // Jumped up to top row
    jumpCounter++;
    if (jumpCounter > 5) { // Stay up for 5 frames
      jumping = false;
      jumpCounter = 0;
      stickmanY = 1; // Return to ground
    }
  }

  // Draw Stickman (simplified)
  lcd.setCursor(2, stickmanY); 
  lcd.write(byte(0)); // Draw stickman

  // Move terrain (mountain or bird)
  for (int i = 0; i < 16; i++) {
    if (terrain[i] == 1) { // Mountain
      lcd.setCursor(i, 1);  // Mountain on second row
      lcd.write(byte(1)); // Draw mountain
    } else if (terrain[i] == 2) { // Bird
      lcd.setCursor(i, 0);  // Bird on first row
      lcd.write(byte(2)); // Draw bird
    }
  }

  // Scroll terrain left
  for (int i = 0; i < 15; i++) {
    terrain[i] = terrain[i + 1];
  }
  generateTerrain(); // Generate new terrain at the end

  // Check collision with terrain
  if (terrain[2] == 1 && stickmanY == 1) { // Mountain collision
    gameOver();
  }
  if (terrain[2] == 2 && stickmanY == 0) { // Bird collision
    gameOver();
  }

  // Update score
  score++;
  Serial.print("Score: ");
  Serial.println(score);

  delay(150); // Adjust speed here
}

void generateTerrain() {
  int obstaclePosition = random(0, 4); // Randomly choose a position between 0-3

  // Ensure obstacles are spaced every 4 spaces
  if (obstaclePosition == 0) {
    int randObstacle = random(1, 3); // 1 = mountain, 2 = bird

    // Check if obstacle doesn't overlap vertically or diagonally
    bool overlap = false;
    if (terrain[15] == 1 || terrain[14] == 1) overlap = true; // Check for overlap with previous obstacles
    if (terrain[15] == 2 || terrain[14] == 2) overlap = true; // Same check for birds

    if (!overlap) {
      if (randObstacle == 1) {
        terrain[15] = 1; // Mountain
      } else if (randObstacle == 2) {
        terrain[15] = 2; // Bird
      }
    } else {
      terrain[15] = 0; // Empty space if overlap is detected
    }
  } else {
    terrain[15] = 0; // Empty space
  }
}

void gameOver() {
  gameOverState = true;
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("GAME OVER!");
  lcd.setCursor(0, 1);
  lcd.print("Score:");
  lcd.print(score);
  lcd.setCursor(10, 1);
  lcd.print("Retry?");
  if (score > highScore) {
    highScore = score; // Update high score
  }
  highScoreDisplayed = false; // Reset high score display state
}

void displayHighScore() {
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("HIGH SCORE");
  lcd.setCursor(6, 1);
  lcd.print(highScore);
  highScoreDisplayed = true; // Set high score display state
}

void resetGame() {
  score = 0;
  for (int i = 0; i < 16; i++) {
    terrain[i] = 0; // Initialize terrain with empty spaces
  }
  stickmanY = 1;
  jumping = false;
  jumpCounter = 0;
  gameOverState = false;
  highScoreDisplayed = false; // Reset high score display state
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Starting New Game!");
  delay(1000); // Display starting message for a second
}
