#ifndef QUIZMANAGER_H
#define QUIZMANAGER_H

#include <QObject>
#include <QTimer>
#include <QQueue>
#include <QStack>
#include <QMap>
#include <QList>
#include <QSet>
#include <QPair>
#include <QVector>
#include <QHash>
#include <queue>
#include <QRandomGenerator>
#include "quizquestion.h"
#include "apimanager.h"

class QuizManager : public QObject
{
    Q_OBJECT

public:
    explicit QuizManager(QObject *parent = nullptr);
    ~QuizManager();

    // Quiz Control
    void startQuiz(const QString& quizType = "default");
    void pauseQuiz();
    void resumeQuiz();
    void finishQuiz();
    void resetQuiz();
    
    // Question Navigation
    void nextQuestion();
    void previousQuestion();
    void goToQuestion(int index);
    void answerQuestion(const QString& answer);
    void clearAnswer();
    
    // Quiz State
    bool isQuizActive() const;
    bool isQuizPaused() const;
    int getCurrentQuestionIndex() const;
    int getTotalQuestions() const;
    int getAnsweredCount() const;
    int getCorrectCount() const;
    int getRemainingTime() const;
    QuizQuestion getCurrentQuestion() const;
    QList<QuizQuestion> getAllQuestions() const;
    
    // Adaptive Logic
    Difficulty getCurrentDifficulty() const;
    int getConsecutiveCorrect() const;
    int getConsecutiveWrong() const;
    void updateDifficulty();
    
    // Analytics
    QMap<Difficulty, int> getDifficultyStats() const;
    QMap<Difficulty, double> getAccuracyByDifficulty() const;
    double getOverallAccuracy() const;
    double getAverageTimePerQuestion() const;
    QList<int> getTimePerQuestion() const;
    
    // Custom Quiz
    void loadCustomQuiz(const QString& name);
    void createCustomQuiz(const QString& name, const QList<QuizQuestion>& questions);

signals:
    void quizStarted();
    void quizPaused();
    void quizResumed();
    void quizFinished();
    void questionChanged(int index);
    void answerSubmitted(int index, bool correct);
    void timeUpdated(int remainingSeconds);
    void difficultyChanged(Difficulty newDifficulty);
    void progressUpdated(int answered, int total);
    void streakUpdated(int consecutiveCorrect, int consecutiveWrong);

private slots:
    void updateTimer();
    void onQuestionsFetched(const QList<QuizQuestion>& questions);
    void onApiError(const QString& error);

private:
    // Quiz State
    bool m_isActive;
    bool m_isPaused;
    int m_currentIndex;
    int m_totalQuestions;
    int m_answeredCount;
    int m_correctCount;
    int m_remainingTime;
    
    // Questions
    QList<QuizQuestion> m_questions;
    QStack<int> m_questionHistory;
    
    // Adaptive Logic
    Difficulty m_currentDifficulty;
    int m_consecutiveCorrect;
    int m_consecutiveWrong;
    int m_easyCorrect;
    int m_mediumCorrect;
    int m_hardCorrect;
    
    // Timer
    QTimer* m_timer;
    int m_initialTime; // in seconds
    
    // API Manager
    ApiManager* m_apiManager;
    
    // DSA Algorithms for question selection
    void initializeQuestionQueue();
    void selectNextQuestion();
    void adjustDifficulty();
    void updateStreaks(bool isCorrect);
    QList<QuizQuestion> getQuestionsForDifficulty(Difficulty difficulty);
    
    // Advanced DSA Algorithms
    void buildQuestionPriorityQueue();
    void updateQuestionWeights();
    void adaptiveQuestionSelection();
    void weightedRandomSelection();
    void difficultyBasedSelection();
    void performanceBasedSelection();
    void timeBasedSelection();
    
    // Priority Queue for question selection
    struct QuestionPriority {
        int questionIndex;
        double priority;
        Difficulty difficulty;
        int timeSpent;
        bool answered;
        
        bool operator<(const QuestionPriority& other) const {
            return priority < other.priority;
        }
    };
    
    std::priority_queue<QuestionPriority> m_questionQueue;
    QHash<int, double> m_questionWeights;
    QSet<int> m_answeredQuestions;
    QMap<Difficulty, QList<int>> m_questionsByDifficulty;
    QMap<Difficulty, double> m_difficultyWeights;
    
    // Performance tracking for adaptive selection
    QMap<Difficulty, double> m_accuracyByDifficulty;
    QMap<Difficulty, double> m_averageTimeByDifficulty;
    QMap<Difficulty, int> m_attemptsByDifficulty;
    
    // Analytics tracking
    QMap<Difficulty, int> m_difficultyStats;
    QMap<Difficulty, int> m_correctByDifficulty;
    QList<int> m_timePerQuestion;
    
    // Helper methods
    void calculateStats();
    void saveQuizResults();
    void loadQuizResults();
    void updatePerformanceMetrics(Difficulty difficulty, bool isCorrect, int timeSpent);
};

#endif // QUIZMANAGER_H 