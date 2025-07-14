#include "quizmanager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDateTime>
#include <algorithm>
#include <QRandomGenerator>
#include <QDebug> // Added for qDebug

QuizManager::QuizManager(QObject *parent)
    : QObject(parent)
    , m_isActive(false)
    , m_isPaused(false)
    , m_currentIndex(0)
    , m_totalQuestions(0)
    , m_answeredCount(0)
    , m_correctCount(0)
    , m_remainingTime(0)
    , m_currentDifficulty(Difficulty::Easy)
    , m_consecutiveCorrect(0)
    , m_consecutiveWrong(0)
    , m_easyCorrect(0)
    , m_mediumCorrect(0)
    , m_hardCorrect(0)
    , m_initialTime(1800) // 30 minutes default
    , m_apiManager(new ApiManager(this))
{
    // Setup timer
    m_timer = new QTimer(this);
    m_timer->setInterval(1000); // 1 second
    connect(m_timer, &QTimer::timeout, this, &QuizManager::updateTimer);
    
    // Connect API signals
    connect(m_apiManager, &ApiManager::questionsFetched, 
            this, &QuizManager::onQuestionsFetched);
    connect(m_apiManager, &ApiManager::errorOccurred, 
            this, &QuizManager::onApiError);
    
    // Initialize analytics
    m_difficultyStats[Difficulty::Easy] = 0;
    m_difficultyStats[Difficulty::Medium] = 0;
    m_difficultyStats[Difficulty::Hard] = 0;
    
    m_correctByDifficulty[Difficulty::Easy] = 0;
    m_correctByDifficulty[Difficulty::Medium] = 0;
    m_correctByDifficulty[Difficulty::Hard] = 0;
}

QuizManager::~QuizManager()
{
}

void QuizManager::startQuiz(const QString& quizType)
{
    if (m_isActive) return;
    
    resetQuiz();
    
    if (quizType == "cpp_dsa") {
        m_questions = m_apiManager->getCppDSAQuestions();
    } else if (quizType == "custom") {
        // Custom quiz will be loaded separately
        return;
    } else {
        // Default: fetch from API
        m_apiManager->fetchQuestions(15);
        return;
    }
    
    // Initialize DSA structures for intelligent question selection
    m_totalQuestions = m_questions.size();
    m_timePerQuestion.resize(m_totalQuestions);
    initializeQuestionQueue();
    
    m_isActive = true;
    m_remainingTime = m_initialTime;
    m_timer->start();
    
    emit quizStarted();
    emit questionChanged(0);
    emit progressUpdated(0, m_totalQuestions);
}

void QuizManager::pauseQuiz()
{
    if (!m_isActive || m_isPaused) return;
    
    m_isPaused = true;
    m_timer->stop();
    emit quizPaused();
}

void QuizManager::resumeQuiz()
{
    if (!m_isActive || !m_isPaused) return;
    
    m_isPaused = false;
    m_timer->start();
    emit quizResumed();
}

void QuizManager::finishQuiz()
{
    if (!m_isActive) return;
    
    m_isActive = false;
    m_timer->stop();
    calculateStats();
    saveQuizResults();
    
    emit quizFinished();
}

void QuizManager::resetQuiz()
{
    m_isActive = false;
    m_isPaused = false;
    m_currentIndex = 0;
    m_answeredCount = 0;
    m_correctCount = 0;
    m_remainingTime = m_initialTime;
    m_consecutiveCorrect = 0;
    m_consecutiveWrong = 0;
    m_easyCorrect = 0;
    m_mediumCorrect = 0;
    m_hardCorrect = 0;
    
    m_questions.clear();
    m_questionHistory.clear();
    m_timePerQuestion.clear();
    
    // Reset analytics
    m_difficultyStats.clear();
    m_correctByDifficulty.clear();
    m_difficultyStats[Difficulty::Easy] = 0;
    m_difficultyStats[Difficulty::Medium] = 0;
    m_difficultyStats[Difficulty::Hard] = 0;
    m_correctByDifficulty[Difficulty::Easy] = 0;
    m_correctByDifficulty[Difficulty::Medium] = 0;
    m_correctByDifficulty[Difficulty::Hard] = 0;
}

