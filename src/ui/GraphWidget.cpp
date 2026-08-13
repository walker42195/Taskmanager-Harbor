#include "GraphWidget.hpp"
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QLinearGradient>
#include <algorithm>
#include <cmath>

namespace Harbor {

GraphWidget::GraphWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(140);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void GraphWidget::setColors(const QColor &lineColor, const QColor &fillColor) {
    m_lineColor = lineColor;
    m_fillColor = fillColor;
    update();
}

void GraphWidget::setSecondaryColors(const QColor &lineColor, const QColor &fillColor) {
    m_hasDualLine = true;
    m_lineColor2 = lineColor;
    m_fillColor2 = fillColor;
    update();
}

void GraphWidget::setDualLabels(const QString &label1, const QString &label2) {
    m_label1 = label1;
    m_label2 = label2;
    update();
}

void GraphWidget::setTitle(const QString &title) {
    m_title = title;
    update();
}

void GraphWidget::setUnit(const QString &unit) {
    m_unit = unit;
    update();
}

void GraphWidget::setCustomValueText(const QString &text) {
    m_customValueText = text;
    update();
}

void GraphWidget::setRange(double minVal, double maxVal, bool autoScale) {
    m_minVal = minVal;
    m_maxVal = maxVal;
    m_autoScale = autoScale;
    update();
}

void GraphWidget::setMaxHistoryLength(int length) {
    m_maxHistory = length;
}

void GraphWidget::addDataPoint(double value) {
    m_dataHistory.push_back(value);
    while (static_cast<int>(m_dataHistory.size()) > m_maxHistory) {
        m_dataHistory.pop_front();
    }
    update();
}

void GraphWidget::addDualDataPoint(double val1, double val2) {
    m_hasDualLine = true;
    m_dataHistory.push_back(val1);
    m_dataHistory2.push_back(val2);
    while (static_cast<int>(m_dataHistory.size()) > m_maxHistory) {
        m_dataHistory.pop_front();
    }
    while (static_cast<int>(m_dataHistory2.size()) > m_maxHistory) {
        m_dataHistory2.pop_front();
    }
    update();
}

void GraphWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    int w = width();
    int h = height();

    // Dark sleek container background
    QColor cardBg(24, 28, 38);
    QColor cardBorder(38, 44, 60);

    painter.setPen(cardBorder);
    painter.setBrush(cardBg);
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 8, 8);

    // Padding for graph plotting area
    int topMargin = 36;
    int bottomMargin = 16;
    int leftMargin = 16;
    int rightMargin = 16;

    int plotW = w - leftMargin - rightMargin;
    int plotH = h - topMargin - bottomMargin;

    if (plotW <= 10 || plotH <= 10) return;

    // Determine current max scale
    double effectiveMax = m_maxVal;
    if (m_autoScale) {
        double maxInHist = 1.0;
        for (double val : m_dataHistory) maxInHist = std::max(maxInHist, val);
        for (double val : m_dataHistory2) maxInHist = std::max(maxInHist, val);
        effectiveMax = std::max(m_minVal + 1.0, maxInHist * 1.15);
    }

    // Draw Grid Lines (3 horizontal lines)
    QPen gridPen(QColor(50, 56, 75), 1, Qt::DotLine);
    painter.setPen(gridPen);
    for (int i = 0; i <= 3; ++i) {
        int y = topMargin + (plotH * i) / 3;
        painter.drawLine(leftMargin, y, leftMargin + plotW, y);
    }

    // Draw Header / Title / Current Value
    painter.setFont(QFont("Cantarell", 10, QFont::Bold));
    painter.setPen(QColor(220, 225, 235));
    painter.drawText(leftMargin, 24, m_title);

    if (!m_dataHistory.empty()) {
        double curVal = m_dataHistory.back();
        QString valStr;
        if (!m_customValueText.isEmpty()) {
            valStr = m_customValueText;
        } else if (m_unit == "%") {
            valStr = QString::number(curVal, 'f', 1) + "%";
        } else if (m_unit == "B/s") {
            if (curVal >= 1024 * 1024 * 1024) valStr = QString::number(curVal / (1024 * 1024 * 1024), 'f', 1) + " GB/s";
            else if (curVal >= 1024 * 1024) valStr = QString::number(curVal / (1024 * 1024), 'f', 1) + " MB/s";
            else if (curVal >= 1024) valStr = QString::number(curVal / 1024, 'f', 1) + " KB/s";
            else valStr = QString::number(curVal, 'f', 0) + " B/s";
        } else {
            valStr = QString::number(curVal, 'f', 1) + " " + m_unit;
        }

        if (m_hasDualLine && !m_dataHistory2.empty()) {
            double curVal2 = m_dataHistory2.back();
            QString valStr2;
            if (m_unit == "%") {
                valStr2 = QString::number(curVal2, 'f', 1) + "%";
            } else if (m_unit == "B/s") {
                if (curVal2 >= 1024 * 1024 * 1024) valStr2 = QString::number(curVal2 / (1024 * 1024 * 1024), 'f', 1) + " GB/s";
                else if (curVal2 >= 1024 * 1024) valStr2 = QString::number(curVal2 / (1024 * 1024), 'f', 1) + " MB/s";
                else if (curVal2 >= 1024) valStr2 = QString::number(curVal2 / 1024, 'f', 1) + " KB/s";
                else valStr2 = QString::number(curVal2, 'f', 0) + " B/s";
            } else {
                valStr2 = QString::number(curVal2, 'f', 1) + " " + m_unit;
            }
            valStr = QString("%1: %2  |  %3: %4").arg(m_label1, valStr, m_label2, valStr2);
        }

        painter.setFont(QFont("Cantarell", 9, QFont::Medium));
        painter.setPen(m_lineColor);
        painter.drawText(w - rightMargin - painter.fontMetrics().horizontalAdvance(valStr), 24, valStr);
    }

    // Draw Graphs
    if (m_hasDualLine && !m_dataHistory2.empty()) {
        drawSingleGraph(painter, m_dataHistory2, m_lineColor2, m_fillColor2, effectiveMax);
    }
    if (!m_dataHistory.empty()) {
        drawSingleGraph(painter, m_dataHistory, m_lineColor, m_fillColor, effectiveMax);
    }
}

