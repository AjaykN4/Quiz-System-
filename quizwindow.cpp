#include "quizwindow.h"
#include <QApplication>
#include <QDebug> // Added for qDebug()
#include <QTimer> // Added for QTimer

QuizWindow::QuizWindow(QuizManager* quizManager, QWidget *parent)
    : QWidget(parent)
    , m_quizManager(quizManager)
    , m_currentQuestionIndex(0)
    , m_isAnswered(false)
    , m_flickerState(false)
    , m_correctAnswerButton(nullptr)
    , m_spiceShown(false)
{
    setupUI();
    setupConnections();
    setupSound();
    
    // Initialize flicker timer
    m_flickerTimer = new QTimer(this);
    m_flickerTimer->setInterval(500); // 500ms interval for flickering
    connect(m_flickerTimer, &QTimer::timeout, this, &QuizWindow::onFlickerTimer);
    
    // Initialize spice it up timer
    m_spiceTimer = new QTimer(this);
    m_spiceTimer->setInterval(3000); // 3 seconds display
    connect(m_spiceTimer, &QTimer::timeout, this, &QuizWindow::onSpiceTimer);
    
    // Apply light theme
    setStyleSheet(
        "QuizWindow {"
        "    background-color: #f8f9fa;"
        "    color: #2c3e50;"
        "}"
        "QLabel {"
        "    color: #2c3e50;"
        "    font-size: 16px;"
        "}"
        "QRadioButton {"
        "    color: #2c3e50;"
        "    background-color: transparent;"
        "    font-size: 16px;"
        "    padding: 8px;"
        "}"
        "QRadioButton::indicator {"
        "    border: 2px solid #3498db;"
        "    background: white;"
        "    border-radius: 10px;"
        "    width: 16px;"
        "    height: 16px;"
        "}"
        "QRadioButton::indicator:checked {"
        "    background: #3498db;"
        "    border: 2px solid #3498db;"
        "}"
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 2px solid #bdc3c7;"
        "    border-radius: 5px;"
        "    margin-top: 1ex;"
        "    padding-top: 10px;"
        "    background-color: white;"
        "    color: #2c3e50;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    padding: 0 5px 0 5px;"
        "    color: #2c3e50;"
        "}"
        "QListWidget {"
        "    background-color: white;"
        "    color: #2c3e50;"
        "    border: 2px solid #bdc3c7;"
        "    border-radius: 5px;"
        "}"
        "QListWidget::item {"
        "    border: 2px solid #bdc3c7;"
        "    border-radius: 5px;"
        "    padding: 5px;"
        "    margin: 2px;"
        "    background-color: #ecf0f1;"
        "    color: #2c3e50;"
        "}"
        "QListWidget::item:selected {"
        "    border-color: #3498db;"
        "    background-color: #3498db;"
        "    color: white;"
        "}"
    );
}

QuizWindow::~QuizWindow()
{
}

void QuizWindow::setupUI()
{
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setSpacing(20);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    
    setupLeftPanel();
    setupRightPanel();
    
    // Add panels to main layout
    m_mainLayout->addLayout(m_leftPanelLayout, 2); // 2:1 ratio
    m_mainLayout->addLayout(m_rightPanelLayout, 1);
    
    // Setup spice it up overlay
    setupSpiceItUpOverlay();
}

