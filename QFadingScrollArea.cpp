#include "QFadingScrollArea.h"

#include <QScrollBar>
#include <QPainter>
#include <QEvent>
#include <QPalette>
#include <algorithm>

QFadingScrollArea::QFadingScrollArea(QWidget *parent)
    : QScrollArea(parent)
{
    setWidgetResizable(true);

    m_scrollTimer.setSingleShot(true);
    m_scrollTimer.setInterval(m_fadeTimeout);
    connect(&m_scrollTimer, &QTimer::timeout,
            this, &QFadingScrollArea::onScrollTimeout);

    // Отслеживаем вертикальный скролл
    connect(verticalScrollBar(), &QScrollBar::valueChanged,
            this, [this](int){
        startScrollEffect();
    });

    // Более плавный скролл
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

QFadingScrollArea::QFadingScrollArea(QWidget *widget, QWidget *parent)
    : QFadingScrollArea(parent)
{
    setWidget(widget);
}

void QFadingScrollArea::setFadeHeight(int h)
{
    h = std::max(0, h);
    if (m_fadeHeight == h)
        return;

    m_fadeHeight = h;
    if (viewport())
        viewport()->update();
}

void QFadingScrollArea::setFadeEnabled(bool on)
{
    if (m_fadeEnabled == on)
        return;

    m_fadeEnabled = on;
    if (!m_fadeEnabled) {
        m_scrolling = false;
        m_scrollTimer.stop();
    }

    if (viewport())
        viewport()->update();
}

void QFadingScrollArea::setFadeTimeout(int ms)
{
    if (ms <= 0)
        ms = 1;

    m_fadeTimeout = ms;
    m_scrollTimer.setInterval(m_fadeTimeout);
}

bool QFadingScrollArea::isScrollable() const
{
    const QScrollBar *sb = verticalScrollBar();
    if (!sb)
        return false;

    return sb->maximum() > 0;
}

void QFadingScrollArea::startScrollEffect()
{
    if (!m_fadeEnabled || !isScrollable())
        return;

    m_scrolling = true;
    m_scrollTimer.start();
    if (viewport())
        viewport()->update();
}

void QFadingScrollArea::onScrollTimeout()
{
    m_scrolling = false;
    if (viewport())
        viewport()->update();
}

bool QFadingScrollArea::viewportEvent(QEvent *event)
{
    // Сначала даём базовой реализации всё нарисовать/обработать
    bool handled = QScrollArea::viewportEvent(event);

    if (event->type() == QEvent::Paint) {
        // Поверх содержимого рисуем градиенты
        if (m_fadeEnabled && m_scrolling && isScrollable()) {
            paintFadeOverlay();
        }
    }

    return handled;
}

void QFadingScrollArea::paintFadeOverlay()
{
    QWidget *vp = viewport();
    if (!vp)
        return;

    const int w = vp->width();
    const int h = vp->height();
    if (w <= 0 || h <= 0)
        return;

    int fade = std::min(m_fadeHeight, h / 2);
    if (fade <= 0)
        return;

    QPainter p(vp);
    p.setRenderHint(QPainter::Antialiasing, false);

    // Цвет фона — из палитры viewport’а
    QColor base = vp->palette().color(QPalette::Base);
    if (!base.isValid())
        base = vp->palette().color(QPalette::Window);

    QColor opaque = base;
    opaque.setAlpha(255);

    QColor transparent = base;
    transparent.setAlpha(0);

    // Верхний градиент: у самого края полностью скрывает контент
    {
        QLinearGradient gradTop(0, 0, 0, fade);
        gradTop.setColorAt(0.0, opaque);
        gradTop.setColorAt(1.0, transparent);

        p.fillRect(0, 0, w, fade, gradTop);
    }

    // Нижний градиент
    {
        QLinearGradient gradBottom(0, h - fade, 0, h);
        gradBottom.setColorAt(0.0, transparent);
        gradBottom.setColorAt(1.0, opaque);

        p.fillRect(0, h - fade, w, fade, gradBottom);
    }
}
