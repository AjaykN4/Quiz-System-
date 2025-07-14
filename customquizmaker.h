#ifndef CUSTOMQUIZMAKER_H
#define CUSTOMQUIZMAKER_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QListWidget>
#include <QSpinBox>
#include <QGroupBox>
#include <QMessageBox>
#include <QFileDialog>
#include "quizquestion.h"

class CustomQuizMaker : public QDialog
{
    Q_OBJECT

public:
    explicit CustomQuizMaker(QWidget *parent = nullptr);
    ~CustomQuizMaker();

    QList<QuizQuestion> getQuestions() const;
    QString getQuizName() const;

public slots:
    void addQuestion();
    void removeQuestion();
    void editQuestion();
    void saveQuiz();
    void loadQuiz();
    void clearForm();
    void validateForm();

signals:
    void quizCreated(const QString& name, const QList<QuizQuestion>& questions);

private slots:
    void onQuestionSelected(int index);
    void onDifficultyChanged(int index);

private:
    // UI Components
    QLineEdit* m_quizNameEdit;
    QTextEdit* m_questionEdit;
    QListWidget* m_optionsList;
    QLineEdit* m_optionEdit;
    QComboBox* m_difficultyCombo;
    QComboBox* m_correctAnswerCombo;
    QPushButton* m_addOptionButton;
    QPushButton* m_removeOptionButton;
    QPushButton* m_addQuestionButton;
    QPushButton* m_removeQuestionButton;
    QPushButton* m_editQuestionButton;
    QPushButton* m_saveButton;
    QPushButton* m_loadButton;
    QPushButton* m_clearButton;
    QListWidget* m_questionsList;
    
    // Data
    QList<QuizQuestion> m_questions;
    int m_currentQuestionIndex;
    
    void setupUI();
    void setupConnections();
    void updateQuestionsList();
    void updateCorrectAnswerCombo();
    void loadQuestionToForm(int index);
    void clearQuestionForm();
    bool validateQuestionForm();
    void showError(const QString& message);
    void showSuccess(const QString& message);
};

#endif // CUSTOMQUIZMAKER_H 