void QuizWindow::setupLeftPanel()
{
    m_leftPanelLayout = new QVBoxLayout();
    m_leftPanelLayout->setSpacing(20);
    
    // Question display
    QGroupBox* questionGroup = new QGroupBox("Question");
    QVBoxLayout* questionLayout = new QVBoxLayout(questionGroup);
    
    m_questionLabel = new QLabel();
    m_questionLabel->setWordWrap(true);
    m_questionLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    QFont questionFont = m_questionLabel->font();
    questionFont.setPointSize(14);
    questionFont.setBold(true);
    m_questionLabel->setFont(questionFont);
    m_questionLabel->setStyleSheet("QLabel { color: #2c3e50; padding: 10px; }");
    questionLayout->addWidget(m_questionLabel);
    
    // Options
    m_optionsGroup = new QButtonGroup(this);
    for (int i = 0; i < 4; ++i) {
        QRadioButton* optionButton = new QRadioButton();
        optionButton->setStyleSheet(
            "QRadioButton {"
            "    font-size: 12px;"
            "    padding: 10px;"
            "    margin: 5px;"
            "    border: 2px solid #bdc3c7;"
            "    border-radius: 5px;"
            "    background-color: white;"
            "}"
            "QRadioButton:hover {"
            "    border-color: #3498db;"
            "    background-color: #ecf0f1;"
            "}"
            "QRadioButton:checked {"
            "    border-color: #3498db;"
            "    background-color: #3498db;"
            "    color: white;"
            "}"
        );
        m_optionButtons.append(optionButton);
        m_optionsGroup->addButton(optionButton, i);
        questionLayout->addWidget(optionButton);
    }
    
    // Navigation buttons
    QHBoxLayout* navLayout = new QHBoxLayout();
    
    m_previousButton = new QPushButton("← Previous");
    m_clearButton = new QPushButton("Clear");
    m_pauseButton = new QPushButton("⏸️ Pause");
    m_resumeButton = new QPushButton("▶️ Resume");
    m_nextButton = new QPushButton("Next →");
    m_finishButton = new QPushButton("Finish Quiz");
    
    QString buttonStyle = 
        "QPushButton {"
        "    background-color: #3498db;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 5px;"
        "    padding: 10px 20px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2980b9;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #bdc3c7;"
        "}";
    
    m_previousButton->setStyleSheet(buttonStyle);
    m_clearButton->setStyleSheet(buttonStyle);
    m_pauseButton->setStyleSheet(buttonStyle);
    m_resumeButton->setStyleSheet(buttonStyle);
    m_nextButton->setStyleSheet(buttonStyle);
    m_finishButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #e74c3c;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 5px;"
        "    padding: 10px 20px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #c0392b;"
        "}"
    );
    
    // Initially hide resume button
    m_resumeButton->setVisible(false);
    
    navLayout->addWidget(m_previousButton);
    navLayout->addWidget(m_clearButton);
    navLayout->addWidget(m_pauseButton);
    navLayout->addWidget(m_resumeButton);
    navLayout->addStretch();
    navLayout->addWidget(m_nextButton);
    navLayout->addWidget(m_finishButton);
    
    // Add all components to left panel
    m_leftPanelLayout->addWidget(questionGroup);
    m_leftPanelLayout->addLayout(navLayout);
    m_leftPanelLayout->addStretch();
}

