#include "mainwindow.h"
#include <QFont>
#include <QPalette>
#include <QStyle>
#include <QMessageBox>
#include <QScreen>
#include <QGuiApplication>
#include <QDebug> // Added for qDebug

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_quizManager(new QuizManager(this))
{
    setWindowTitle("Quiz System");
    setMinimumSize(1000, 700);
    
    setupUI();
    setupConnections();
    
    // Apply light theme
    setStyleSheet(
        "QMainWindow, QWidget {"
        "    background-color: #f8f9fa;"
        "    color: #2c3e50;"
        "}"
        "QLabel {"
        "    color: #2c3e50;"
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
        "QPushButton {"
        "    background-color: #3498db;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 5px;"
        "    padding: 8px 16px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2980b9;"
        "}"
        "QLineEdit, QTextEdit, QComboBox, QListWidget, QSpinBox, QSlider {"
        "    border: 2px solid #bdc3c7;"
        "    border-radius: 5px;"
        "    padding: 5px;"
        "    background-color: white;"
        "    color: #2c3e50;"
        "}"
        "QCheckBox {"
        "    color: #2c3e50;"
        "}"
        "QProgressBar {"
        "    border: 2px solid #bdc3c7;"
        "    border-radius: 5px;"
        "    background-color: #ecf0f1;"
        "    color: #2c3e50;"
        "}"
        "QProgressBar::chunk {"
        "    background-color: #27ae60;"
        "    border-radius: 3px;"
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
    
    // Center the window
    if (QScreen* screen = QGuiApplication::primaryScreen()) {
        QRect screenGeometry = screen->availableGeometry();
        QRect windowGeometry = geometry();
        windowGeometry.moveCenter(screenGeometry.center());
        setGeometry(windowGeometry);
    }
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    m_stackedWidget = new QStackedWidget();
    setCentralWidget(m_stackedWidget);
    
    setupHomeScreen();
    setupSettingsScreen();
    
    // Create other screens
    m_quizWindow = new QuizWindow(m_quizManager);
    m_analyticsWidget = new ChartWidget();
    m_customQuizMaker = new CustomQuizMaker();
    
    // Add widgets to stacked widget
    m_stackedWidget->addWidget(m_homeWidget);
    m_stackedWidget->addWidget(m_quizWindow);
    m_stackedWidget->addWidget(m_analyticsWidget);
    m_stackedWidget->addWidget(m_settingsWidget);
    
    // Show home screen initially
    m_stackedWidget->setCurrentWidget(m_homeWidget);
}

void MainWindow::setupHomeScreen()
{
    m_homeWidget = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(m_homeWidget);
    mainLayout->setSpacing(30);
    mainLayout->setContentsMargins(50, 50, 50, 50);
    
    // Title
    m_titleLabel = new QLabel("Quiz System");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(36);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet("QLabel { color: #2c3e50; margin-bottom: 20px; }");
    
    // Subtitle
    QLabel* subtitleLabel = new QLabel("Professional Adaptive Quiz Platform");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    QFont subtitleFont = subtitleLabel->font();
    subtitleFont.setPointSize(16);
    subtitleFont.setItalic(true);
    subtitleLabel->setFont(subtitleFont);
    subtitleLabel->setStyleSheet("QLabel { color: #7f8c8d; margin-bottom: 40px; }");
    
    // Quiz options grid
    QGridLayout* optionsLayout = new QGridLayout();
    optionsLayout->setSpacing(20);
    
    // Current Affairs Button
    m_defaultQuizButton = new QPushButton("📰 Current Affairs");
    m_defaultQuizButton->setMinimumSize(200, 80);
    m_defaultQuizButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #3498db;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 10px;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    padding: 15px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2980b9;"
        "}"
    );
    
    // C++ DSA Quiz Button
    m_cppDSAQuizButton = new QPushButton("💻 C++ DSA Quiz");
    m_cppDSAQuizButton->setMinimumSize(200, 80);
    m_cppDSAQuizButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #e74c3c;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 10px;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    padding: 15px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #c0392b;"
        "}"
    );
    
    // Custom Quiz Button
    m_customQuizButton = new QPushButton("🎯 Custom Quiz");
    m_customQuizButton->setMinimumSize(200, 80);
    m_customQuizButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #f39c12;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 10px;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    padding: 15px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #e67e22;"
        "}"
    );
    
    // Settings Button
    m_settingsButton = new QPushButton("⚙️ Settings");
    m_settingsButton->setMinimumSize(200, 80);
    m_settingsButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #95a5a6;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 10px;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    padding: 15px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #7f8c8d;"
        "}"
    );
    
    // Analytics Button
    m_analyticsButton = new QPushButton("📊 Analytics");
    m_analyticsButton->setMinimumSize(200, 80);
    m_analyticsButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #9b59b6;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 10px;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    padding: 15px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #8e44ad;"
        "}"
    );
    
    // Add buttons to grid
    optionsLayout->addWidget(m_defaultQuizButton, 0, 0);
    optionsLayout->addWidget(m_cppDSAQuizButton, 0, 1);
    optionsLayout->addWidget(m_customQuizButton, 0, 2);
    optionsLayout->addWidget(m_analyticsButton, 1, 0);
    optionsLayout->addWidget(m_settingsButton, 1, 1);
    
    // Top bar
    QHBoxLayout* topBarLayout = new QHBoxLayout();
    m_homeButton = new QPushButton("🏠 Home");
    m_homeButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #f1c40f;"
        "    color: #2c3e50;"
        "    border: none;"
        "    border-radius: 5px;"
        "    padding: 8px 16px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #f39c12;"
        "}"
    );
    topBarLayout->addWidget(m_homeButton);
    topBarLayout->addStretch();
    mainLayout->addLayout(topBarLayout);
    // Home button is hidden on quiz screen
    m_homeButton->setVisible(true);
    
    // Add all components to main layout
    mainLayout->addWidget(m_titleLabel);
    mainLayout->addWidget(subtitleLabel);
    mainLayout->addLayout(optionsLayout);
    mainLayout->addStretch();
    
    // Style the home widget
    // Theme will be applied globally through main window stylesheet
}

