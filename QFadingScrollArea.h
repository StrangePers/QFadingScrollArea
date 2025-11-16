#pragma once

#include <QScrollArea>
#include <QTimer>

class QFadingScrollArea : public QScrollArea
{
    Q_OBJECT
public:
    explicit QFadingScrollArea(QWidget *parent = nullptr);
    explicit QFadingScrollArea(QWidget *widget, QWidget *parent = nullptr);

    // Высота градиента сверху/снизу в пикселях
    void setFadeHeight(int h);
    int  fadeHeight() const { return m_fadeHeight; }

    // Включить/выключить эффект
    void setFadeEnabled(bool on);
    bool isFadeEnabled() const { return m_fadeEnabled; }

    // Время в мс, сколько градиент остаётся после окончания скролла
    void setFadeTimeout(int ms);
    int  fadeTimeout() const { return m_fadeTimeout; }

protected:
    // Здесь ловим перерисовку viewport
    bool viewportEvent(QEvent *event) override;

private slots:
    void onScrollTimeout();

private:
    void startScrollEffect();
    bool isScrollable() const;
    void paintFadeOverlay();

    QTimer m_scrollTimer;
    bool   m_scrolling   = false;
    bool   m_fadeEnabled = true;
    int    m_fadeHeight  = 24;   // px, сверху и снизу
    int    m_fadeTimeout = 250;  // мс
};