void QuizWindow::setupRightPanel()
{
    m_rightPanelLayout = new QVBoxLayout();
    m_rightPanelLayout->setSpacing(15);
    
    // Timer
    m_timerWidget = new TimerWidget();
    m_rightPanelLayout->addWidget(m_timerWidget);
    
    // Progress
    QGroupBox* progressGroup = new QGroupBox("Progress");
    QVBoxLayout* progressLayout = new QVBoxLayout(progressGroup);
    
    m_progressLabel = new QLabel("0 / 0 questions answered");
    m_progressBar = new QProgressBar();
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setStyleSheet(
        "QProgressBar {"
        "    border: 2px solid #bdc3c7;"
        "    border-radius: 5px;"
        "    background-color: #ecf0f1;"
        "}"
        "QProgressBar::chunk {"
        "    background-color: #27ae60;"
        "    border-radius: 3px;"
        "}"
    );
    
    progressLayout->addWidget(m_progressLabel);
    progressLayout->addWidget(m_progressBar);
    m_rightPanelLayout->addWidget(progressGroup);
    
    // Difficulty and Streak
    QGroupBox* statsGroup = new QGroupBox("Stats");
    QVBoxLayout* statsLayout = new QVBoxLayout(statsGroup);
    
    m_difficultyLabel = new QLabel("Difficulty: Easy");
    m_streakLabel = new QLabel("Streak: 0 correct, 0 wrong");
    
    statsLayout->addWidget(m_difficultyLabel);
    statsLayout->addWidget(m_streakLabel);
    m_rightPanelLayout->addWidget(statsGroup);
    
    // Question Grid
    QGroupBox* gridGroup = new QGroupBox("Question Grid");
    QVBoxLayout* gridLayout = new QVBoxLayout(gridGroup);
    
    m_questionGrid = new QListWidget();
    m_questionGrid->setViewMode(QListView::IconMode);
    m_questionGrid->setIconSize(QSize(40, 40));
    m_questionGrid->setSpacing(5);
    m_questionGrid->setResizeMode(QListView::Adjust);
    m_questionGrid->setStyleSheet(
        "QListWidget {"
        "    background-color: white;"
        "    border: 2px solid #bdc3c7;"
        "    border-radius: 5px;"
        "}"
        "QListWidget::item {"
        "    border: 2px solid #bdc3c7;"
        "    border-radius: 5px;"
        "    padding: 5px;"
        "    margin: 2px;"
        "    background-color: #ecf0f1;"
        "}"
        "QListWidget::item:selected {"
        "    border-color: #3498db;"
        "    background-color: #3498db;"
        "    color: white;"
        "}"
    );
    
    gridLayout->addWidget(m_questionGrid);
    m_rightPanelLayout->addWidget(gridGroup);
    
    // Summary
    QGroupBox* summaryGroup = new QGroupBox("Summary");
    QVBoxLayout* summaryLayout = new QVBoxLayout(summaryGroup);
    
    m_summaryLabel = new QLabel("Ready to start!");
    m_summaryLabel->setWordWrap(true);
    m_summaryLabel->setStyleSheet("QLabel { color: #2c3e50; padding: 5px; }");
    
    summaryLayout->addWidget(m_summaryLabel);
    m_rightPanelLayout->addWidget(summaryGroup);
    
    // Style right panel
    m_rightPanelLayout->setStretchFactor(progressGroup, 0);
    m_rightPanelLayout->setStretchFactor(statsGroup, 0);
    m_rightPanelLayout->setStretchFactor(gridGroup, 1);
    m_rightPanelLayout->setStretchFactor(summaryGroup, 0);
}

void QuizWindow::setupConnections()
{
    // Quiz manager connections
    connect(m_quizManager, &QuizManager::questionChanged, this, &QuizWindow::onQuestionChanged);
    connect(m_quizManager, &QuizManager::answerSubmitted, this, &QuizWindow::onAnswerSubmitted);
    connect(m_quizManager, &QuizManager::timeUpdated, this, &QuizWindow::onTimeUpdated);
    connect(m_quizManager, &QuizManager::difficultyChanged, this, &QuizWindow::onDifficultyChanged);
    connect(m_quizManager, &QuizManager::progressUpdated, this, &QuizWindow::onProgressUpdated);
    connect(m_quizManager, &QuizManager::streakUpdated, this, &QuizWindow::onStreakUpdated);
    connect(m_quizManager, &QuizManager::quizFinished, this, &QuizWindow::onQuizFinished);
    
    // Navigation connections
    connect(m_nextButton, &QPushButton::clicked, this, &QuizWindow::nextQuestion);
    connect(m_previousButton, &QPushButton::clicked, this, &QuizWindow::previousQuestion);
    connect(m_clearButton, &QPushButton::clicked, this, &QuizWindow::clearAnswer);
    connect(m_pauseButton, &QPushButton::clicked, this, &QuizWindow::pauseQuiz);
    connect(m_resumeButton, &QPushButton::clicked, this, &QuizWindow::resumeQuiz);
    connect(m_finishButton, &QPushButton::clicked, this, &QuizWindow::finishQuiz);
    
    // Quiz manager pause/resume signals
    connect(m_quizManager, &QuizManager::quizPaused, this, &QuizWindow::onQuizPaused);
    connect(m_quizManager, &QuizManager::quizResumed, this, &QuizWindow::onQuizResumed);
    
    // Timer connections
    connect(m_timerWidget, &TimerWidget::timeUp, this, &QuizWindow::onTimeUp);
    
    // Question grid connections
    connect(m_questionGrid, &QListWidget::itemClicked, [this](QListWidgetItem* item) {
        int index = item->data(Qt::UserRole).toInt();
        goToQuestion(index);
    });
    
    // Option selection
    connect(m_optionsGroup, static_cast<void(QButtonGroup::*)(QAbstractButton*)>(&QButtonGroup::buttonClicked),
            [this](QAbstractButton* button) {
                int id = m_optionsGroup->id(button);
                if (id >= 0 && id < m_optionButtons.size()) {
                    QString answer = m_optionButtons[id]->text();
                    m_quizManager->answerQuestion(answer);
                }
            });
}

