#pragma once

#include <QScrollArea>
#include <QTimer>
#include <QWidget>

class FadeOverlay : public QWidget
{
    Q_OBJECT
public:
    explicit FadeOverlay(QWidget *parent = nullptr);
    
protected:
    void paintEvent(QPaintEvent *event) override;
    
    friend class QFadingScrollArea;
};

class QFadingScrollArea : public QScrollArea
{
    Q_OBJECT
public:
    explicit QFadingScrollArea(QWidget *parent = nullptr);
    explicit QFadingScrollArea(QWidget *widget, QWidget *parent);

    // Высота градиента сверху/снизу в пикселях
    void setFadeHeight(int h);
    int  fadeHeight() const { return m_fadeHeight; }

    // Включить/выключить эффект
    void setFadeEnabled(bool on);
    bool isFadeEnabled() const { return m_fadeEnabled; }

    // Время в мс, сколько градиент остаётся после окончания скролла
    void setFadeTimeout(int ms);
    int  fadeTimeout() const { return m_fadeTimeout; }
    
    // Публичные методы для проверки состояния (для отладки)
    bool isScrollable() const;

protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onScrollTimeout();

    friend class FadeOverlay;

private:
    void setupOverlay();
    void updateOverlayGeometry();
    void startScrollEffect();
    bool shouldShowTopFade() const;
    bool shouldShowBottomFade() const;
    void paintFadeOverlay(QPainter *painter);
    bool isListView() const;
    
    FadeOverlay *m_overlay = nullptr;

    QTimer m_scrollTimer;
    bool   m_scrolling   = false;
    bool   m_fadeEnabled = true;
    int    m_fadeHeight  = 24;   // px, сверху и снизу
    int    m_fadeTimeout = 250;  // мс
};
