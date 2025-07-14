#include "chartwidget.h"
#include <QGridLayout>
#include <QGroupBox>
#include <QFont>
#include <QPalette>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>
#include <QDebug> // Added for qDebug()
#include <QPushButton> // Added for QPushButton
#include <QTabWidget> // Added for QTabWidget
#include <QVBoxLayout> // Added for QVBoxLayout
#include <QLabel> // Added for QLabel

ChartWidget::ChartWidget(QWidget *parent)
    : QWidget(parent)
    , m_totalQuestions(0)
    , m_answeredCount(0)
    , m_correctCount(0)
    , m_overallAccuracy(0.0)
    , m_averageTime(0.0)
{
    setupUI();
}

ChartWidget::~ChartWidget()
{
}

void ChartWidget::updateCharts(const QList<QuizQuestion>& questions)
{
    qDebug() << "ChartWidget::updateCharts called with" << questions.size() << "questions";
    m_questions = questions;
    
    // Ensure we have data to display
    if (questions.isEmpty()) {
        qDebug() << "No questions provided to chart widget";
        return;
    }
    
    createAccuracyChart();
    createDifficultyChart();
    createTimeChart();
    createQuestionAnalysis();
    
    qDebug() << "Charts updated successfully";
}

void ChartWidget::setQuizStats(int totalQuestions, int answeredCount, int correctCount, 
                              double overallAccuracy, double averageTime)
{
    qDebug() << "Setting quiz stats:" << totalQuestions << answeredCount << correctCount << overallAccuracy << averageTime;
    
    m_totalQuestions = totalQuestions;
    m_answeredCount = answeredCount;
    m_correctCount = correctCount;
    m_overallAccuracy = overallAccuracy;
    m_averageTime = averageTime;
    
    // Update stats labels
    m_totalQuestionsLabel->setText(QString("Total Questions: %1").arg(totalQuestions));
    m_answeredCountLabel->setText(QString("Answered: %1").arg(answeredCount));
    m_correctCountLabel->setText(QString("Correct: %1").arg(correctCount));
    m_accuracyLabel->setText(QString("Overall Accuracy: %1%").arg(overallAccuracy, 0, 'f', 1));
    m_averageTimeLabel->setText(QString("Average Time: %1s").arg(averageTime, 0, 'f', 1));
    
    qDebug() << "Quiz stats updated successfully";
}

void ChartWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // Create home button
    m_homeButton = new QPushButton("🏠 Back to Home");
    m_homeButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #f1c40f;"
        "    color: #2c3e50;"
        "    border: none;"
        "    border-radius: 5px;"
        "    padding: 10px 20px;"
        "    font-weight: bold;"
        "    font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #f39c12;"
        "}"
    );
    connect(m_homeButton, &QPushButton::clicked, this, &ChartWidget::onHomeButtonClicked);
    
    // Add home button to top
    mainLayout->addWidget(m_homeButton);
    
    // Create tab widget
    m_tabWidget = new QTabWidget();
    m_tabWidget->setStyleSheet(
        "QTabWidget::pane {"
        "    border: 2px solid #bdc3c7;"
        "    border-radius: 5px;"
        "    background-color: white;"
        "}"
        "QTabBar::tab {"
        "    background-color: #ecf0f1;"
        "    border: 1px solid #bdc3c7;"
        "    padding: 8px 16px;"
        "    margin-right: 2px;"
        "}"
        "QTabBar::tab:selected {"
        "    background-color: #3498db;"
        "    color: white;"
        "}"
    );
    
    // Create chart views
    m_accuracyChartView = new QChartView();
    m_difficultyChartView = new QChartView();
    m_timeChartView = new QChartView();
    
    // Create stats widget
    createStatsWidget();
    
    // Add tabs
    QWidget* accuracyTab = new QWidget();
    QVBoxLayout* accuracyLayout = new QVBoxLayout(accuracyTab);
    accuracyLayout->addWidget(m_accuracyChartView);
    m_tabWidget->addTab(accuracyTab, "Accuracy Analysis");
    
    QWidget* difficultyTab = new QWidget();
    QVBoxLayout* difficultyLayout = new QVBoxLayout(difficultyTab);
    difficultyLayout->addWidget(m_difficultyChartView);
    m_tabWidget->addTab(difficultyTab, "Difficulty Breakdown");
    
    QWidget* timeTab = new QWidget();
    QVBoxLayout* timeLayout = new QVBoxLayout(timeTab);
    timeLayout->addWidget(m_timeChartView);
    m_tabWidget->addTab(timeTab, "Time Analysis");
    
    QWidget* statsTab = new QWidget();
    QVBoxLayout* statsLayout = new QVBoxLayout(statsTab);
    statsLayout->addWidget(m_statsWidget);
    m_tabWidget->addTab(statsTab, "Statistics");
    
    mainLayout->addWidget(m_tabWidget);
}

