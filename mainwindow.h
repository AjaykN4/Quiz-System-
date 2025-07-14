#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QFrame>
#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
#include <QSlider>
#include <QSpinBox>
#include <QApplication>
#include <QStyle>
#include "quizwindow.h"
#include "chartwidget.h"
#include "customquizmaker.h"
#include "quizmanager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void startDefaultQuiz();
    void startCppDSAQuiz();
    void startCustomQuiz();
    void showAnalytics();
    void showSettings();
    void onQuizFinished();
    void onCustomQuizCreated(const QString& name, const QList<QuizQuestion>& questions);

private:
    // UI Components
    QStackedWidget* m_stackedWidget;
    QWidget* m_homeWidget;
    QuizWindow* m_quizWindow;
    ChartWidget* m_analyticsWidget;
    CustomQuizMaker* m_customQuizMaker;
    
    // Home screen components
    QLabel* m_titleLabel;
    QPushButton* m_defaultQuizButton;
    QPushButton* m_cppDSAQuizButton;
    QPushButton* m_customQuizButton;
    QPushButton* m_analyticsButton;
    QPushButton* m_settingsButton;
    QPushButton* m_homeButton;
    
    // Settings components
    QWidget* m_settingsWidget;
    QComboBox* m_timerCombo;
    QCheckBox* m_adaptiveModeCheck;
    QSlider* m_questionCountSlider;
    QSpinBox* m_questionCountSpinBox;
    
    // Quiz Manager
    QuizManager* m_quizManager;
    
    void setupUI();
    void setupHomeScreen();
    void setupSettingsScreen();
    void setupConnections();
    void showHomeScreen();
    void showQuizScreen();
    void showAnalyticsScreen();
    void showSettingsScreen();
    void showCustomQuizMaker();

signals:
    void quizStarted();
    void quizFinished();
};

#endif // MAINWINDOW_H 