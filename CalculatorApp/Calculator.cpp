#include "Calculator.h"

Calculator::Calculator(QObject *parent)
    : QObject{parent}
{}

QString Calculator::displayText() const {
    return m_display;
}

void Calculator::pressButton(const QString &label) {

    // --- Clear ---
    if (label == "C") {
        m_display   = "0";
        m_operand1  = 0;
        m_operator  = "";
        m_newInput  = true;
        emit displayTextChanged();
        return;
    }

    // --- Delete ---
    if (label == "DEL") {
        if (m_newInput || m_display == "0" || m_display == "Error") {
            // nothing to delete — already at a clean state
            return;
        }

        if (m_display.length() == 1) {
            // last digit — go back to "0" instead of leaving an empty string
            m_display = "0";
            m_newInput = true;
        } else if (m_display.length() == 2 && m_display.startsWith("-")) {
            // only a minus sign left after deleting — e.g. "-5" → "-" → "0"
            m_display = "0";
            m_newInput = true;
        } else {
            m_display.chop(1);    // remove last character normally
        }

        emit displayTextChanged();
        return;
    }

    // --- Equals ---
    if (label == "=") {
        evaluate();
        m_operator = "";
        m_newInput = true;
        return;
    }

    // --- Operator (+, -, *, /) ---
    if (label == "+" || label == "-" ||
        label == "×" || label == "÷") {
        m_operand1 = m_display.toDouble();
        m_operator = label;
        m_newInput = true;
        return;
    }

    // --- Digit or decimal ---
    if (m_newInput) {
        m_display  = (label == ".") ? "0." : label;
        m_newInput = false;
    } else {
        if (label == "." && m_display.contains("."))
            return;                         // prevent double decimal
        m_display += label;
    }

    emit displayTextChanged();              // tell QML to refresh
}

void Calculator::evaluate() {
    if (m_operator.isEmpty()) return;

    m_operand2 = m_display.toDouble();
    double result = 0;

    if      (m_operator == "+") result = m_operand1 + m_operand2;
    else if (m_operator == "-") result = m_operand1 - m_operand2;
    else if (m_operator == "×") result = m_operand1 * m_operand2;
    else if (m_operator == "÷") {
        if (m_operand2 == 0) { m_display = "Error"; emit displayTextChanged(); return; }
        result = m_operand1 / m_operand2;
    }

    // Remove trailing zeros cleanly
    m_display = QString::number(result, 'g', 10);
    emit displayTextChanged();
}