void QuizWindow::setupSpiceItUpOverlay()
{
    // Create overlay widget
    m_spiceOverlay = new QWidget(this);
    m_spiceOverlay->setFixedSize(400, 200);
    m_spiceOverlay->setStyleSheet(
        "QWidget {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "        stop:0 #ff6b6b, stop:0.2 #ffa726, stop:0.4 #ffeb3b,"
        "        stop:0.6 #4caf50, stop:0.8 #2196f3, stop:1 #9c27b0);"
        "    border: 3px solid #ffffff;"
        "    border-radius: 20px;"
        "    box-shadow: 0 8px 32px rgba(0,0,0,0.3);"
        "}"
    );
    
    // Create label with animated text
    m_spiceLabel = new QLabel("Letsss Spice it upppp", m_spiceOverlay);
    m_spiceLabel->setAlignment(Qt::AlignCenter);
    m_spiceLabel->setStyleSheet(
        "QLabel {"
        "    color: white;"
        "    font-size: 28px;"
        "    font-weight: bold;"
        "    text-shadow: 2px 2px 4px rgba(0,0,0,0.5);"
        "    background: transparent;"
        "    border: none;"
        "}"
    );
    
    // Center the label in the overlay
    QVBoxLayout* overlayLayout = new QVBoxLayout(m_spiceOverlay);
    overlayLayout->addWidget(m_spiceLabel);
    overlayLayout->setAlignment(Qt::AlignCenter);
    
    // Initially hide the overlay
    m_spiceOverlay->hide();
    
    // Position overlay in center of window
    m_spiceOverlay->move((width() - m_spiceOverlay->width()) / 2,
                         (height() - m_spiceOverlay->height()) / 2);
}

void QuizWindow::onQuestionChanged(int index)
{
    // Stop any ongoing flickering effect
    stopFlickerEffect();
    
    m_currentQuestionIndex = index;
    updateQuestionDisplay();
    updateQuestionGrid();
    updateProgress();
    updateSummary();
}

void QuizWindow::onAnswerSubmitted(int index, bool correct)
{
    m_isAnswered = true;
    showQuestionStatus(index, true, correct);
    updateProgress();
    updateSummary();
    
    // Play sound for wrong answers
    if (!correct) {
        playWrongSound();
    }
    
    // Update option button styles with immediate feedback
    updateOptionButtonStyles();
    
    // Start flickering effect for correct answer if user answered wrong
    if (!correct) {
        QuizQuestion currentQuestion = m_quizManager->getCurrentQuestion();
        QString correctAnswer = currentQuestion.getCorrectAnswer();
        
        // Find the correct answer button
        for (QRadioButton* button : m_optionButtons) {
            if (button->text() == correctAnswer) {
                m_correctAnswerButton = button;
                startFlickerEffect(button);
                break;
            }
        }
    }
    
    // Update navigation buttons
    m_nextButton->setEnabled(index < m_quizManager->getTotalQuestions() - 1);
    m_previousButton->setEnabled(index > 0);
}

void QuizWindow::onTimeUpdated(int remainingSeconds)
{
    m_timerWidget->setTime(remainingSeconds);
}