void GraphWidget::drawSingleGraph(QPainter &painter, const std::deque<double> &data, const QColor &lineColor, const QColor &fillColor, double effectiveMax) {
    if (data.size() < 2) return;

    int w = width();
    int h = height();
    int topMargin = 36;
    int bottomMargin = 16;
    int leftMargin = 16;
    int rightMargin = 16;

    int plotW = w - leftMargin - rightMargin;
    int plotH = h - topMargin - bottomMargin;

    double stepX = static_cast<double>(plotW) / (m_maxHistory - 1);
    double rangeY = std::max(0.001, effectiveMax - m_minVal);

    QPainterPath linePath;
    QPolygonF fillPolygon;

    int numPoints = static_cast<int>(data.size());
    int startIndex = m_maxHistory - numPoints;

    double firstX = leftMargin + startIndex * stepX;
    double firstY = topMargin + plotH - ((std::clamp(data[0], m_minVal, effectiveMax) - m_minVal) / rangeY) * plotH;

    linePath.moveTo(firstX, firstY);
    fillPolygon << QPointF(firstX, topMargin + plotH);
    fillPolygon << QPointF(firstX, firstY);

    for (int i = 1; i < numPoints; ++i) {
        double x = leftMargin + (startIndex + i) * stepX;
        double val = std::clamp(data[i], m_minVal, effectiveMax);
        double y = topMargin + plotH - ((val - m_minVal) / rangeY) * plotH;

        // Smooth cubic curve interpolation
        double prevX = leftMargin + (startIndex + i - 1) * stepX;
        double prevVal = std::clamp(data[i - 1], m_minVal, effectiveMax);
        double prevY = topMargin + plotH - ((prevVal - m_minVal) / rangeY) * plotH;

        double cX1 = prevX + stepX * 0.5;
        double cY1 = prevY;
        double cX2 = prevX + stepX * 0.5;
        double cY2 = y;

        linePath.cubicTo(cX1, cY1, cX2, cY2, x, y);
        fillPolygon << QPointF(x, y);
    }

    double lastX = leftMargin + (startIndex + numPoints - 1) * stepX;
    fillPolygon << QPointF(lastX, topMargin + plotH);

    // Draw Gradient Fill
    QLinearGradient fillGrad(0, topMargin, 0, topMargin + plotH);
    fillGrad.setColorAt(0.0, fillColor);
    fillGrad.setColorAt(1.0, QColor(fillColor.red(), fillColor.green(), fillColor.blue(), 5));

    painter.setPen(Qt::NoPen);
    painter.setBrush(fillGrad);
    painter.drawPolygon(fillPolygon);

    // Draw Smooth Line
    QPen linePen(lineColor, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(linePen);
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(linePath);
}

} // namespace Harbor