void QuizManager::nextQuestion()
{
    qDebug() << "QuizManager::nextQuestion - Current index:" << m_currentIndex << "Total:" << m_totalQuestions;
    if (!m_isActive || m_currentIndex >= m_totalQuestions - 1) {
        qDebug() << "Cannot go to next question - quiz not active or at last question";
        return;
    }
    
    // Save current question time
    if (m_currentIndex < m_timePerQuestion.size()) {
        m_timePerQuestion[m_currentIndex] = m_initialTime - m_remainingTime;
    }
    
    m_questionHistory.push(m_currentIndex);
    m_currentIndex++;
    
    qDebug() << "Moving to question" << m_currentIndex;
    emit questionChanged(m_currentIndex);
    emit progressUpdated(m_answeredCount, m_totalQuestions);
}

void QuizManager::previousQuestion()
{
    qDebug() << "QuizManager::previousQuestion - Current index:" << m_currentIndex;
    if (!m_isActive || m_questionHistory.isEmpty()) {
        qDebug() << "Cannot go to previous question - quiz not active or no history";
        return;
    }
    
    m_currentIndex = m_questionHistory.pop();
    qDebug() << "Moving to question" << m_currentIndex;
    emit questionChanged(m_currentIndex);
    emit progressUpdated(m_answeredCount, m_totalQuestions);
}

void QuizManager::goToQuestion(int index)
{
    qDebug() << "QuizManager::goToQuestion - Index:" << index << "Total:" << m_totalQuestions;
    if (!m_isActive || index < 0 || index >= m_totalQuestions) {
        qDebug() << "Cannot go to question - invalid index or quiz not active";
        return;
    }
    
    m_currentIndex = index;
    qDebug() << "Moving to question" << m_currentIndex;
    emit questionChanged(m_currentIndex);
    emit progressUpdated(m_answeredCount, m_totalQuestions);
}

void QuizManager::answerQuestion(const QString& answer)
{
    if (!m_isActive || m_currentIndex >= m_questions.size()) return;
    
    QuizQuestion& currentQuestion = m_questions[m_currentIndex];
    currentQuestion.setUserAnswer(answer);
    currentQuestion.setTimeSpent(m_initialTime - m_remainingTime);
    
    bool isCorrect = currentQuestion.isCorrect();
    Difficulty questionDifficulty = currentQuestion.getDifficulty();
    
    // Update answered questions set (O(1) lookup)
    m_answeredQuestions.insert(m_currentIndex);
    
    if (isCorrect) {
        m_correctCount++;
        m_consecutiveCorrect++;
        m_consecutiveWrong = 0;
        
        // Track by difficulty
        switch (questionDifficulty) {
            case Difficulty::Easy: m_easyCorrect++; break;
            case Difficulty::Medium: m_mediumCorrect++; break;
            case Difficulty::Hard: m_hardCorrect++; break;
        }
    } else {
        m_consecutiveWrong++;
        m_consecutiveCorrect = 0;
    }
    
    m_answeredCount++;
    
    // Update performance metrics for DSA algorithms
    updatePerformanceMetrics(questionDifficulty, isCorrect, currentQuestion.getTimeSpent());
    
    // Update question weights for adaptive selection
    updateQuestionWeights();
    
    // Update streaks
    updateStreaks(isCorrect);
    
    // Update difficulty based on performance
    adjustDifficulty();
    
    emit answerSubmitted(m_currentIndex, isCorrect);
    emit progressUpdated(m_answeredCount, m_totalQuestions);
    emit streakUpdated(m_consecutiveCorrect, m_consecutiveWrong);
}

void QuizManager::clearAnswer()
{
    if (!m_isActive || m_currentIndex >= m_questions.size()) return;
    
    QuizQuestion& currentQuestion = m_questions[m_currentIndex];
    currentQuestion.setUserAnswer("");
    currentQuestion.setAnswered(false);
}

bool QuizManager::isQuizActive() const
{
    return m_isActive;
}

bool QuizManager::isQuizPaused() const
{
    return m_isPaused;
}

int QuizManager::getCurrentQuestionIndex() const
{
    return m_currentIndex;
}

int QuizManager::getTotalQuestions() const
{
    return m_totalQuestions;
}

int QuizManager::getAnsweredCount() const
{
    return m_answeredCount;
}

int QuizManager::getCorrectCount() const
{
    return m_correctCount;
}

int QuizManager::getRemainingTime() const
{
    return m_remainingTime;
}

QuizQuestion QuizManager::getCurrentQuestion() const
{
    if (m_currentIndex < m_questions.size()) {
        return m_questions[m_currentIndex];
    }
    return QuizQuestion();
}

QList<QuizQuestion> QuizManager::getAllQuestions() const
{
    return m_questions;
}

Difficulty QuizManager::getCurrentDifficulty() const
{
    return m_currentDifficulty;
}