void QuizWindow::onDifficultyChanged(Difficulty newDifficulty)
{
    updateDifficultyDisplay();
}

void QuizWindow::onProgressUpdated(int answered, int total)
{
    updateProgress();
}

void QuizWindow::onStreakUpdated(int consecutiveCorrect, int consecutiveWrong)
{
    updateStreakDisplay();
    
    // Show spice it up overlay after 3 consecutive correct answers
    if (consecutiveCorrect == 3 && !m_spiceShown) {
        showSpiceItUp();
    }
}

void QuizWindow::onQuizFinished()
{
    // Quiz finished - let MainWindow handle the popup and navigation
    // No popup here to avoid duplicates
}

void QuizWindow::nextQuestion()
{
    qDebug() << "Next question clicked. Current index:" << m_currentQuestionIndex;
    qDebug() << "Total questions:" << m_quizManager->getTotalQuestions();
    m_quizManager->nextQuestion();
}

void QuizWindow::previousQuestion()
{
    qDebug() << "Previous question clicked. Current index:" << m_currentQuestionIndex;
    m_quizManager->previousQuestion();
}

void QuizWindow::finishQuiz()
{
    m_quizManager->finishQuiz();
}

void QuizWindow::pauseQuiz()
{
    m_quizManager->pauseQuiz();
}

void QuizWindow::resumeQuiz()
{
    m_quizManager->resumeQuiz();
}

void QuizWindow::onQuizPaused()
{
    m_pauseButton->setVisible(false);
    m_resumeButton->setVisible(true);
    m_timerWidget->setPaused(true);
}

void QuizWindow::onQuizResumed()
{
    m_pauseButton->setVisible(true);
    m_resumeButton->setVisible(false);
    m_timerWidget->setPaused(false);
}

void QuizWindow::clearAnswer()
{
    m_quizManager->clearAnswer();
    m_isAnswered = false;
    // Clear radio button selection
    m_optionsGroup->setExclusive(false);
    for (QRadioButton* button : m_optionButtons) {
        button->setChecked(false);
    }
    m_optionsGroup->setExclusive(true);
    resetOptionButtonStyles();
}

void QuizWindow::goToQuestion(int index)
{
    qDebug() << "Going to question" << index;
    m_quizManager->goToQuestion(index);
}

void QuizWindow::onTimeUp()
{
    QMessageBox::warning(this, "Time's Up!", "Time has expired. Quiz will be finished automatically.");
    finishQuiz();
}

void QuizWindow::updateQuestionDisplay()
{
    QuizQuestion currentQuestion = m_quizManager->getCurrentQuestion();
    
    if (currentQuestion.getQuestion().isEmpty()) {
        m_questionLabel->setText("Loading question...");
        return;
    }
    
    // Update question text
    QString questionText = QString("Question %1 of %2\n\n%3")
                          .arg(m_currentQuestionIndex + 1)
                          .arg(m_quizManager->getTotalQuestions())
                          .arg(currentQuestion.getQuestion());
    m_questionLabel->setText(questionText);
    
    // Clear radio button selection first
    m_optionsGroup->setExclusive(false);
    for (QRadioButton* button : m_optionButtons) {
        button->setChecked(false);
    }
    m_optionsGroup->setExclusive(true);
    
    // Update options
    QStringList options = currentQuestion.getOptions();
    for (int i = 0; i < m_optionButtons.size(); ++i) {
        if (i < options.size()) {
            m_optionButtons[i]->setText(options[i]);
            m_optionButtons[i]->setVisible(true);
        } else {
            m_optionButtons[i]->setVisible(false);
        }
    }
    
    // Restore selection if answered
    if (currentQuestion.isAnswered()) {
        for (QRadioButton* button : m_optionButtons) {
            if (button->text() == currentQuestion.getUserAnswer()) {
                button->setChecked(true);
            }
        }
    }
    
    // Update feedback styles
    updateOptionButtonStyles();
    
    // Update navigation buttons
    m_nextButton->setEnabled(m_currentQuestionIndex < m_quizManager->getTotalQuestions() - 1);
    m_previousButton->setEnabled(m_currentQuestionIndex > 0);
}

