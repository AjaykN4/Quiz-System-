#ifndef QUIZWINDOW_H
#define QUIZWINDOW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QScrollArea>
#include <QFrame>
#include <QGroupBox>
#include <QProgressBar>
#include <QMessageBox>
#include <QTimer>
#include <QListWidget>
#include <QListWidgetItem>
#include <QFont>
#include <QPalette>
#include <QStyle>
#include <QApplication>
#include "quizmanager.h"
#include "timerwidget.h"

class QuizWindow : public QWidget
{
    Q_OBJECT

public:
    explicit QuizWindow(QuizManager* quizManager, QWidget *parent = nullptr);
    ~QuizWindow();

public slots:
    void onQuestionChanged(int index);
    void onAnswerSubmitted(int index, bool correct);
    void onTimeUpdated(int remainingSeconds);
    void onDifficultyChanged(Difficulty newDifficulty);
    void onProgressUpdated(int answered, int total);
    void onStreakUpdated(int consecutiveCorrect, int consecutiveWrong);
    void onQuizPaused();
    void onQuizResumed();
    void onQuizFinished();
    
    // Navigation slots
    void nextQuestion();
    void previousQuestion();
    void finishQuiz();
    void clearAnswer();
    void pauseQuiz();
    void resumeQuiz();
    void goToQuestion(int index);
    
    // Timer slots
    void onTimeUp();
    void onFlickerTimer();

private:
    // Quiz Manager
    QuizManager* m_quizManager;
    
    // UI Components - Left Panel
    QLabel* m_questionLabel;
    QButtonGroup* m_optionsGroup;
    QList<QRadioButton*> m_optionButtons;
    QPushButton* m_nextButton;
    QPushButton* m_previousButton;
    QPushButton* m_clearButton;
    QPushButton* m_finishButton;
    QPushButton* m_pauseButton;
    QPushButton* m_resumeButton;
    
    // UI Components - Right Panel
    TimerWidget* m_timerWidget;
    QLabel* m_progressLabel;
    QProgressBar* m_progressBar;
    QLabel* m_difficultyLabel;
    QLabel* m_streakLabel;
    QListWidget* m_questionGrid;
    QLabel* m_summaryLabel;
    
    // Layout
    QHBoxLayout* m_mainLayout;
    QVBoxLayout* m_leftPanelLayout;
    QVBoxLayout* m_rightPanelLayout;
    
    // State
    int m_currentQuestionIndex;
    bool m_isAnswered;
    
    // Flickering effect
    QTimer* m_flickerTimer;
    bool m_flickerState;
    QRadioButton* m_correctAnswerButton;
    
    // Spice it up feature
    QWidget* m_spiceOverlay;
    QLabel* m_spiceLabel;
    QTimer* m_spiceTimer;
    bool m_spiceShown;
    
    void setupUI();
    void setupLeftPanel();
    void setupRightPanel();
    void setupConnections();
    void setupSpiceItUpOverlay();
    void updateQuestionDisplay();
    void updateQuestionGrid();
    void updateProgress();
    void updateSummary();
    void updateStreakDisplay();
    void updateDifficultyDisplay();
    void showQuestionStatus(int index, bool answered, bool correct);
    void showFlawlessScreen();
    void startFlickerEffect(QRadioButton* correctButton);
    void stopFlickerEffect();
    
    // Helper methods
    QString formatTime(int seconds) const;
    QString getDifficultyColor(Difficulty difficulty) const;
    QString getStatusColor(bool answered, bool correct) const;
    void updateOptionButtonStyles();
    void resetOptionButtonStyles();
    void setupSound();
    void playWrongSound();
    
    // Spice it up methods
    void showSpiceItUp();
    void hideSpiceItUp();
    void onSpiceTimer();
};

#endif // QUIZWINDOW_H 