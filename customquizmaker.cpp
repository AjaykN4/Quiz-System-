#include "customquizmaker.h"
#include <QFont>
#include <QStyle>
#include <QApplication>

CustomQuizMaker::CustomQuizMaker(QWidget *parent)
    : QDialog(parent)
    , m_currentQuestionIndex(-1)
{
    setWindowTitle("Custom Quiz Maker");
    setModal(true);
    setFixedSize(800, 600);
    
    setupUI();
    setupConnections();
    
    setStyleSheet(
        "QDialog {"
        "    background-color: #f8f9fa;"
        "}"
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 2px solid #bdc3c7;"
        "    border-radius: 5px;"
        "    margin-top: 1ex;"
        "    padding-top: 10px;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    padding: 0 5px 0 5px;"
        "}"
        "QLineEdit, QTextEdit, QComboBox, QListWidget {"
        "    border: 2px solid #bdc3c7;"
        "    border-radius: 5px;"
        "    padding: 5px;"
        "    background-color: white;"
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
        "QPushButton:pressed {"
        "    background-color: #21618c;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #bdc3c7;"
        "}"
    );
}

CustomQuizMaker::~CustomQuizMaker()
{
}

QList<QuizQuestion> CustomQuizMaker::getQuestions() const
{
    return m_questions;
}

QString CustomQuizMaker::getQuizName() const
{
    return m_quizNameEdit->text();
}

void CustomQuizMaker::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // Quiz Name Section
    QGroupBox* nameGroup = new QGroupBox("Quiz Name");
    QHBoxLayout* nameLayout = new QHBoxLayout(nameGroup);
    m_quizNameEdit = new QLineEdit();
    m_quizNameEdit->setPlaceholderText("Enter quiz name...");
    nameLayout->addWidget(m_quizNameEdit);
    
    // Question Form Section
    QGroupBox* formGroup = new QGroupBox("Question Form");
    QGridLayout* formLayout = new QGridLayout(formGroup);
    
    // Question text
    formLayout->addWidget(new QLabel("Question:"), 0, 0);
    m_questionEdit = new QTextEdit();
    m_questionEdit->setMaximumHeight(100);
    m_questionEdit->setPlaceholderText("Enter your question here...");
    formLayout->addWidget(m_questionEdit, 0, 1);
    
    // Difficulty
    formLayout->addWidget(new QLabel("Difficulty:"), 1, 0);
    m_difficultyCombo = new QComboBox();
    m_difficultyCombo->addItems({"Easy", "Medium", "Hard"});
    formLayout->addWidget(m_difficultyCombo, 1, 1);
    
    // Options
    formLayout->addWidget(new QLabel("Options:"), 2, 0);
    QHBoxLayout* optionsLayout = new QHBoxLayout();
    m_optionEdit = new QLineEdit();
    m_optionEdit->setPlaceholderText("Enter option...");
    m_addOptionButton = new QPushButton("Add");
    m_removeOptionButton = new QPushButton("Remove");
    optionsLayout->addWidget(m_optionEdit);
    optionsLayout->addWidget(m_addOptionButton);
    optionsLayout->addWidget(m_removeOptionButton);
    formLayout->addLayout(optionsLayout, 2, 1);
    
    // Options list
    formLayout->addWidget(new QLabel("Current Options:"), 3, 0);
    m_optionsList = new QListWidget();
    m_optionsList->setMaximumHeight(100);
    formLayout->addWidget(m_optionsList, 3, 1);
    
    // Correct answer
    formLayout->addWidget(new QLabel("Correct Answer:"), 4, 0);
    m_correctAnswerCombo = new QComboBox();
    formLayout->addWidget(m_correctAnswerCombo, 4, 1);
    
    // Question buttons
    QHBoxLayout* questionButtonsLayout = new QHBoxLayout();
    m_addQuestionButton = new QPushButton("Add Question");
    m_editQuestionButton = new QPushButton("Update Question");
    m_removeQuestionButton = new QPushButton("Remove Question");
    questionButtonsLayout->addWidget(m_addQuestionButton);
    questionButtonsLayout->addWidget(m_editQuestionButton);
    questionButtonsLayout->addWidget(m_removeQuestionButton);
    formLayout->addLayout(questionButtonsLayout, 5, 0, 1, 2);
    
    // Questions List Section
    QGroupBox* questionsGroup = new QGroupBox("Questions List");
    QVBoxLayout* questionsLayout = new QVBoxLayout(questionsGroup);
    m_questionsList = new QListWidget();
    questionsLayout->addWidget(m_questionsList);
    
    // Action Buttons
    QHBoxLayout* actionButtonsLayout = new QHBoxLayout();
    m_saveButton = new QPushButton("Save Quiz");
    m_loadButton = new QPushButton("Load Quiz");
    m_clearButton = new QPushButton("Clear All");
    actionButtonsLayout->addWidget(m_saveButton);
    actionButtonsLayout->addWidget(m_loadButton);
    actionButtonsLayout->addWidget(m_clearButton);
    
    // Add all sections to main layout
    mainLayout->addWidget(nameGroup);
    mainLayout->addWidget(formGroup);
    mainLayout->addWidget(questionsGroup);
    mainLayout->addLayout(actionButtonsLayout);
    
    // Set initial states
    m_editQuestionButton->setEnabled(false);
    m_removeQuestionButton->setEnabled(false);
}

