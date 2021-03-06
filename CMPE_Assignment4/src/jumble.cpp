#include "jumble.h"
#include <iostream>

/*
 * Class methods
 */
JumblePuzzle::JumblePuzzle(std::string _hiddenWord, std::string _difficulty)
  : difficulty(initDifficulty(_difficulty)),
    hiddenWord(initHiddenWord(_hiddenWord)) {

  this->puzzleSize = difficulty * hiddenWord.length();

  /*
   * Randomly assign a rowPos and columnPos;
   */
  std::srand(time(0));
  this->rowPos = std::rand() % this->puzzleSize;
  this->colPos = std::rand() % this->puzzleSize;

  /*
   * Determine the valid directions
   */
  char validDirections[4];
  int numberOfValidDirections = 
      JumblePuzzle ::getValidDirections(rowPos, 
                                        colPos, 
                                        puzzleSize, 
                                        hiddenWord, 
                                        validDirections);

  this->direction = validDirections[std::rand() % numberOfValidDirections];
  this->puzzle = JumblePuzzle::generatePuzzle(puzzleSize, direction, rowPos, colPos, hiddenWord);
}

JumblePuzzle::~JumblePuzzle() {
    for (int i = 0; i < puzzleSize; i++) {
        char* row = this->puzzle[i];
        delete[] row;
    }
    delete[] this->puzzle;
}

JumblePuzzle& JumblePuzzle::operator=(JumblePuzzle& right) {
    if (this == &right) {
        return right;
    }

    for (int i = 0; i < puzzleSize; i++) {
        char* row = this->puzzle[i];
        delete[] row;
    }
    delete[] this->puzzle;

    this->direction = right.direction;
    this->puzzleSize = right.puzzleSize;
    this->rowPos = right.rowPos;
    this->colPos = right.colPos;
    this->hiddenWord = right.hiddenWord;
    this->difficulty = right.difficulty;

    this->puzzle = new char*[puzzleSize];
    for (int i = 0; i < puzzleSize; i++) {
        puzzle[i] = new char[puzzleSize];
        for (int j = 0; j < puzzleSize; j++) {
            puzzle[i][j] = right.puzzle[i][j];
        }
    }

    return *this;
}


JumblePuzzle::JumblePuzzle(JumblePuzzle& right) :
  difficulty(right.difficulty),
  hiddenWord(right.hiddenWord),
  puzzleSize(right.puzzleSize), 
  direction(right.direction),
  rowPos(right.rowPos), 
  colPos(right.colPos) {
  this->puzzle = new char*[puzzleSize];
  for (int i = 0; i < puzzleSize; i++) {
      puzzle[i] = new char[puzzleSize];
      for (int j = 0; j < puzzleSize; j++) {
          puzzle[i][j] = right.puzzle[i][j];
      }
  }
}

/*
 * Static methods
 */
Difficulty JumblePuzzle::initDifficulty(std::string& difficulty) {
  if (difficulty == "easy") {
    return EASY;
  }
  if (difficulty == "medium") {
    return MEDIUM;
  }

  if (difficulty == "hard") {
    return HARD;
  }

  throw BadJumbleException("Difficulty must be either easy, medium or hard.");
}

const std::string JumblePuzzle::initHiddenWord(std::string hiddenWord) {
  if (hiddenWord.length() > 10) {
    throw BadJumbleException("Word must not be greater than 10 characters");
  }

  if (hiddenWord.length() < 3) {
    throw BadJumbleException("Word must not be less than 3 characters");
  }

  for (char& c : hiddenWord) {
    if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z')) {
      continue;
    }
    throw BadJumbleException("Only characters between a-z are allowed.");
  }

  return hiddenWord;
}

int JumblePuzzle::getValidDirections(
        int rowPos,
        int colPos,
        int puzzleSize,
        const std::string& hiddenWord,
        char* outDirections) {
  int numberOfValidDirections = 0;

  int lettersToAdd = (int) hiddenWord.length() - 1;
  if (rowPos - lettersToAdd >= 0) {
    outDirections[numberOfValidDirections] = 'n';
    numberOfValidDirections++;
  }
  if (rowPos + lettersToAdd < puzzleSize) {
    outDirections[numberOfValidDirections] = 's';
    numberOfValidDirections++;
  }
  if (colPos + lettersToAdd < puzzleSize) {
    outDirections[numberOfValidDirections] = 'e';
    numberOfValidDirections++;
  }
  if (colPos - lettersToAdd >= 0) {
    outDirections[numberOfValidDirections] = 'w';
    numberOfValidDirections++;
  }

  return numberOfValidDirections;
}

/*
 * Creation of jumbled puzzle. Note, the puzzle is created from west to east,
 * north to south. This means that if the direction is north or east, the 
 * hidden word will naturally be places in the proper order.
 *
 * E.g. If the hidden word is "FACE":
 *
 * Going west to east, the letters in order like:
 * F A C E (this is the east configuration)
 *
 * Going north to south, the letters will in order like:
 * F
 * A
 * C
 * E (this is south configuration).
 *
 * For placing west/north, the words will appear like:
 *
 * E C A F
 *
 * or
 *
 * E
 * C
 * A
 * F
 *
 * In these two cases, the letter that will be placed first is the 
 * last letter in the hidden word. Therefore, first row and column
 * will have to be offset by the length of the word.
 *
 */
char** JumblePuzzle::generatePuzzle(
        int puzzleSize, 
        char direction, 
        int rowPos, 
        int colPos, 
        const std::string& hiddenWord) {
  char** puzzle = new char*[puzzleSize];

  int cornerRow;
  int cornerCol;
  int wordIndex;
  int diffIndex;
  if (direction == 's' || direction == 'e') {
      cornerRow = rowPos;
      cornerCol = colPos;
      wordIndex = 0;
      diffIndex = 1;
  } 
  else if (direction == 'n') {  // North
      cornerRow = rowPos - (hiddenWord.length() - 1);
      cornerCol = colPos;
      wordIndex = hiddenWord.length() - 1;
      diffIndex = -1;
  }
  else {  // West
      cornerRow = rowPos;
      cornerCol = colPos - (hiddenWord.length() - 1);
      wordIndex = hiddenWord.length() - 1;
      diffIndex = -1;
  }

  bool placingHiddenWord = false;
  bool directionIsVertical = direction == 'n' || direction == 's';
  for (int row = 0; row < puzzleSize; row++) {
      puzzle[row] = new char[puzzleSize];
      for (int col = 0; col < puzzleSize; col++) {
          if (row == cornerRow && col == cornerCol) {
              placingHiddenWord = true;
          }

          bool canPlaceVertical = col == cornerCol && directionIsVertical;
          bool canPlaceHorizontal = row == cornerRow && !directionIsVertical;

          if (placingHiddenWord && (canPlaceHorizontal || canPlaceVertical)) {
              puzzle[row][col] = hiddenWord[wordIndex];
              wordIndex += diffIndex;
              if (wordIndex < 0 || wordIndex == hiddenWord.length()) {
                  placingHiddenWord = false;
              }
          }
          else {
              char randomLetter = std::rand() % 26 + 'a';
              puzzle[row][col] = randomLetter;
          }
      }
  }

  return puzzle;
}
