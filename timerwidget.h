#ifndef TIMERWIDGET_H
#define TIMERWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>

class TimerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TimerWidget(QWidget *parent = nullptr);
    ~TimerWidget();

    void setTime(int seconds);
    void startTimer();
    void stopTimer();
    void pauseTimer();
    void resumeTimer();
    void setPaused(bool paused);
    bool isRunning() const;
    int getRemainingTime() const;

signals:
    void timeUp();

private slots:
    void updateDisplay();

private:
    QLabel* m_timeLabel;
    QProgressBar* m_progressBar;
    QTimer* m_timer;
    int m_remainingTime;
    int m_totalTime;
    bool m_isRunning;
    bool m_isPaused;
    
    void updateProgressBar();
    QString formatTime(int seconds) const;
};

#endif // TIMERWIDGET_H 