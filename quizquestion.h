#ifndef QUIZQUESTION_H
#define QUIZQUESTION_H

#include <QString>
#include <QStringList>
#include <QJsonObject>

enum class Difficulty {
    Easy,
    Medium,
    Hard
};

enum class QuestionType {
    MultipleChoice,
    TrueFalse
};

class QuizQuestion {
public:
    QuizQuestion();
    QuizQuestion(const QString& question, const QStringList& options, 
                 const QString& correctAnswer, Difficulty difficulty, 
                 QuestionType type = QuestionType::MultipleChoice);
    
    // Getters
    QString getQuestion() const;
    QStringList getOptions() const;
    QString getCorrectAnswer() const;
    Difficulty getDifficulty() const;
    QuestionType getType() const;
    bool isAnswered() const;
    QString getUserAnswer() const;
    bool isCorrect() const;
    int getTimeSpent() const;
    
    // Setters
    void setUserAnswer(const QString& answer);
    void setTimeSpent(int seconds);
    void setAnswered(bool answered);
    
    // Utility
    QString difficultyToString() const;
    static Difficulty stringToDifficulty(const QString& str);
    QJsonObject toJson() const;
    static QuizQuestion fromJson(const QJsonObject& json);
    
    // DSA algorithms for question analysis
    bool operator==(const QuizQuestion& other) const;
    bool operator<(const QuizQuestion& other) const;

private:
    QString m_question;
    QStringList m_options;
    QString m_correctAnswer;
    Difficulty m_difficulty;
    QuestionType m_type;
    QString m_userAnswer;
    bool m_answered;
    int m_timeSpent; // in seconds
};

#endif // QUIZQUESTION_H 