int QuizManager::getConsecutiveCorrect() const
{
    return m_consecutiveCorrect;
}

int QuizManager::getConsecutiveWrong() const
{
    return m_consecutiveWrong;
}

void QuizManager::updateDifficulty()
{
    Difficulty newDifficulty = m_currentDifficulty;
    
    // Adaptive logic: 3 consecutive correct -> increase difficulty
    if (m_consecutiveCorrect >= 3) {
        switch (m_currentDifficulty) {
            case Difficulty::Easy:
                newDifficulty = Difficulty::Medium;
                break;
            case Difficulty::Medium:
                newDifficulty = Difficulty::Hard;
                break;
            case Difficulty::Hard:
                // Stay at hard
                break;
        }
        m_consecutiveCorrect = 0; // Reset streak
    }
    
    // 3 consecutive wrong -> decrease difficulty
    if (m_consecutiveWrong >= 3) {
        switch (m_currentDifficulty) {
            case Difficulty::Hard:
                newDifficulty = Difficulty::Medium;
                break;
            case Difficulty::Medium:
                newDifficulty = Difficulty::Easy;
                break;
            case Difficulty::Easy:
                // Stay at easy
                break;
        }
        m_consecutiveWrong = 0; // Reset streak
    }
    
    if (newDifficulty != m_currentDifficulty) {
        m_currentDifficulty = newDifficulty;
        emit difficultyChanged(newDifficulty);
    }
}

QMap<Difficulty, int> QuizManager::getDifficultyStats() const
{
    return m_difficultyStats;
}

QMap<Difficulty, double> QuizManager::getAccuracyByDifficulty() const
{
    QMap<Difficulty, double> accuracy;
    
    for (auto it = m_difficultyStats.begin(); it != m_difficultyStats.end(); ++it) {
        Difficulty diff = it.key();
        int total = it.value();
        int correct = m_correctByDifficulty.value(diff, 0);
        
        if (total > 0) {
            accuracy[diff] = static_cast<double>(correct) / total * 100.0;
        } else {
            accuracy[diff] = 0.0;
        }
    }
    
    return accuracy;
}

double QuizManager::getOverallAccuracy() const
{
    if (m_answeredCount == 0) return 0.0;
    return static_cast<double>(m_correctCount) / m_answeredCount * 100.0;
}

double QuizManager::getAverageTimePerQuestion() const
{
    if (m_timePerQuestion.isEmpty()) return 0.0;
    
    int totalTime = 0;
    for (int time : m_timePerQuestion) {
        totalTime += time;
    }
    
    return static_cast<double>(totalTime) / m_timePerQuestion.size();
}

QList<int> QuizManager::getTimePerQuestion() const
{
    return m_timePerQuestion;
}

void QuizManager::loadCustomQuiz(const QString& name)
{
    m_questions = m_apiManager->loadCustomQuiz(name);
    initializeQuestionQueue();
    m_isActive = true;
    m_remainingTime = m_initialTime;
    m_timer->start();
    
    emit quizStarted();
    emit questionChanged(0);
    emit progressUpdated(0, m_totalQuestions);
}

void QuizManager::createCustomQuiz(const QString& name, const QList<QuizQuestion>& questions)
{
    m_apiManager->saveCustomQuiz(name, questions);
}

void QuizManager::updateTimer()
{
    if (!m_isActive || m_isPaused) return;
    
    m_remainingTime--;
    emit timeUpdated(m_remainingTime);
    
    if (m_remainingTime <= 0) {
        finishQuiz();
    }
}

void QuizManager::onQuestionsFetched(const QList<QuizQuestion>& questions)
{
    m_questions = questions;
    m_totalQuestions = m_questions.size();
    m_timePerQuestion.resize(m_totalQuestions);
    
    // Initialize DSA structures for intelligent question selection
    initializeQuestionQueue();
    
    m_isActive = true;
    m_remainingTime = m_initialTime;
    m_timer->start();
    
    emit quizStarted();
    emit questionChanged(0);
    emit progressUpdated(0, m_totalQuestions);
}

void QuizManager::onApiError(const QString& error)
{
    // Handle API error gracefully - fallback to local questions
    // In a production environment, you might want to log this error
    m_questions = m_apiManager->getCppDSAQuestions();
    initializeQuestionQueue();
    m_isActive = true;
    m_remainingTime = m_initialTime;
    m_timer->start();
    
    emit quizStarted();
    emit questionChanged(0);
    emit progressUpdated(0, m_totalQuestions);
}