void MainWindow::setupSettingsScreen()
{
    m_settingsWidget = new QWidget();
    QVBoxLayout* settingsLayout = new QVBoxLayout(m_settingsWidget);
    settingsLayout->setSpacing(20);
    settingsLayout->setContentsMargins(50, 50, 50, 50);
    
    // Title
    QLabel* settingsTitle = new QLabel("Settings");
    settingsTitle->setAlignment(Qt::AlignCenter);
    QFont titleFont = settingsTitle->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    settingsTitle->setFont(titleFont);
    settingsTitle->setStyleSheet("QLabel { color: #2c3e50; margin-bottom: 20px; }");
    
    // Settings groups
    QGroupBox* timerGroup = new QGroupBox("Timer Settings");
    QVBoxLayout* timerLayout = new QVBoxLayout(timerGroup);
    
    m_timerCombo = new QComboBox();
    m_timerCombo->addItems({"15 minutes", "30 minutes", "45 minutes", "60 minutes"});
    m_timerCombo->setCurrentIndex(1); // Default to 30 minutes
    timerLayout->addWidget(new QLabel("Quiz Duration:"));
    timerLayout->addWidget(m_timerCombo);
    
    QGroupBox* quizGroup = new QGroupBox("Quiz Settings");
    QVBoxLayout* quizLayout = new QVBoxLayout(quizGroup);
    
    m_adaptiveModeCheck = new QCheckBox("Enable Adaptive Difficulty");
    m_adaptiveModeCheck->setChecked(true);
    quizLayout->addWidget(m_adaptiveModeCheck);
    
    QHBoxLayout* questionCountLayout = new QHBoxLayout();
    questionCountLayout->addWidget(new QLabel("Number of Questions:"));
    m_questionCountSlider = new QSlider(Qt::Horizontal);
    m_questionCountSlider->setRange(5, 50);
    m_questionCountSlider->setValue(15);
    m_questionCountSpinBox = new QSpinBox();
    m_questionCountSpinBox->setRange(5, 50);
    m_questionCountSpinBox->setValue(15);
    
    connect(m_questionCountSlider, &QSlider::valueChanged, m_questionCountSpinBox, &QSpinBox::setValue);
    connect(m_questionCountSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), m_questionCountSlider, &QSlider::setValue);
    
    questionCountLayout->addWidget(m_questionCountSlider);
    questionCountLayout->addWidget(m_questionCountSpinBox);
    quizLayout->addLayout(questionCountLayout);
    
    // Back button
    QPushButton* backButton = new QPushButton("← Back to Home");
    backButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #95a5a6;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 5px;"
        "    padding: 10px 20px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #7f8c8d;"
        "}"
    );
    connect(backButton, &QPushButton::clicked, this, &MainWindow::showHomeScreen);
    
    // Add all components
    settingsLayout->addWidget(settingsTitle);
    settingsLayout->addWidget(timerGroup);
    settingsLayout->addWidget(quizGroup);
    settingsLayout->addWidget(backButton);
    settingsLayout->addStretch();
    
    // Theme will be applied globally through main window stylesheet
}