void CustomQuizMaker::setupConnections()
{
    connect(m_addOptionButton, &QPushButton::clicked, this, &CustomQuizMaker::addQuestion);
    connect(m_removeOptionButton, &QPushButton::clicked, this, &CustomQuizMaker::removeQuestion);
    connect(m_addQuestionButton, &QPushButton::clicked, this, &CustomQuizMaker::addQuestion);
    connect(m_editQuestionButton, &QPushButton::clicked, this, &CustomQuizMaker::editQuestion);
    connect(m_removeQuestionButton, &QPushButton::clicked, this, &CustomQuizMaker::removeQuestion);
    connect(m_saveButton, &QPushButton::clicked, this, &CustomQuizMaker::saveQuiz);
    connect(m_loadButton, &QPushButton::clicked, this, &CustomQuizMaker::loadQuiz);
    connect(m_clearButton, &QPushButton::clicked, this, &CustomQuizMaker::clearForm);
    
    connect(m_questionsList, &QListWidget::currentRowChanged, this, &CustomQuizMaker::onQuestionSelected);
    connect(m_difficultyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &CustomQuizMaker::onDifficultyChanged);
}

void CustomQuizMaker::addQuestion()
{
    if (sender() == m_addOptionButton) {
        // Add option to list
        QString option = m_optionEdit->text().trimmed();
        if (!option.isEmpty()) {
            m_optionsList->addItem(option);
            m_optionEdit->clear();
            updateCorrectAnswerCombo();
        }
        return;
    }
    
    if (!validateQuestionForm()) return;
    
    // Get form data
    QString questionText = m_questionEdit->toPlainText().trimmed();
    Difficulty difficulty = static_cast<Difficulty>(m_difficultyCombo->currentIndex());
    QString correctAnswer = m_correctAnswerCombo->currentText();
    
    // Get options
    QStringList options;
    for (int i = 0; i < m_optionsList->count(); ++i) {
        options.append(m_optionsList->item(i)->text());
    }
    
    // Create question
    QuizQuestion question(questionText, options, correctAnswer, difficulty);
    
    if (m_currentQuestionIndex >= 0) {
        // Update existing question
        m_questions[m_currentQuestionIndex] = question;
    } else {
        // Add new question
        m_questions.append(question);
    }
    
    updateQuestionsList();
    clearQuestionForm();
    showSuccess("Question added successfully!");
}

void CustomQuizMaker::removeQuestion()
{
    if (sender() == m_removeOptionButton) {
        // Remove selected option
        int currentRow = m_optionsList->currentRow();
        if (currentRow >= 0) {
            delete m_optionsList->takeItem(currentRow);
            updateCorrectAnswerCombo();
        }
        return;
    }
    
    // Remove selected question
    int currentRow = m_questionsList->currentRow();
    if (currentRow >= 0) {
        m_questions.removeAt(currentRow);
        updateQuestionsList();
        clearQuestionForm();
        showSuccess("Question removed successfully!");
    }
}