void ChartWidget::createStatsWidget()
{
    m_statsWidget = new QWidget();
    QGridLayout* statsLayout = new QGridLayout(m_statsWidget);
    statsLayout->setSpacing(15);
    
    // Create stat labels
    m_totalQuestionsLabel = new QLabel("Total Questions: 0");
    m_answeredCountLabel = new QLabel("Answered: 0");
    m_correctCountLabel = new QLabel("Correct: 0");
    m_accuracyLabel = new QLabel("Overall Accuracy: 0%");
    m_averageTimeLabel = new QLabel("Average Time: 0s");
    
    // Style the labels
    QFont statFont = m_totalQuestionsLabel->font();
    statFont.setPointSize(14);
    statFont.setBold(true);
    
    m_totalQuestionsLabel->setFont(statFont);
    m_answeredCountLabel->setFont(statFont);
    m_correctCountLabel->setFont(statFont);
    m_accuracyLabel->setFont(statFont);
    m_averageTimeLabel->setFont(statFont);
    
    QString labelStyle = "QLabel { color: #2c3e50; padding: 10px; background-color: #ecf0f1; border-radius: 5px; }";
    m_totalQuestionsLabel->setStyleSheet(labelStyle);
    m_answeredCountLabel->setStyleSheet(labelStyle);
    m_correctCountLabel->setStyleSheet(labelStyle);
    m_accuracyLabel->setStyleSheet(labelStyle);
    m_averageTimeLabel->setStyleSheet(labelStyle);
    
    // Add to layout
    statsLayout->addWidget(m_totalQuestionsLabel, 0, 0);
    statsLayout->addWidget(m_answeredCountLabel, 0, 1);
    statsLayout->addWidget(m_correctCountLabel, 1, 0);
    statsLayout->addWidget(m_accuracyLabel, 1, 1);
    statsLayout->addWidget(m_averageTimeLabel, 2, 0, 1, 2);
    
    m_statsWidget->setStyleSheet(
        "QWidget {"
        "    background-color: white;"
        "    border: 2px solid #bdc3c7;"
        "    border-radius: 10px;"
        "}"
    );
}

void ChartWidget::createAccuracyChart()
{
    qDebug() << "Creating accuracy chart with correct:" << m_correctCount << "answered:" << m_answeredCount;
    
    QPieSeries* series = new QPieSeries();
    
    int correct = m_correctCount;
    int wrong = m_answeredCount - m_correctCount;
    
    // Ensure we have at least some data to display
    if (correct == 0 && wrong == 0) {
        correct = 1; // Show at least one slice
        wrong = 1;
    }
    
    if (correct > 0) {
        QPieSlice* correctSlice = series->append("Correct", correct);
        correctSlice->setColor(QColor("#27ae60"));
        correctSlice->setLabelVisible(true);
    }
    
    if (wrong > 0) {
        QPieSlice* wrongSlice = series->append("Incorrect", wrong);
        wrongSlice->setColor(QColor("#e74c3c"));
        wrongSlice->setLabelVisible(true);
    }
    
    QChart* chart = createPieChart("Quiz Accuracy", series);
    m_accuracyChartView->setChart(chart);
    qDebug() << "Accuracy chart created";
}