void QuizManager::loadQuizResults()
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataPath);
    QString filePath = dataPath + "/quiz_results.json";
    
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonObject results = doc.object();
        
        // Load analytics data
        if (results.contains("difficultyStats")) {
            QJsonObject stats = results["difficultyStats"].toObject();
            m_difficultyStats[Difficulty::Easy] = stats["easy"].toInt();
            m_difficultyStats[Difficulty::Medium] = stats["medium"].toInt();
            m_difficultyStats[Difficulty::Hard] = stats["hard"].toInt();
        }
    }
}

// ========== DSA ALGORITHMS FOR INTELLIGENT QUESTION SELECTION ==========

void QuizManager::buildQuestionPriorityQueue()
{
    m_questionQueue = std::priority_queue<QuestionPriority>(); // Clear by creating new empty queue
    m_questionsByDifficulty.clear();
    m_questionWeights.clear();
    
    // Categorize questions by difficulty using Hash Map (O(n))
    for (int i = 0; i < m_questions.size(); ++i) {
        Difficulty diff = m_questions[i].getDifficulty();
        m_questionsByDifficulty[diff].append(i);
        
        // Initialize weights using weighted random selection
        double baseWeight = 1.0;
        switch (diff) {
            case Difficulty::Easy: baseWeight = 1.0; break;
            case Difficulty::Medium: baseWeight = 1.5; break;
            case Difficulty::Hard: baseWeight = 2.0; break;
        }
        m_questionWeights[i] = baseWeight;
    }
    
    // Build priority queue using Heap Sort algorithm (O(n log n))
    for (int i = 0; i < m_questions.size(); ++i) {
        QuestionPriority qp;
        qp.questionIndex = i;
        qp.priority = m_questionWeights[i];
        qp.difficulty = m_questions[i].getDifficulty();
        qp.timeSpent = 0;
        qp.answered = false;
        
        m_questionQueue.push(qp);
    }
    
    // Initialize difficulty weights for adaptive selection
    m_difficultyWeights[Difficulty::Easy] = 0.4;
    m_difficultyWeights[Difficulty::Medium] = 0.4;
    m_difficultyWeights[Difficulty::Hard] = 0.2;
}

void QuizManager::updateQuestionWeights()
{
    // Update weights based on performance using Dynamic Programming approach
    for (int i = 0; i < m_questions.size(); ++i) {
        if (m_answeredQuestions.contains(i)) continue;
        
        double newWeight = m_questionWeights[i];
        Difficulty diff = m_questions[i].getDifficulty();
        
        // Performance-based weight adjustment
        double accuracy = m_accuracyByDifficulty.value(diff, 0.5);
        double avgTime = m_averageTimeByDifficulty.value(diff, 60.0);
        
        // Adaptive weight calculation using exponential backoff
        if (accuracy < 0.3) {
            newWeight *= 0.8; // Reduce weight for difficult questions
        } else if (accuracy > 0.7) {
            newWeight *= 1.2; // Increase weight for easier questions
        }
        
        // Time-based adjustment
        if (avgTime > 120) {
            newWeight *= 0.9; // Reduce weight for time-consuming questions
        }
        
        m_questionWeights[i] = newWeight;
    }
}

void QuizManager::adaptiveQuestionSelection()
{
    // Algorithm: Adaptive Selection using Multi-Criteria Decision Making
    
    if (m_questionQueue.empty()) {
        buildQuestionPriorityQueue();
    }
    
    // Update weights based on current performance
    updateQuestionWeights();
    
    // Select question using weighted random selection
    weightedRandomSelection();
}

void QuizManager::weightedRandomSelection()
{
    // Algorithm: Weighted Random Selection using Reservoir Sampling
    
    if (m_questionsByDifficulty.isEmpty()) {
        buildQuestionPriorityQueue();
    }
    
    // Calculate total weight
    double totalWeight = 0.0;
    QList<int> availableQuestions;
    
    for (int i = 0; i < m_questions.size(); ++i) {
        if (!m_answeredQuestions.contains(i)) {
            totalWeight += m_questionWeights[i];
            availableQuestions.append(i);
        }
    }
    
    if (availableQuestions.isEmpty()) return;
    
    // Reservoir sampling for weighted random selection
    double randomValue = QRandomGenerator::global()->bounded(totalWeight);
    double cumulativeWeight = 0.0;
    int selectedIndex = -1;
    
    for (int questionIndex : availableQuestions) {
        cumulativeWeight += m_questionWeights[questionIndex];
        if (cumulativeWeight >= randomValue) {
            selectedIndex = questionIndex;
            break;
        }
    }
    
    if (selectedIndex != -1) {
        m_currentIndex = selectedIndex;
        emit questionChanged(m_currentIndex);
    }
}

