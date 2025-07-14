#include "apimanager.h"
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QApplication>
#include <QRandomGenerator>
#include <algorithm>
#include <random>

ApiManager::ApiManager(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_timeoutTimer(new QTimer(this))
    , m_isLoading(false)
{
    // Connect network signals
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &ApiManager::onNetworkReplyFinished);
    
    // Setup timeout timer
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout, this, &ApiManager::onNetworkTimeout);
    
    // Initialize local questions
    initializeDefaultQuestions();
    initializeCppDSAQuestions();
}

ApiManager::~ApiManager()
{
}

void ApiManager::fetchQuestions(int count)
{
    // Fetch current affairs questions from a free API
    // Using NewsAPI for current affairs (free tier available)
    QString apiKey = "demo"; // Replace with actual API key for production
    QString url = QString("https://newsapi.org/v2/top-headlines?country=us&apiKey=%1&pageSize=%2")
                  .arg(apiKey)
                  .arg(count);
    
    QNetworkRequest request((QUrl(url)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    m_isLoading = true;
    emit loadingStateChanged(true);
    m_timeoutTimer->start(10000); // 10 second timeout
    
    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, [this, reply, count]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject response = doc.object();
            
            if (response["status"].toString() == "ok") {
                QJsonArray articles = response["articles"].toArray();
                QList<QuizQuestion> quizQuestions;
                
                for (int i = 0; i < qMin(articles.size(), count); ++i) {
                    QJsonObject article = articles[i].toObject();
                    QString title = article["title"].toString();
                    QString description = article["description"].toString();
                    
                    if (!title.isEmpty() && !description.isEmpty()) {
                        // Create question from news article
                        QString question = QString("What is the main topic of this news: %1?").arg(title);
                        
                        // Create options based on the article content
                        QStringList options;
                        options.append("Current Affairs"); // Correct answer
                        options.append("Sports");
                        options.append("Entertainment");
                        options.append("Technology");
                        
                        // Shuffle options
                        std::mt19937 rng(QRandomGenerator::global()->generate());
                        std::shuffle(options.begin(), options.end(), rng);
                        
                        // Determine difficulty based on title length
                        Difficulty difficulty = Difficulty::Easy;
                        if (title.length() > 100) {
                            difficulty = Difficulty::Hard;
                        } else if (title.length() > 60) {
                            difficulty = Difficulty::Medium;
                        }
                        
                        QuizQuestion quizQuestion(question, options, "Current Affairs", difficulty);
                        quizQuestions.append(quizQuestion);
                    }
                }
                
                emit questionsFetched(quizQuestions);
            } else {
                // Fallback to local current affairs questions
                emit questionsFetched(getLocalCurrentAffairsQuestions(count));
            }
        } else {
            // Fallback to local current affairs questions
            emit questionsFetched(getLocalCurrentAffairsQuestions(count));
        }
        
        reply->deleteLater();
    });
}

void ApiManager::fetchCustomQuestions()
{
    // Fetch programming questions from GeeksforGeeks API
    QString url = "https://api.geeksforgeeks.org/problems/get-problems?category=all&difficulty=all&type=all";
    
    QNetworkRequest request((QUrl(url)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    m_isLoading = true;
    emit loadingStateChanged(true);
    m_timeoutTimer->start(10000); // 10 second timeout
    
    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject response = doc.object();
            
            if (response["status"].toString() == "SUCCESS") {
                QJsonArray problems = response["results"].toArray();
                QList<QuizQuestion> quizQuestions;
                
                for (const QJsonValue& value : problems) {
                    QJsonObject problem = value.toObject();
                    QString title = problem["title"].toString();
                    QString difficultyStr = problem["difficulty"].toString();
                    
                    if (!title.isEmpty()) {
                        // Create question from problem title
                        QString question = QString("What is the main concept tested in: %1?").arg(title);
                        
                        // Create options based on common DSA concepts
                        QStringList options;
                        options.append("Data Structures");
                        options.append("Algorithms");
                        options.append("Dynamic Programming");
                        options.append("Graph Theory");
                        
                        // Shuffle options
                        std::mt19937 rng(QRandomGenerator::global()->generate());
                        std::shuffle(options.begin(), options.end(), rng);
                        
                        // Determine difficulty
                        Difficulty difficulty = Difficulty::Easy;
                        if (difficultyStr == "Hard") {
                            difficulty = Difficulty::Hard;
                        } else if (difficultyStr == "Medium") {
                            difficulty = Difficulty::Medium;
                        }
                        
                        QuizQuestion quizQuestion(question, options, "Data Structures", difficulty);
                        quizQuestions.append(quizQuestion);
                    }
                }
                
                emit questionsFetched(quizQuestions);
            } else {
                // Fallback to local C++/DSA questions
                emit questionsFetched(getCppDSAQuestions());
            }
        } else {
            // Fallback to local C++/DSA questions
            emit questionsFetched(getCppDSAQuestions());
        }
        
        reply->deleteLater();
    });
}

