#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTabWidget>
#include <QPushButton>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>
#include "quizquestion.h"

class ChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChartWidget(QWidget *parent = nullptr);
    ~ChartWidget();

    void updateCharts(const QList<QuizQuestion>& questions);
    void setQuizStats(int totalQuestions, int answeredCount, int correctCount, 
                     double overallAccuracy, double averageTime);

signals:
    void homeRequested();

private slots:
    void createAccuracyChart();
    void createDifficultyChart();
    void createTimeChart();
    void createQuestionAnalysis();
    void onHomeButtonClicked();

private:
    // Charts
    QChartView* m_accuracyChartView;
    QChartView* m_difficultyChartView;
    QChartView* m_timeChartView;
    
    // Stats labels
    QLabel* m_totalQuestionsLabel;
    QLabel* m_answeredCountLabel;
    QLabel* m_correctCountLabel;
    QLabel* m_accuracyLabel;
    QLabel* m_averageTimeLabel;
    
    // Home button
    QPushButton* m_homeButton;
    
    // Data
    QList<QuizQuestion> m_questions;
    int m_totalQuestions;
    int m_answeredCount;
    int m_correctCount;
    double m_overallAccuracy;
    double m_averageTime;
    
    // Layout
    QTabWidget* m_tabWidget;
    QWidget* m_statsWidget;
    
    void setupUI();
    void createStatsWidget();
    QChart* createPieChart(const QString& title, QPieSeries* series);
    QChart* createBarChart(const QString& title, QBarSeries* series);
    QMap<Difficulty, int> getDifficultyStats() const;
    QMap<Difficulty, double> getAccuracyByDifficulty() const;
    QList<int> getTimePerQuestion() const;
};

#endif // CHARTWIDGET_H 