void QuizManager::difficultyBasedSelection()
{
    // Algorithm: Difficulty-Based Selection using Priority Queue
    
    // Determine target difficulty based on performance
    Difficulty targetDifficulty = m_currentDifficulty;
    
    // Adaptive difficulty adjustment using sliding window
    if (m_consecutiveCorrect >= 3) {
        // Increase difficulty
        switch (targetDifficulty) {
            case Difficulty::Easy: targetDifficulty = Difficulty::Medium; break;
            case Difficulty::Medium: targetDifficulty = Difficulty::Hard; break;
            case Difficulty::Hard: targetDifficulty = Difficulty::Hard; break;
        }
    } else if (m_consecutiveWrong >= 2) {
        // Decrease difficulty
        switch (targetDifficulty) {
            case Difficulty::Easy: targetDifficulty = Difficulty::Easy; break;
            case Difficulty::Medium: targetDifficulty = Difficulty::Easy; break;
            case Difficulty::Hard: targetDifficulty = Difficulty::Medium; break;
        }
    }
    
    // Select question from target difficulty using Binary Search
    QList<int>& questions = m_questionsByDifficulty[targetDifficulty];
    if (!questions.isEmpty()) {
        // Find unasked question using Set lookup (O(1))
        for (int questionIndex : questions) {
            if (!m_answeredQuestions.contains(questionIndex)) {
                m_currentIndex = questionIndex;
                emit questionChanged(m_currentIndex);
                return;
            }
        }
    }
    
    // Fallback to weighted selection
    weightedRandomSelection();
}

void QuizManager::performanceBasedSelection()
{
    // Algorithm: Performance-Based Selection using Dynamic Programming
    
    // Calculate performance metrics
    QMap<Difficulty, double> performanceScores;
    
    for (auto it = m_accuracyByDifficulty.begin(); it != m_accuracyByDifficulty.end(); ++it) {
        Difficulty diff = it.key();
        double accuracy = it.value();
        double avgTime = m_averageTimeByDifficulty.value(diff, 60.0);
        
        // Performance score calculation using weighted average
        double timeScore = 1.0 / (1.0 + avgTime / 60.0); // Normalize time
        double performanceScore = 0.7 * accuracy + 0.3 * timeScore;
        performanceScores[diff] = performanceScore;
    }
    
    // Select difficulty with best performance score
    Difficulty bestDifficulty = Difficulty::Easy;
    double bestScore = performanceScores.value(Difficulty::Easy, 0.0);
    
    for (auto it = performanceScores.begin(); it != performanceScores.end(); ++it) {
        if (it.value() > bestScore) {
            bestScore = it.value();
            bestDifficulty = it.key();
        }
    }
    
    // Select question from best performing difficulty
    QList<int>& questions = m_questionsByDifficulty[bestDifficulty];
    for (int questionIndex : questions) {
        if (!m_answeredQuestions.contains(questionIndex)) {
            m_currentIndex = questionIndex;
            emit questionChanged(m_currentIndex);
            return;
        }
    }
    
    // Fallback
    weightedRandomSelection();
}

void QuizManager::timeBasedSelection()
{
    // Algorithm: Time-Based Selection using Priority Queue
    
    // Calculate time-based priorities
    QList<QuestionPriority> timePriorities;
    
    for (int i = 0; i < m_questions.size(); ++i) {
        if (m_answeredQuestions.contains(i)) continue;
        
        QuestionPriority qp;
        qp.questionIndex = i;
        qp.difficulty = m_questions[i].getDifficulty();
        qp.timeSpent = 0;
        qp.answered = false;
        
        // Time-based priority calculation
        double avgTime = m_averageTimeByDifficulty.value(qp.difficulty, 60.0);
        double timePriority = 1.0 / (1.0 + avgTime / 60.0);
        
        // Combine with difficulty weight
        qp.priority = timePriority * m_difficultyWeights.value(qp.difficulty, 1.0);
        
        timePriorities.append(qp);
    }
    
    if (!timePriorities.isEmpty()) {
        // Sort by priority (Heap Sort)
        std::sort(timePriorities.begin(), timePriorities.end());
        
        // Select highest priority question
        m_currentIndex = timePriorities.last().questionIndex;
        emit questionChanged(m_currentIndex);
    }
}