QList<QuizQuestion> ApiManager::getDefaultQuestions()
{
    return m_defaultQuestions;
}

QList<QuizQuestion> ApiManager::getCppDSAQuestions()
{
    return m_cppDSAQuestions;
}

QList<QuizQuestion> ApiManager::getQuestionsByDifficulty(Difficulty difficulty)
{
    return selectQuestionsByDifficulty(m_cppDSAQuestions, difficulty, 10);
}

void ApiManager::saveCustomQuiz(const QString& name, const QList<QuizQuestion>& questions)
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    QString filePath = dir.filePath(name + ".json");
    QFile file(filePath);
    
    if (file.open(QIODevice::WriteOnly)) {
        QJsonArray questionsArray;
        for (const QuizQuestion& question : questions) {
            questionsArray.append(question.toJson());
        }
        
        QJsonObject quizObject;
        quizObject["name"] = name;
        quizObject["questions"] = questionsArray;
        
        QJsonDocument doc(quizObject);
        file.write(doc.toJson());
        file.close();
    }
}

QList<QuizQuestion> ApiManager::loadCustomQuiz(const QString& name)
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString filePath = QDir(dataPath).filePath(name + ".json");
    QFile file(filePath);
    
    QList<QuizQuestion> questions;
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonObject quizObject = doc.object();
        QJsonArray questionsArray = quizObject["questions"].toArray();
        
        for (const QJsonValue& value : questionsArray) {
            questions.append(QuizQuestion::fromJson(value.toObject()));
        }
        file.close();
    }
    
    return questions;
}

QStringList ApiManager::getAvailableCustomQuizzes()
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataPath);
    QStringList quizzes;
    
    if (dir.exists()) {
        QStringList filters;
        filters << "*.json";
        QStringList files = dir.entryList(filters, QDir::Files);
        
        for (const QString& file : files) {
            QString name = file;
            name.remove(".json");
            quizzes.append(name);
        }
    }
    
    return quizzes;
}

void ApiManager::onNetworkReplyFinished()
{
    m_timeoutTimer->stop();
    m_isLoading = false;
    emit loadingStateChanged(false);
    
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        
        QList<QuizQuestion> questions = parseApiResponse(doc);
        emit questionsFetched(questions);
    } else {
        handleApiError(reply->errorString());
    }
    
    reply->deleteLater();
}

void ApiManager::onNetworkTimeout()
{
    m_isLoading = false;
    emit loadingStateChanged(false);
    emit errorOccurred("Request timed out");
}

void ApiManager::initializeDefaultQuestions()
{
    // Add some general knowledge questions as default
    m_defaultQuestions = {
        QuizQuestion("What is the capital of France?", 
                    {"London", "Berlin", "Paris", "Madrid"}, 
                    "Paris", Difficulty::Easy),
        QuizQuestion("Which planet is closest to the Sun?", 
                    {"Venus", "Mercury", "Earth", "Mars"}, 
                    "Mercury", Difficulty::Easy),
        QuizQuestion("What is 2 + 2?", 
                    {"3", "4", "5", "6"}, 
                    "4", Difficulty::Easy)
    };
}

void ApiManager::initializeCppDSAQuestions()
{
    m_cppDSAQuestions = {
        // Easy C++/DSA Questions
        QuizQuestion("What is the time complexity of accessing an element in an array?", 
                    {"O(1)", "O(n)", "O(log n)", "O(n²)"}, 
                    "O(1)", Difficulty::Easy),
        QuizQuestion("Which data structure follows LIFO principle?", 
                    {"Queue", "Stack", "Tree", "Graph"}, 
                    "Stack", Difficulty::Easy),
        QuizQuestion("What is the default constructor in C++?", 
                    {"A constructor with no parameters", "A constructor with all default parameters", 
                     "A constructor that initializes all members to zero", "A constructor that takes a class object as parameter"}, 
                    "A constructor with no parameters", Difficulty::Easy),
        
        // Medium C++/DSA Questions
        QuizQuestion("What is the time complexity of binary search?", 
                    {"O(1)", "O(n)", "O(log n)", "O(n²)"}, 
                    "O(log n)", Difficulty::Medium),
        QuizQuestion("Which sorting algorithm has the best average-case time complexity?", 
                    {"Bubble Sort", "Quick Sort", "Selection Sort", "Insertion Sort"}, 
                    "Quick Sort", Difficulty::Medium),
        QuizQuestion("What is a virtual function in C++?", 
                    {"A function that can be overridden in derived classes", "A function that cannot be inherited", 
                     "A function that is automatically called", "A function that returns void"}, 
                    "A function that can be overridden in derived classes", Difficulty::Medium),
        
        // Hard C++/DSA Questions
        QuizQuestion("What is the time complexity of merge sort?", 
                    {"O(n log n)", "O(n²)", "O(n)", "O(log n)"}, 
                    "O(n log n)", Difficulty::Hard),
        QuizQuestion("What is a smart pointer in C++?", 
                    {"A pointer that automatically manages memory", "A pointer that points to smart objects", 
                     "A pointer that can only point to integers", "A pointer that is always null"}, 
                    "A pointer that automatically manages memory", Difficulty::Hard),
        QuizQuestion("What is the space complexity of recursive Fibonacci implementation?", 
                    {"O(1)", "O(n)", "O(2^n)", "O(log n)"}, 
                    "O(2^n)", Difficulty::Hard)
    };
}