void QuizWindow::updateQuestionGrid()
{
    m_questionGrid->clear();
    
    for (int i = 0; i < m_quizManager->getTotalQuestions(); ++i) {
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(QString::number(i + 1));
        item->setData(Qt::UserRole, i);
        
        // Set icon based on status
        bool answered = false;
        bool correct = false;
        
        if (i < m_quizManager->getAllQuestions().size()) {
            const QuizQuestion& question = m_quizManager->getAllQuestions()[i];
            answered = question.isAnswered();
            correct = question.isCorrect();
        }
        
        QString statusColor = getStatusColor(answered, correct);
        item->setBackground(QColor(statusColor));
        
        if (i == m_currentQuestionIndex) {
            item->setBackground(QColor("#3498db"));
            item->setForeground(QColor("white"));
        }
        
        m_questionGrid->addItem(item);
    }
}

void QuizWindow::updateProgress()
{
    int answered = m_quizManager->getAnsweredCount();
    int total = m_quizManager->getTotalQuestions();
    
    m_progressLabel->setText(QString("%1 / %2 questions answered").arg(answered).arg(total));
    m_progressBar->setValue(total > 0 ? (answered * 100) / total : 0);
}

void QuizWindow::updateSummary()
{
    int correct = m_quizManager->getCorrectCount();
    int total = m_quizManager->getTotalQuestions();
    double accuracy = m_quizManager->getOverallAccuracy();
    
    QString summary = QString("Correct: %1/%2 (%3%)\nAccuracy: %4%")
                     .arg(correct)
                     .arg(total)
                     .arg(total > 0 ? (correct * 100) / total : 0)
                     .arg(accuracy, 0, 'f', 1);
    
    m_summaryLabel->setText(summary);
}

void QuizWindow::updateStreakDisplay()
{
    int consecutiveCorrect = m_quizManager->getConsecutiveCorrect();
    int consecutiveWrong = m_quizManager->getConsecutiveWrong();
    
    m_streakLabel->setText(QString("Streak: %1 correct, %2 wrong")
                           .arg(consecutiveCorrect)
                           .arg(consecutiveWrong));
}

void QuizWindow::updateDifficultyDisplay()
{
    Difficulty currentDifficulty = m_quizManager->getCurrentDifficulty();
    QString difficultyText = QString("Difficulty: %1")
                            .arg(currentDifficulty == Difficulty::Easy ? "Easy" :
                                 currentDifficulty == Difficulty::Medium ? "Medium" : "Hard");
    
    QString color = getDifficultyColor(currentDifficulty);
    m_difficultyLabel->setStyleSheet(QString("QLabel { color: %1; font-weight: bold; }").arg(color));
    m_difficultyLabel->setText(difficultyText);
}

void QuizWindow::showQuestionStatus(int index, bool answered, bool correct)
{
    // Update the question grid item
    if (index < m_questionGrid->count()) {
        QListWidgetItem* item = m_questionGrid->item(index);
        QString statusColor = getStatusColor(answered, correct);
        item->setBackground(QColor(statusColor));
    }
}

void QuizWindow::showFlawlessScreen()
{
    if (m_quizManager->getCorrectCount() == 10) {
        QMessageBox::information(this, "Flawless!", 
                               "Congratulations! You got 10 correct answers in a row!");
    }
}

QString QuizWindow::formatTime(int seconds) const
{
    int minutes = seconds / 60;
    int remainingSeconds = seconds % 60;
    return QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(remainingSeconds, 2, 10, QChar('0'));
}

QString QuizWindow::getDifficultyColor(Difficulty difficulty) const
{
    switch (difficulty) {
        case Difficulty::Easy: return "#3498db";    // Blue
        case Difficulty::Medium: return "#f1c40f";  // Yellow
        case Difficulty::Hard: return "#e74c3c";    // Red
        default: return "#2c3e50";
    }
}