void MainWindow::setupConnections()
{
    connect(m_defaultQuizButton, &QPushButton::clicked, this, &MainWindow::startDefaultQuiz);
    connect(m_cppDSAQuizButton, &QPushButton::clicked, this, &MainWindow::startCppDSAQuiz);
    connect(m_customQuizButton, &QPushButton::clicked, this, &MainWindow::startCustomQuiz);
    connect(m_analyticsButton, &QPushButton::clicked, this, &MainWindow::showAnalytics);
    connect(m_settingsButton, &QPushButton::clicked, this, &MainWindow::showSettings);
    connect(m_homeButton, &QPushButton::clicked, this, &MainWindow::showHomeScreen);
    
    connect(m_quizManager, &QuizManager::quizFinished, this, &MainWindow::onQuizFinished);
    connect(m_customQuizMaker, &CustomQuizMaker::quizCreated, this, &MainWindow::onCustomQuizCreated);
    
    // Connect analytics widget home button
    connect(m_analyticsWidget, &ChartWidget::homeRequested, this, &MainWindow::showHomeScreen);
}

void MainWindow::startDefaultQuiz()
{
    m_quizManager->startQuiz("default");
    showQuizScreen();
}

void MainWindow::startCppDSAQuiz()
{
    m_quizManager->startQuiz("cpp_dsa");
    showQuizScreen();
}

void MainWindow::startCustomQuiz()
{
    showCustomQuizMaker();
}

void MainWindow::showAnalytics()
{
    // Update analytics with current quiz data
    m_analyticsWidget->updateCharts(m_quizManager->getAllQuestions());
    m_analyticsWidget->setQuizStats(
        m_quizManager->getTotalQuestions(),
        m_quizManager->getAnsweredCount(),
        m_quizManager->getCorrectCount(),
        m_quizManager->getOverallAccuracy(),
        m_quizManager->getAverageTimePerQuestion()
    );
    showAnalyticsScreen();
}

void MainWindow::showSettings()
{
    showSettingsScreen();
}

void MainWindow::onQuizFinished()
{
    // Update analytics with current quiz data before showing
    QList<QuizQuestion> questions = m_quizManager->getAllQuestions();
    qDebug() << "Quiz finished. Total questions:" << questions.size();
    qDebug() << "Answered questions:" << m_quizManager->getAnsweredCount();
    qDebug() << "Correct answers:" << m_quizManager->getCorrectCount();
    
    m_analyticsWidget->updateCharts(questions);
    m_analyticsWidget->setQuizStats(
        m_quizManager->getTotalQuestions(),
        m_quizManager->getAnsweredCount(),
        m_quizManager->getCorrectCount(),
        m_quizManager->getOverallAccuracy(),
        m_quizManager->getAverageTimePerQuestion()
    );
    
    showAnalyticsScreen();
}

void MainWindow::onCustomQuizCreated(const QString& name, const QList<QuizQuestion>& questions)
{
    m_quizManager->createCustomQuiz(name, questions);
    m_quizManager->loadCustomQuiz(name);
    showQuizScreen();
}

void MainWindow::showHomeScreen()
{
    m_homeButton->setVisible(false);
    m_stackedWidget->setCurrentWidget(m_homeWidget);
}

void MainWindow::showQuizScreen()
{
    m_homeButton->setVisible(false);
    m_stackedWidget->setCurrentWidget(m_quizWindow);
}

void MainWindow::showAnalyticsScreen()
{
    m_homeButton->setVisible(true);
    m_stackedWidget->setCurrentWidget(m_analyticsWidget);
}

void MainWindow::showSettingsScreen()
{
    m_homeButton->setVisible(true);
    m_stackedWidget->setCurrentWidget(m_settingsWidget);
}

void MainWindow::showCustomQuizMaker()
{
    m_homeButton->setVisible(true);
    m_customQuizMaker->show();
} 