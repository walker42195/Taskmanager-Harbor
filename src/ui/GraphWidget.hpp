#ifndef GRAPHWIDGET_HPP
#define GRAPHWIDGET_HPP

#include <iterator>
#include <QWidget>
#include <QColor>
#include <QString>
#include <deque>

namespace Harbor {

class GraphWidget : public QWidget {
    Q_OBJECT

public:
    explicit GraphWidget(QWidget *parent = nullptr);
    ~GraphWidget() override = default;

    void setColors(const QColor &lineColor, const QColor &fillColor);
    void setSecondaryColors(const QColor &lineColor, const QColor &fillColor); // For dual lines (e.g. RX/TX)
    void setDualLabels(const QString &label1, const QString &label2); // Custom labels instead of IN/OUT
    void setTitle(const QString &title);
    void setUnit(const QString &unit);
    void setCustomValueText(const QString &text);
    void setRange(double minVal, double maxVal, bool autoScale = false);
    void setMaxHistoryLength(int length);

    void addDataPoint(double value);
    void addDualDataPoint(double val1, double val2); // For combined graphs (e.g. In/Out traffic)

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void drawSingleGraph(QPainter &painter, const std::deque<double> &data, const QColor &lineColor, const QColor &fillColor, double effectiveMax);

    QString m_title;
    QString m_unit;
    QString m_customValueText;
    QString m_label1{"IN"};
    QString m_label2{"OUT"};
    QColor m_lineColor{0, 210, 255};   // Electric Cyan default
    QColor m_fillColor{0, 210, 255, 40};

    bool m_hasDualLine{false};
    QColor m_lineColor2{255, 0, 128};  // Neon Pink/Magenta
    QColor m_fillColor2{255, 0, 128, 30};

    double m_minVal{0.0};
    double m_maxVal{100.0};
    bool m_autoScale{false};
    int m_maxHistory{60};

    std::deque<double> m_dataHistory;
    std::deque<double> m_dataHistory2;
};

} // namespace Harbor

#endif // GRAPHWIDGET_HPP