QString QuizWindow::getStatusColor(bool answered, bool correct) const
{
    if (!answered) return "#ecf0f1";  // Light gray for unanswered
    return correct ? "#3498db" : "#e74c3c";  // Blue for correct, Red for incorrect
} 

void QuizWindow::updateOptionButtonStyles()
{
    QuizQuestion currentQuestion = m_quizManager->getCurrentQuestion();
    QString correctAnswer = currentQuestion.getCorrectAnswer();
    QString userAnswer = currentQuestion.getUserAnswer();
    bool answered = currentQuestion.isAnswered();

    for (QRadioButton* button : m_optionButtons) {
        QString optionText = button->text();
        
        if (answered) {
            if (optionText == correctAnswer) {
                // Correct answer: green
                button->setStyleSheet(
                    "QRadioButton {"
                    "    font-size: 12px;"
                    "    padding: 10px;"
                    "    margin: 5px;"
                    "    border: 2px solid #27ae60;"
                    "    border-radius: 5px;"
                    "    background-color: #27ae60;"
                    "    color: white;"
                    "}"
                    "QRadioButton:hover {"
                    "    border-color: #27ae60;"
                    "    background-color: #27ae60;"
                    "}"
                    "QRadioButton:checked {"
                    "    border-color: #27ae60;"
                    "    background-color: #27ae60;"
                    "    color: white;"
                    "}"
                );
            } else if (optionText == userAnswer && userAnswer != correctAnswer) {
                // Wrong selected answer: red
                button->setStyleSheet(
                    "QRadioButton {"
                    "    font-size: 12px;"
                    "    padding: 10px;"
                    "    margin: 5px;"
                    "    border: 2px solid #e74c3c;"
                    "    border-radius: 5px;"
                    "    background-color: #e74c3c;"
                    "    color: white;"
                    "}"
                    "QRadioButton:hover {"
                    "    border-color: #e74c3c;"
                    "    background-color: #e74c3c;"
                    "}"
                    "QRadioButton:checked {"
                    "    border-color: #e74c3c;"
                    "    background-color: #e74c3c;"
                    "    color: white;"
                    "}"
                );
            } else {
                // Unselected options: neutral
                button->setStyleSheet(
                    "QRadioButton {"
                    "    font-size: 12px;"
                    "    padding: 10px;"
                    "    margin: 5px;"
                    "    border: 2px solid #bdc3c7;"
                    "    border-radius: 5px;"
                    "    background-color: white;"
                    "    color: #2c3e50;"
                    "}"
                    "QRadioButton:hover {"
                    "    border-color: #3498db;"
                    "    background-color: #ecf0f1;"
                    "}"
                    "QRadioButton:checked {"
                    "    border-color: #3498db;"
                    "    background-color: #3498db;"
                    "    color: white;"
                    "}"
                );
            }
        } else {
            // Not answered yet: default style
            button->setStyleSheet(
                "QRadioButton {"
                "    font-size: 12px;"
                "    padding: 10px;"
                "    margin: 5px;"
                "    border: 2px solid #bdc3c7;"
                "    border-radius: 5px;"
                "    background-color: white;"
                "    color: #2c3e50;"
                "}"
                "QRadioButton:hover {"
                "    border-color: #3498db;"
                "    background-color: #ecf0f1;"
                "}"
                "QRadioButton:checked {"
                "    border-color: #3498db;"
                "    background-color: #3498db;"
                "    color: white;"
                "}"
            );
        }
    }
}

void QuizWindow::startFlickerEffect(QRadioButton* correctButton)
{
    m_correctAnswerButton = correctButton;
    m_flickerState = false;
    m_flickerTimer->start();
}

void QuizWindow::stopFlickerEffect()
{
    m_flickerTimer->stop();
    if (m_correctAnswerButton) {
        // Set final green state
        m_correctAnswerButton->setStyleSheet(
            "QRadioButton {"
            "    font-size: 12px;"
            "    padding: 10px;"
            "    margin: 5px;"
            "    border: 2px solid #27ae60;"
            "    border-radius: 5px;"
            "    background-color: #27ae60;"
            "    color: white;"
            "}"
        );
        m_correctAnswerButton = nullptr;
    }
}