void CustomQuizMaker::editQuestion()
{
    int currentRow = m_questionsList->currentRow();
    if (currentRow >= 0) {
        m_currentQuestionIndex = currentRow;
        loadQuestionToForm(currentRow);
        m_addQuestionButton->setText("Update Question");
        m_editQuestionButton->setEnabled(false);
    }
}

void CustomQuizMaker::saveQuiz()
{
    if (m_quizNameEdit->text().trimmed().isEmpty()) {
        showError("Please enter a quiz name!");
        return;
    }
    
    if (m_questions.isEmpty()) {
        showError("Please add at least one question!");
        return;
    }
    
    emit quizCreated(m_quizNameEdit->text().trimmed(), m_questions);
    accept();
}

void CustomQuizMaker::loadQuiz()
{
    // This would typically load from a file or database
    // For now, we'll just show a message
    showSuccess("Load functionality will be implemented!");
}

void CustomQuizMaker::clearForm()
{
    m_questions.clear();
    updateQuestionsList();
    clearQuestionForm();
    m_quizNameEdit->clear();
    showSuccess("Form cleared!");
}

void CustomQuizMaker::validateForm()
{
    // This method can be used for real-time validation
}

void CustomQuizMaker::onQuestionSelected(int index)
{
    if (index >= 0 && index < m_questions.size()) {
        loadQuestionToForm(index);
        m_removeQuestionButton->setEnabled(true);
    } else {
        m_removeQuestionButton->setEnabled(false);
    }
}

void CustomQuizMaker::onDifficultyChanged(int index)
{
    // This can be used for difficulty-specific logic
    Q_UNUSED(index)
}

void CustomQuizMaker::updateQuestionsList()
{
    m_questionsList->clear();
    for (int i = 0; i < m_questions.size(); ++i) {
        const QuizQuestion& question = m_questions[i];
        QString itemText = QString("%1. %2 (%3)")
                          .arg(i + 1)
                          .arg(question.getQuestion().left(50))
                          .arg(question.difficultyToString());
        m_questionsList->addItem(itemText);
    }
}

void CustomQuizMaker::updateCorrectAnswerCombo()
{
    m_correctAnswerCombo->clear();
    for (int i = 0; i < m_optionsList->count(); ++i) {
        m_correctAnswerCombo->addItem(m_optionsList->item(i)->text());
    }
}

void CustomQuizMaker::loadQuestionToForm(int index)
{
    if (index < 0 || index >= m_questions.size()) return;
    
    const QuizQuestion& question = m_questions[index];
    
    m_questionEdit->setText(question.getQuestion());
    m_difficultyCombo->setCurrentIndex(static_cast<int>(question.getDifficulty()));
    
    m_optionsList->clear();
    for (const QString& option : question.getOptions()) {
        m_optionsList->addItem(option);
    }
    
    updateCorrectAnswerCombo();
    m_correctAnswerCombo->setCurrentText(question.getCorrectAnswer());
}

void CustomQuizMaker::clearQuestionForm()
{
    m_questionEdit->clear();
    m_difficultyCombo->setCurrentIndex(0);
    m_optionsList->clear();
    m_correctAnswerCombo->clear();
    m_optionEdit->clear();
    m_currentQuestionIndex = -1;
    m_addQuestionButton->setText("Add Question");
    m_editQuestionButton->setEnabled(false);
}

bool CustomQuizMaker::validateQuestionForm()
{
    if (m_questionEdit->toPlainText().trimmed().isEmpty()) {
        showError("Please enter a question!");
        return false;
    }
    
    if (m_optionsList->count() < 2) {
        showError("Please add at least 2 options!");
        return false;
    }
    
    if (m_correctAnswerCombo->currentText().isEmpty()) {
        showError("Please select a correct answer!");
        return false;
    }
    
    return true;
}

void CustomQuizMaker::showError(const QString& message)
{
    QMessageBox::warning(this, "Error", message);
}

void CustomQuizMaker::showSuccess(const QString& message)
{
    QMessageBox::information(this, "Success", message);
} 