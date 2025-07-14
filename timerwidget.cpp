#include "timerwidget.h"
#include <QFont>
#include <QPalette>
#include <QStyle>

TimerWidget::TimerWidget(QWidget *parent)
    : QWidget(parent)
    , m_timer(new QTimer(this))
    , m_remainingTime(0)
    , m_totalTime(0)
    , m_isRunning(false)
    , m_isPaused(false)
{
    // Setup layout
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(5);
    layout->setContentsMargins(10, 10, 10, 10);
    
    // Time label
    m_timeLabel = new QLabel("00:00");
    m_timeLabel->setAlignment(Qt::AlignCenter);
    QFont timeFont = m_timeLabel->font();
    timeFont.setPointSize(24);
    timeFont.setBold(true);
    m_timeLabel->setFont(timeFont);
    m_timeLabel->setStyleSheet("QLabel { color: #2c3e50; }");
    
    // Progress bar
    m_progressBar = new QProgressBar();
    m_progressBar->setTextVisible(false);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(100);
    m_progressBar->setStyleSheet(
        "QProgressBar {"
        "    border: 2px solid #bdc3c7;"
        "    border-radius: 5px;"
        "    background-color: #ecf0f1;"
        "}"
        "QProgressBar::chunk {"
        "    background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "        stop:0 #3498db, stop:1 #2980b9);"
        "    border-radius: 3px;"
        "}"
    );
    
    layout->addWidget(m_timeLabel);
    layout->addWidget(m_progressBar);
    
    // Setup timer
    m_timer->setInterval(1000); // 1 second
    connect(m_timer, &QTimer::timeout, this, &TimerWidget::updateDisplay);
    
    setFixedSize(200, 100);
    setStyleSheet(
        "TimerWidget {"
        "    background-color: white;"
        "    border: 2px solid #bdc3c7;"
        "    border-radius: 10px;"
        "}"
    );
}

TimerWidget::~TimerWidget()
{
}

void TimerWidget::setTime(int seconds)
{
    m_totalTime = seconds;
    m_remainingTime = seconds;
    updateDisplay();
}

void TimerWidget::startTimer()
{
    if (m_isRunning) return;
    
    m_isRunning = true;
    m_isPaused = false;
    m_timer->start();
}

void TimerWidget::stopTimer()
{
    m_isRunning = false;
    m_isPaused = false;
    m_timer->stop();
}

void TimerWidget::pauseTimer()
{
    if (!m_isRunning || m_isPaused) return;
    
    m_isPaused = true;
    m_timer->stop();
}

void TimerWidget::resumeTimer()
{
    if (!m_isRunning || !m_isPaused) return;
    
    m_isPaused = false;
    m_timer->start();
}

void TimerWidget::setPaused(bool paused)
{
    if (paused) {
        pauseTimer();
    } else {
        resumeTimer();
    }
}

bool TimerWidget::isRunning() const
{
    return m_isRunning && !m_isPaused;
}

int TimerWidget::getRemainingTime() const
{
    return m_remainingTime;
}

void TimerWidget::updateDisplay()
{
    if (m_remainingTime <= 0) {
        m_remainingTime = 0;
        stopTimer();
        emit timeUp();
    } else {
        m_remainingTime--;
    }
    
    m_timeLabel->setText(formatTime(m_remainingTime));
    updateProgressBar();
    
    // Change color based on remaining time
    if (m_remainingTime <= 300) { // Last 5 minutes
        m_timeLabel->setStyleSheet("QLabel { color: #e74c3c; font-weight: bold; }");
    } else if (m_remainingTime <= 600) { // Last 10 minutes
        m_timeLabel->setStyleSheet("QLabel { color: #f39c12; font-weight: bold; }");
    } else {
        m_timeLabel->setStyleSheet("QLabel { color: #2c3e50; }");
    }
}

void TimerWidget::updateProgressBar()
{
    if (m_totalTime > 0) {
        int percentage = static_cast<int>((static_cast<double>(m_remainingTime) / m_totalTime) * 100);
        m_progressBar->setValue(percentage);
        
        // Update progress bar color based on remaining time
        if (m_remainingTime <= 300) {
            m_progressBar->setStyleSheet(
                "QProgressBar {"
                "    border: 2px solid #bdc3c7;"
                "    border-radius: 5px;"
                "    background-color: #ecf0f1;"
                "}"
                "QProgressBar::chunk {"
                "    background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
                "        stop:0 #e74c3c, stop:1 #c0392b);"
                "    border-radius: 3px;"
                "}"
            );
        } else if (m_remainingTime <= 600) {
            m_progressBar->setStyleSheet(
                "QProgressBar {"
                "    border: 2px solid #bdc3c7;"
                "    border-radius: 5px;"
                "    background-color: #ecf0f1;"
                "}"
                "QProgressBar::chunk {"
                "    background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
                "        stop:0 #f39c12, stop:1 #e67e22);"
                "    border-radius: 3px;"
                "}"
            );
        } else {
            m_progressBar->setStyleSheet(
                "QProgressBar {"
                "    border: 2px solid #bdc3c7;"
                "    border-radius: 5px;"
                "    background-color: #ecf0f1;"
                "}"
                "QProgressBar::chunk {"
                "    background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
                "        stop:0 #3498db, stop:1 #2980b9);"
                "    border-radius: 3px;"
                "}"
            );
        }
    }
}

QString TimerWidget::formatTime(int seconds) const
{
    int minutes = seconds / 60;
    int remainingSeconds = seconds % 60;
    return QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(remainingSeconds, 2, 10, QChar('0'));
} 