QList<QuizQuestion> ApiManager::parseApiResponse(const QJsonDocument& response)
{
    QList<QuizQuestion> questions;
    QJsonObject root = response.object();
    
    if (root["response_code"].toInt() == 0) {
        QJsonArray results = root["results"].toArray();
        
        for (const QJsonValue& value : results) {
            QJsonObject questionObj = value.toObject();
            
            QString question = questionObj["question"].toString();
            QString correctAnswer = questionObj["correct_answer"].toString();
            QString difficultyStr = questionObj["difficulty"].toString();
            
            QJsonArray incorrectAnswers = questionObj["incorrect_answers"].toArray();
            QStringList options;
            options.append(correctAnswer);
            
            for (const QJsonValue& incorrect : incorrectAnswers) {
                options.append(incorrect.toString());
            }
            
            // Shuffle options
            std::mt19937 rng(QRandomGenerator::global()->generate());
            std::shuffle(options.begin(), options.end(), rng);
            
            Difficulty difficulty = QuizQuestion::stringToDifficulty(difficultyStr);
            QuizQuestion q(question, options, correctAnswer, difficulty);
            questions.append(q);
        }
    }
    
    return questions;
}

void ApiManager::handleApiError(const QString& error)
{
    emit errorOccurred("API Error: " + error);
}

QList<QuizQuestion> ApiManager::selectQuestionsByDifficulty(const QList<QuizQuestion>& questions, 
                                                           Difficulty difficulty, int count)
{
    QList<QuizQuestion> filtered;
    
    for (const QuizQuestion& question : questions) {
        if (question.getDifficulty() == difficulty) {
            filtered.append(question);
        }
    }
    
    // Shuffle and limit
    filtered = shuffleQuestions(filtered);
    if (filtered.size() > count) {
        filtered = filtered.mid(0, count);
    }
    
    return filtered;
}

QList<QuizQuestion> ApiManager::shuffleQuestions(const QList<QuizQuestion>& questions)
{
    QList<QuizQuestion> shuffled = questions;
    std::mt19937 rng(QRandomGenerator::global()->generate());
    std::shuffle(shuffled.begin(), shuffled.end(), rng);
    return shuffled;
} 

QList<QuizQuestion> ApiManager::getLocalCurrentAffairsQuestions(int count)
{
    QList<QuizQuestion> currentAffairsQuestions = {
        QuizQuestion("Which country recently hosted the G20 summit?", 
                    {"India", "Brazil", "Indonesia", "Italy"}, 
                    "India", Difficulty::Easy),
        QuizQuestion("What is the main focus of COP28 climate conference?", 
                    {"Renewable Energy", "Carbon Reduction", "Climate Finance", "All of the above"}, 
                    "All of the above", Difficulty::Medium),
        QuizQuestion("Which technology company recently launched AI-powered search?", 
                    {"Google", "Microsoft", "OpenAI", "Meta"}, 
                    "Google", Difficulty::Easy),
        QuizQuestion("What is the primary goal of the Paris Agreement?", 
                    {"Limit global temperature rise", "Reduce fossil fuel use", "Increase renewable energy", "All of the above"}, 
                    "All of the above", Difficulty::Medium),
        QuizQuestion("Which country recently joined NATO?", 
                    {"Ukraine", "Finland", "Sweden", "Both Finland and Sweden"}, 
                    "Both Finland and Sweden", Difficulty::Medium),
        QuizQuestion("What is the main challenge in global supply chains?", 
                    {"Labor shortages", "Climate change", "Geopolitical tensions", "All of the above"}, 
                    "All of the above", Difficulty::Hard),
        QuizQuestion("Which sector is leading in AI adoption?", 
                    {"Healthcare", "Finance", "Technology", "Manufacturing"}, 
                    "Technology", Difficulty::Easy),
        QuizQuestion("What is the primary concern about social media platforms?", 
                    {"Privacy", "Misinformation", "Addiction", "All of the above"}, 
                    "All of the above", Difficulty::Medium),
        QuizQuestion("Which renewable energy source is growing fastest?", 
                    {"Solar", "Wind", "Hydroelectric", "Geothermal"}, 
                    "Solar", Difficulty::Easy),
        QuizQuestion("What is the main focus of digital transformation?", 
                    {"Automation", "Customer experience", "Data analytics", "All of the above"}, 
                    "All of the above", Difficulty::Hard)
    };
    
    // Shuffle and return requested count
    currentAffairsQuestions = shuffleQuestions(currentAffairsQuestions);
    if (currentAffairsQuestions.size() > count) {
        currentAffairsQuestions = currentAffairsQuestions.mid(0, count);
    }
    
    return currentAffairsQuestions;
} 