void ChartWidget::createDifficultyChart()
{
    qDebug() << "Creating difficulty chart";
    
    QBarSeries* series = new QBarSeries();
    QBarSet* correctSet = new QBarSet("Correct");
    QBarSet* incorrectSet = new QBarSet("Incorrect");
    
    QMap<Difficulty, int> difficultyStats = getDifficultyStats();
    QMap<Difficulty, double> accuracyByDifficulty = getAccuracyByDifficulty();
    
    QStringList categories;
    categories << "Easy" << "Medium" << "Hard";
    
    for (Difficulty diff : {Difficulty::Easy, Difficulty::Medium, Difficulty::Hard}) {
        int total = difficultyStats.value(diff, 0);
        double accuracy = accuracyByDifficulty.value(diff, 0.0);
        int correct = static_cast<int>((accuracy / 100.0) * total);
        int incorrect = total - correct;
        
        correctSet->append(correct);
        incorrectSet->append(incorrect);
    }
    
    correctSet->setColor(QColor("#27ae60"));
    incorrectSet->setColor(QColor("#e74c3c"));
    
    series->append(correctSet);
    series->append(incorrectSet);
    
    QChart* chart = createBarChart("Performance by Difficulty", series);
    
    QBarCategoryAxis* axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    
    QValueAxis* axisY = new QValueAxis();
    axisY->setRange(0, 10);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    
    m_difficultyChartView->setChart(chart);
    qDebug() << "Difficulty chart created";
}

void ChartWidget::createTimeChart()
{
    qDebug() << "Creating time chart";
    
    QBarSeries* series = new QBarSeries();
    QBarSet* timeSet = new QBarSet("Time (seconds)");
    
    QList<int> timePerQuestion = getTimePerQuestion();
    
    // Ensure we have some data
    if (timePerQuestion.isEmpty()) {
        timePerQuestion = {30, 45, 60, 30, 45}; // Default times
    }
    
    for (int i = 0; i < timePerQuestion.size(); ++i) {
        timeSet->append(timePerQuestion[i]);
    }
    
    timeSet->setColor(QColor("#3498db"));
    series->append(timeSet);
    
    QChart* chart = createBarChart("Time per Question", series);
    
    QValueAxis* axisX = new QValueAxis();
    axisX->setRange(0, timePerQuestion.size());
    axisX->setTitleText("Question Number");
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    
    QValueAxis* axisY = new QValueAxis();
    axisY->setRange(0, 120); // 2 minutes max
    axisY->setTitleText("Time (seconds)");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    
    m_timeChartView->setChart(chart);
    qDebug() << "Time chart created";
}

void ChartWidget::createQuestionAnalysis()
{
    // This method can be used to create detailed question analysis
    // For now, it's handled by the other chart methods
}

QChart* ChartWidget::createPieChart(const QString& title, QPieSeries* series)
{
    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(title);
    chart->setTitleFont(QFont("Arial", 16, QFont::Bold));
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    
    return chart;
}

QChart* ChartWidget::createBarChart(const QString& title, QBarSeries* series)
{
    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(title);
    chart->setTitleFont(QFont("Arial", 16, QFont::Bold));
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    
    return chart;
}

QMap<Difficulty, int> ChartWidget::getDifficultyStats() const
{
    QMap<Difficulty, int> stats;
    
    for (const QuizQuestion& question : m_questions) {
        if (question.isAnswered()) {
            stats[question.getDifficulty()]++;
        }
    }
    
    return stats;
}

QMap<Difficulty, double> ChartWidget::getAccuracyByDifficulty() const
{
    QMap<Difficulty, double> accuracy;
    QMap<Difficulty, int> total;
    QMap<Difficulty, int> correct;
    
    for (const QuizQuestion& question : m_questions) {
        if (question.isAnswered()) {
            total[question.getDifficulty()]++;
            if (question.isCorrect()) {
                correct[question.getDifficulty()]++;
            }
        }
    }
    
    for (auto it = total.begin(); it != total.end(); ++it) {
        Difficulty diff = it.key();
        int totalCount = it.value();
        int correctCount = correct.value(diff, 0);
        
        if (totalCount > 0) {
            accuracy[diff] = (static_cast<double>(correctCount) / totalCount) * 100.0;
        } else {
            accuracy[diff] = 0.0;
        }
    }
    
    return accuracy;
}

QList<int> ChartWidget::getTimePerQuestion() const
{
    QList<int> times;
    
    for (const QuizQuestion& question : m_questions) {
        times.append(question.getTimeSpent());
    }
    
    return times;
} 

void ChartWidget::onHomeButtonClicked()
{
    emit homeRequested();
} 