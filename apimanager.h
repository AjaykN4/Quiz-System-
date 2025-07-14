#ifndef APIMANAGER_H
#define APIMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QTimer>
#include "quizquestion.h"

class ApiManager : public QObject
{
    Q_OBJECT

public:
    explicit ApiManager(QObject *parent = nullptr);
    ~ApiManager();

    // API Methods
    void fetchQuestions(int count);
    void fetchCustomQuestions();
    
    // Local question management
    QList<QuizQuestion> getDefaultQuestions();
    QList<QuizQuestion> getCppDSAQuestions();
    QList<QuizQuestion> getQuestionsByDifficulty(Difficulty difficulty);
    
    // Question storage
    void saveCustomQuiz(const QString& name, const QList<QuizQuestion>& questions);
    QList<QuizQuestion> loadCustomQuiz(const QString& name);
    QStringList getAvailableCustomQuizzes();

signals:
    void questionsFetched(const QList<QuizQuestion>& questions);
    void errorOccurred(const QString& error);
    void loadingStateChanged(bool loading);

private slots:
    void onNetworkReplyFinished();
    void onNetworkTimeout();

private:
    QNetworkAccessManager* m_networkManager;
    QTimer* m_timeoutTimer;
    bool m_isLoading;
    
    // API Configuration
    QString m_apiUrl;
    QString m_apiKey;
    
    // Local question storage
    QList<QuizQuestion> m_defaultQuestions;
    QList<QuizQuestion> m_cppDSAQuestions;
    
    // Helper methods
    void initializeDefaultQuestions();
    void initializeCppDSAQuestions();
    QList<QuizQuestion> parseApiResponse(const QJsonDocument& response);
    void handleApiError(const QString& error);
    QList<QuizQuestion> getLocalCurrentAffairsQuestions(int count);
    
    // DSA algorithms for question selection
    QList<QuizQuestion> selectQuestionsByDifficulty(const QList<QuizQuestion>& questions, 
                                                   Difficulty difficulty, int count);
    QList<QuizQuestion> shuffleQuestions(const QList<QuizQuestion>& questions);
};

#endif // APIMANAGER_H 