void QuizManager::selectNextQuestion()
{
    // Main selection algorithm using multiple DSA approaches
    
    // Update performance metrics
    updateStreaks(false); // Update without changing streaks
    
    // Choose selection strategy based on quiz progress
    int progress = (m_answeredCount * 100) / m_totalQuestions;
    
    if (progress < 30) {
        // Early stage: Use difficulty-based selection
        difficultyBasedSelection();
    } else if (progress < 70) {
        // Middle stage: Use adaptive selection
        adaptiveQuestionSelection();
    } else {
        // Late stage: Use performance-based selection
        performanceBasedSelection();
    }
}

void QuizManager::initializeQuestionQueue()
{
    // Initialize all DSA structures
    buildQuestionPriorityQueue();
    
    // Set initial difficulty weights
    m_difficultyWeights[Difficulty::Easy] = 0.4;
    m_difficultyWeights[Difficulty::Medium] = 0.4;
    m_difficultyWeights[Difficulty::Hard] = 0.2;
    
    // Initialize performance tracking
    m_accuracyByDifficulty[Difficulty::Easy] = 0.5;
    m_accuracyByDifficulty[Difficulty::Medium] = 0.5;
    m_accuracyByDifficulty[Difficulty::Hard] = 0.5;
    
    m_averageTimeByDifficulty[Difficulty::Easy] = 60.0;
    m_averageTimeByDifficulty[Difficulty::Medium] = 90.0;
    m_averageTimeByDifficulty[Difficulty::Hard] = 120.0;
    
    m_attemptsByDifficulty[Difficulty::Easy] = 0;
    m_attemptsByDifficulty[Difficulty::Medium] = 0;
    m_attemptsByDifficulty[Difficulty::Hard] = 0;
}

void QuizManager::updatePerformanceMetrics(Difficulty difficulty, bool isCorrect, int timeSpent)
{
    // Update difficulty statistics
    m_difficultyStats[difficulty]++;
    m_attemptsByDifficulty[difficulty]++;
    
    if (isCorrect) {
        m_correctByDifficulty[difficulty]++;
    }
    
    // Calculate accuracy by difficulty using rolling average
    int attempts = m_attemptsByDifficulty[difficulty];
    int correct = m_correctByDifficulty[difficulty];
    
    if (attempts > 0) {
        double accuracy = static_cast<double>(correct) / attempts;
        m_accuracyByDifficulty[difficulty] = accuracy;
    }
    
    // Update average time by difficulty using exponential moving average
    double currentAvgTime = m_averageTimeByDifficulty.value(difficulty, 60.0);
    double alpha = 0.3; // Smoothing factor
    double newAvgTime = alpha * timeSpent + (1 - alpha) * currentAvgTime;
    m_averageTimeByDifficulty[difficulty] = newAvgTime;
}

void QuizManager::adjustDifficulty()
{
    updateDifficulty();
    
    // After difficulty change, we could select the next question from the new difficulty
    // This would make the adaptive system more responsive
    // For now, we'll keep the current approach but this is where we'd implement
    // dynamic question selection based on the new difficulty
}

void QuizManager::updateStreaks(bool isCorrect)
{
    // Streaks are already updated in answerQuestion
    // This method can be used for additional streak-based logic
}

QList<QuizQuestion> QuizManager::getQuestionsForDifficulty(Difficulty difficulty)
{
    QList<QuizQuestion> filtered;
    for (const QuizQuestion& question : m_questions) {
        if (question.getDifficulty() == difficulty) {
            filtered.append(question);
        }
    }
    return filtered;
}

void QuizManager::calculateStats()
{
    // Stats are calculated in real-time
    // This method can be used for final calculations
}

void QuizManager::saveQuizResults()
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    QString filePath = dir.filePath("quiz_results.json");
    QFile file(filePath);
    
    if (file.open(QIODevice::WriteOnly)) {
        QJsonObject results;
        results["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        results["totalQuestions"] = m_totalQuestions;
        results["answeredCount"] = m_answeredCount;
        results["correctCount"] = m_correctCount;
        results["overallAccuracy"] = getOverallAccuracy();
        results["averageTime"] = getAverageTimePerQuestion();
        
        QJsonArray questionsArray;
        for (const QuizQuestion& question : m_questions) {
            questionsArray.append(question.toJson());
        }
        results["questions"] = questionsArray;
        
        QJsonDocument doc(results);
        file.write(doc.toJson());
        file.close();
    }
} 