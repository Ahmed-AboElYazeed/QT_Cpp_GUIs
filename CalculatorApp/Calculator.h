#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QObject>

class Calculator : public QObject
{
    Q_OBJECT                               // enables the meta-object system

    Q_PROPERTY(QString displayText          // Q_PROPERTY makes this readable AND watchable from QML
                READ displayText
                NOTIFY displayTextChanged)

public:
    explicit Calculator(QObject *parent = nullptr);     // constructor

    QString displayText() const;           // getter

    Q_INVOKABLE void pressButton(const QString &label);  // Q_INVOKABLE = callable directly from QML

signals:
    void displayTextChanged();             // QML auto-updates when this fires

private:
    QString m_display   = "0";
    double  m_operand1  = 0;
    double  m_operand2  = 0;
    QString m_operator  = "";
    bool    m_newInput  = true;

    void evaluate();
};

#endif // CALCULATOR_H
