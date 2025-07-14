#include "quizquestion.h"
#include <QJsonArray>

QuizQuestion::QuizQuestion()
    : m_difficulty(Difficulty::Easy)
    , m_type(QuestionType::MultipleChoice)
    , m_answered(false)
    , m_timeSpent(0)
{
}

QuizQuestion::QuizQuestion(const QString& question, const QStringList& options, 
                         const QString& correctAnswer, Difficulty difficulty, 
                         QuestionType type)
    : m_question(question)
    , m_options(options)
    , m_correctAnswer(correctAnswer)
    , m_difficulty(difficulty)
    , m_type(type)
    , m_answered(false)
    , m_timeSpent(0)
{
}

QString QuizQuestion::getQuestion() const {
    return m_question;
}

QStringList QuizQuestion::getOptions() const {
    return m_options;
}

QString QuizQuestion::getCorrectAnswer() const {
    return m_correctAnswer;
}

Difficulty QuizQuestion::getDifficulty() const {
    return m_difficulty;
}

QuestionType QuizQuestion::getType() const {
    return m_type;
}

bool QuizQuestion::isAnswered() const {
    return m_answered;
}

QString QuizQuestion::getUserAnswer() const {
    return m_userAnswer;
}

bool QuizQuestion::isCorrect() const {
    return m_answered && m_userAnswer == m_correctAnswer;
}

int QuizQuestion::getTimeSpent() const {
    return m_timeSpent;
}

void QuizQuestion::setUserAnswer(const QString& answer) {
    m_userAnswer = answer;
    m_answered = true;
}

void QuizQuestion::setTimeSpent(int seconds) {
    m_timeSpent = seconds;
}

void QuizQuestion::setAnswered(bool answered) {
    m_answered = answered;
}

QString QuizQuestion::difficultyToString() const {
    switch (m_difficulty) {
        case Difficulty::Easy: return "Easy";
        case Difficulty::Medium: return "Medium";
        case Difficulty::Hard: return "Hard";
        default: return "Unknown";
    }
}

Difficulty QuizQuestion::stringToDifficulty(const QString& str) {
    if (str.toLower() == "easy") return Difficulty::Easy;
    if (str.toLower() == "medium") return Difficulty::Medium;
    if (str.toLower() == "hard") return Difficulty::Hard;
    return Difficulty::Easy; // default
}

QJsonObject QuizQuestion::toJson() const {
    QJsonObject obj;
    obj["question"] = m_question;
    obj["correctAnswer"] = m_correctAnswer;
    obj["difficulty"] = difficultyToString();
    obj["type"] = (m_type == QuestionType::MultipleChoice) ? "multiple" : "boolean";
    obj["answered"] = m_answered;
    obj["userAnswer"] = m_userAnswer;
    obj["timeSpent"] = m_timeSpent;
    
    QJsonArray optionsArray;
    for (const QString& option : m_options) {
        optionsArray.append(option);
    }
    obj["options"] = optionsArray;
    
    return obj;
}

QuizQuestion QuizQuestion::fromJson(const QJsonObject& json) {
    QString question = json["question"].toString();
    QString correctAnswer = json["correctAnswer"].toString();
    Difficulty difficulty = stringToDifficulty(json["difficulty"].toString());
    
    QJsonArray optionsArray = json["options"].toArray();
    QStringList options;
    for (const QJsonValue& value : optionsArray) {
        options.append(value.toString());
    }
    
    QuizQuestion q(question, options, correctAnswer, difficulty);
    q.setUserAnswer(json["userAnswer"].toString());
    q.setAnswered(json["answered"].toBool());
    q.setTimeSpent(json["timeSpent"].toInt());
    
    return q;
}

bool QuizQuestion::operator==(const QuizQuestion& other) const {
    return m_question == other.m_question && 
           m_correctAnswer == other.m_correctAnswer &&
           m_difficulty == other.m_difficulty;
}

bool QuizQuestion::operator<(const QuizQuestion& other) const {
    if (m_difficulty != other.m_difficulty) {
        return static_cast<int>(m_difficulty) < static_cast<int>(other.m_difficulty);
    }
    return m_question < other.m_question;
} 