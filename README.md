# Quiz System

This is a Qt-based Quiz System application that fetches quiz questions from online APIs and provides a quiz interface with various question categories and difficulty levels.

## Features

- Fetches current affairs questions from NewsAPI.
- Fetches programming questions from GeeksforGeeks API.
- Supports local fallback questions if API requests fail.
- Categorizes questions by difficulty: Easy, Medium, Hard.
- Displays quiz statistics and charts.
- Allows saving and loading custom quizzes.

## Setup and Build

### Prerequisites

- Qt framework installed (version compatible with QtCharts, QtWidgets, QtNetwork, etc.)
- C++17 compatible compiler (e.g., clang++)
- qmake build system

### Build Instructions

1. Run qmake to generate the Makefile:

   ```
   qmake QuizSystem.pro
   ```

2. Build the project using make:

   ```
   make
   ```

3. Run the application:

   ```
   ./QuizSystem.app/Contents/MacOS/QuizSystem
   ```

## Configuration

- The NewsAPI key is set in `apimanager.cpp` in the `fetchQuestions` method. Replace the placeholder API key with your own valid key.

## Usage

- Launch the application and select quiz categories.
- Answer the questions presented.
- View quiz statistics and charts after completion.
- Save and load custom quizzes as needed.

## License

This project is licensed under the MIT License.