void QuizWindow::onFlickerTimer()
{
    if (!m_correctAnswerButton) return;
    
    m_flickerState = !m_flickerState;
    
    if (m_flickerState) {
        // Bright green
        m_correctAnswerButton->setStyleSheet(
            "QRadioButton {"
            "    font-size: 12px;"
            "    padding: 10px;"
            "    margin: 5px;"
            "    border: 2px solid #2ecc71;"
            "    border-radius: 5px;"
            "    background-color: #2ecc71;"
            "    color: white;"
            "}"
        );
    } else {
        // Darker green
        m_correctAnswerButton->setStyleSheet(
            "QRadioButton {"
            "    font-size: 12px;"
            "    padding: 10px;"
            "    margin: 5px;"
            "    border: 2px solid #27ae60;"
            "    border-radius: 5px;"
            "    background-color: #27ae60;"
            "    color: white;"
            "}"
        );
    }
}

void QuizWindow::resetOptionButtonStyles()
{
    for (QRadioButton* button : m_optionButtons) {
        button->setStyleSheet(
            "QRadioButton {"
            "    font-size: 12px;"
            "    padding: 10px;"
            "    margin: 5px;"
            "    border: 2px solid #bdc3c7;"
            "    border-radius: 5px;"
            "    background-color: white;"
            "}"
            "QRadioButton:hover {"
            "    border-color: #3498db;"
            "    background-color: #ecf0f1;"
            "}"
            "QRadioButton:checked {"
            "    border-color: #3498db;"
            "    background-color: #3498db;"
            "    color: white;"
            "}"
        );
    }
} 

void QuizWindow::setupSound()
{
    // Sound feedback is handled by QApplication::beep()
    // No additional setup needed
}

void QuizWindow::playWrongSound()
{
    // Play system beep for wrong answer
    QApplication::beep();
}

void QuizWindow::showSpiceItUp()
{
    if (m_spiceShown) return;
    
    m_spiceShown = true;
    
    // Position overlay in center of window
    m_spiceOverlay->move((width() - m_spiceOverlay->width()) / 2,
                         (height() - m_spiceOverlay->height()) / 2);
    
    // Show overlay with animation
    m_spiceOverlay->show();
    m_spiceOverlay->raise();
    
    // Start timer to hide after 3 seconds
    m_spiceTimer->start();
    
    // Add some visual effects
    m_spiceLabel->setText("Letsss Spice it upppp");
    
    // Apply pulsing animation effect
    QTimer* pulseTimer = new QTimer(this);
    pulseTimer->setInterval(200);
    int pulseCount = 0;
    connect(pulseTimer, &QTimer::timeout, [this, pulseTimer, &pulseCount]() {
        pulseCount++;
        if (pulseCount >= 15) { // 3 seconds
            pulseTimer->stop();
            pulseTimer->deleteLater();
            return;
        }
        
        // Pulse effect
        if (pulseCount % 2 == 0) {
            m_spiceLabel->setStyleSheet(
                "QLabel {"
                "    color: white;"
                "    font-size: 32px;"
                "    font-weight: bold;"
                "    text-shadow: 2px 2px 4px rgba(0,0,0,0.5);"
                "    background: transparent;"
                "    border: none;"
                "}"
            );
        } else {
            m_spiceLabel->setStyleSheet(
                "QLabel {"
                "    color: #ffeb3b;"
                "    font-size: 28px;"
                "    font-weight: bold;"
                "    text-shadow: 2px 2px 4px rgba(0,0,0,0.5);"
                "    background: transparent;"
                "    border: none;"
                "}"
            );
        }
    });
    pulseTimer->start();
}

void QuizWindow::hideSpiceItUp()
{
    m_spiceOverlay->hide();
    m_spiceShown = false;
}

void QuizWindow::onSpiceTimer()
{
    hideSpiceItUp();
} 