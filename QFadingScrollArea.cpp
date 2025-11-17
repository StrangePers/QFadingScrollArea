#include "QFadingScrollArea.h"

#include <QScrollBar>
#include <QPainter>
#include <QEvent>
#include <QShowEvent>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QApplication>
#include <QPalette>
#include <QListView>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <algorithm>

// Реализация FadeOverlay
FadeOverlay::FadeOverlay(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_AlwaysStackOnTop, true);
}

void FadeOverlay::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    // Получаем QFadingScrollArea через родителя
    QFadingScrollArea *scrollArea = nullptr;
    QWidget *parentWidget = qobject_cast<QWidget*>(parent());
    if (!parentWidget)
        return;
    
    // Если родитель - viewport, получаем QScrollArea через viewport->parent()
    // Если родитель - сам QScrollArea, используем его напрямую
    scrollArea = qobject_cast<QFadingScrollArea*>(parentWidget);
    if (!scrollArea) {
        QWidget *vp = parentWidget;
        scrollArea = qobject_cast<QFadingScrollArea*>(vp->parent());
    }
    
    if (!scrollArea)
        return;
    
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);
    
    // Рисуем градиенты (проверка isScrollable внутри paintFadeOverlay)
    if (scrollArea->isFadeEnabled()) {
        scrollArea->paintFadeOverlay(&p);
    }
}

QFadingScrollArea::QFadingScrollArea(QWidget *parent)
    : QScrollArea(parent)
{
    setWidgetResizable(true);
    // Устанавливаем атрибут, чтобы QScrollArea перерисовывался
    setAttribute(Qt::WA_OpaquePaintEvent, false);

    m_scrollTimer.setSingleShot(true);
    m_scrollTimer.setInterval(m_fadeTimeout);
    connect(&m_scrollTimer, &QTimer::timeout,
            this, &QFadingScrollArea::onScrollTimeout);

    // Отслеживаем вертикальный скролл
    connect(verticalScrollBar(), &QScrollBar::valueChanged,
            this, [this](int){
        startScrollEffect();
        // Принудительно обновляем overlay через таймер (несколько раз для надёжности)
        QTimer::singleShot(0, this, [this]() {
            if (m_overlay) {
                m_overlay->raise();
                m_overlay->update();
            }
            // Также обновляем сам QScrollArea для отрисовки градиентов в paintEvent
            update();
        });
        QTimer::singleShot(10, this, [this]() {
            if (m_overlay) {
                m_overlay->raise();
                m_overlay->update();
            }
            // Также обновляем сам QScrollArea для отрисовки градиентов в paintEvent
            update();
        });
    });

    // Более плавный скролл
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    // Overlay будет создан в showEvent, когда viewport точно будет готов
}

QFadingScrollArea::QFadingScrollArea(QWidget *widget, QWidget *parent)
    : QScrollArea(parent)
{
    setWidgetResizable(true);
    // Устанавливаем атрибут, чтобы QScrollArea перерисовывался
    setAttribute(Qt::WA_OpaquePaintEvent, false);

    m_scrollTimer.setSingleShot(true);
    m_scrollTimer.setInterval(m_fadeTimeout);
    connect(&m_scrollTimer, &QTimer::timeout,
            this, &QFadingScrollArea::onScrollTimeout);

    // Отслеживаем вертикальный скролл
    connect(verticalScrollBar(), &QScrollBar::valueChanged,
            this, [this](int){
        startScrollEffect();
        // Принудительно обновляем overlay через таймер (несколько раз для надёжности)
        QTimer::singleShot(0, this, [this]() {
            if (m_overlay) {
                m_overlay->raise();
                m_overlay->update();
            }
            // Также обновляем сам QScrollArea для отрисовки градиентов в paintEvent
            update();
        });
        QTimer::singleShot(10, this, [this]() {
            if (m_overlay) {
                m_overlay->raise();
                m_overlay->update();
            }
            // Также обновляем сам QScrollArea для отрисовки градиентов в paintEvent
            update();
        });
    });

    // Более плавный скролл
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    setWidget(widget);
    // Overlay будет создан в showEvent, когда viewport точно будет готов
}

void QFadingScrollArea::showEvent(QShowEvent *event)
{
    QScrollArea::showEvent(event);
    // Принудительно создаём overlay после показа
    QTimer::singleShot(0, this, [this]() {
        setupOverlay();
        updateOverlayGeometry();
    });
}

void QFadingScrollArea::resizeEvent(QResizeEvent *event)
{
    QScrollArea::resizeEvent(event);
    updateOverlayGeometry();
    // Также обновляем overlay при изменении размера
    QTimer::singleShot(0, this, [this]() {
        if (m_overlay) {
            m_overlay->raise();
            m_overlay->update();
        }
    });
}

void QFadingScrollArea::setupOverlay()
{
    if (!viewport())
        return;
        
    if (!m_overlay) {
        // Для ListView создаём overlay как дочерний виджет самого QScrollArea
        // чтобы он был поверх всего содержимого
        if (isListView()) {
            m_overlay = new FadeOverlay(this);
        } else {
            m_overlay = new FadeOverlay(viewport());
        }
        
        m_overlay->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        m_overlay->setAttribute(Qt::WA_NoSystemBackground, true);
        m_overlay->setAttribute(Qt::WA_AlwaysStackOnTop, true);
        // Устанавливаем высокий z-order для overlay
        m_overlay->lower();
        updateOverlayGeometry();
        m_overlay->show();
        m_overlay->raise(); // Поднимаем overlay поверх всех дочерних виджетов
        m_overlay->stackUnder(nullptr); // Убеждаемся, что overlay наверху
        
        // Устанавливаем event filter на viewport для обновления overlay
        viewport()->installEventFilter(this);
        
        // Также устанавливаем event filter на сам widget, если это ListView
        if (widget()) {
            widget()->installEventFilter(this);
        }
    }
}

void QFadingScrollArea::updateOverlayGeometry()
{
    if (m_overlay && viewport()) {
        if (isListView()) {
            // Для ListView используем координаты viewport'а в системе координат QScrollArea
            QPoint vpTopLeft = viewport()->mapTo(this, QPoint(0, 0));
            QRect vpRect(vpTopLeft, viewport()->size());
            m_overlay->setGeometry(vpRect);
        } else {
            m_overlay->setGeometry(viewport()->rect());
        }
        m_overlay->raise(); // Поднимаем overlay поверх всех дочерних виджетов
        // Принудительно обновляем overlay
        QTimer::singleShot(0, this, [this]() {
            if (m_overlay) {
                m_overlay->raise();
                if (isListView()) {
                    m_overlay->stackUnder(nullptr);
                }
                m_overlay->update();
            }
        });
    }
}

void QFadingScrollArea::setFadeHeight(int h)
{
    h = std::max(0, h);
    if (m_fadeHeight == h)
        return;

    m_fadeHeight = h;
    if (m_overlay)
        m_overlay->update();
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

    if (m_overlay) {
        m_overlay->setVisible(m_fadeEnabled);
        m_overlay->update();
    }
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

    // Для ListView используем другой способ проверки скроллируемости
    if (isListView()) {
        QListView *listView = qobject_cast<QListView*>(widget());
        if (listView) {
            // Проверяем, есть ли элементы в модели
            QAbstractItemModel *model = listView->model();
            if (model && model->rowCount() > 0) {
                // Проверяем размер содержимого
                int viewportHeight = viewport()->height();
                int rowHeight = listView->sizeHintForRow(0);
                int contentsHeight = rowHeight * model->rowCount();
                return contentsHeight > viewportHeight;
            }
        }
    }

    return sb->maximum() > 0;
}

bool QFadingScrollArea::isListView() const
{
    QWidget *w = widget();
    if (!w)
        return false;
    
    // Проверяем, является ли widget QListView
    return qobject_cast<QListView*>(w) != nullptr;
}

bool QFadingScrollArea::shouldShowTopFade() const
{
    if (!isScrollable())
        return false;
    
    // Для ListView используем другой способ проверки
    if (isListView()) {
        QListView *listView = qobject_cast<QListView*>(widget());
        if (listView) {
            // Проверяем, виден ли первый элемент полностью
            QModelIndex firstIndex = listView->indexAt(QPoint(0, 0));
            if (firstIndex.isValid()) {
                QRect firstRect = listView->visualRect(firstIndex);
                return firstRect.top() < 0;
            }
        }
    }
    
    const QScrollBar *sb = verticalScrollBar();
    return sb->value() > 0;
}

bool QFadingScrollArea::shouldShowBottomFade() const
{
    if (!isScrollable())
        return false;
    
    // Для ListView используем другой способ проверки
    if (isListView()) {
        QListView *listView = qobject_cast<QListView*>(widget());
        if (listView) {
            QAbstractItemModel *model = listView->model();
            if (model && model->rowCount() > 0) {
                // Проверяем, виден ли последний элемент полностью
                QModelIndex lastIndex = model->index(model->rowCount() - 1, 0);
                if (lastIndex.isValid()) {
                    QRect lastRect = listView->visualRect(lastIndex);
                    QRect viewportRect = listView->viewport()->rect();
                    return lastRect.bottom() > viewportRect.bottom();
                }
            }
        }
    }
    
    const QScrollBar *sb = verticalScrollBar();
    return sb->value() < sb->maximum();
}

void QFadingScrollArea::startScrollEffect()
{
    if (!m_fadeEnabled || !isScrollable())
        return;

    m_scrolling = true;
    m_scrollTimer.start();
    // Принудительно обновляем overlay при скролле
    QTimer::singleShot(0, this, [this]() {
        if (m_overlay) {
            m_overlay->raise();
            m_overlay->update();
        }
        // Также обновляем сам QScrollArea для отрисовки градиентов в paintEvent
        update();
    });
}

void QFadingScrollArea::onScrollTimeout()
{
    m_scrolling = false;
    // Принудительно обновляем overlay после окончания скролла
    QTimer::singleShot(0, this, [this]() {
        if (m_overlay) {
            m_overlay->raise();
            m_overlay->update();
        }
        // Также обновляем сам QScrollArea для отрисовки градиентов в paintEvent
        update();
    });
}

bool QFadingScrollArea::eventFilter(QObject *obj, QEvent *event)
{
    // Обновляем overlay при каждом paintEvent viewport'а
    if (obj == viewport()) {
        if (event->type() == QEvent::Paint) {
            // Даём viewport отрисоваться
            viewport()->removeEventFilter(this);
            QApplication::sendEvent(obj, event);
            viewport()->installEventFilter(this);
            
            // Обновляем overlay после отрисовки viewport (несколько раз для надёжности)
            QTimer::singleShot(0, this, [this]() {
                if (m_overlay) {
                    m_overlay->raise();
                    m_overlay->stackUnder(nullptr);
                    m_overlay->update();
                }
                // Также обновляем QScrollArea для отрисовки градиентов в paintEvent
                update();
            });
            QTimer::singleShot(5, this, [this]() {
                if (m_overlay) {
                    m_overlay->raise();
                    m_overlay->stackUnder(nullptr);
                    m_overlay->update();
                }
                // Также обновляем QScrollArea для отрисовки градиентов в paintEvent
                update();
            });
            return true;
        } else if (event->type() == QEvent::Resize) {
            // Обновляем геометрию overlay при изменении размера viewport
            updateOverlayGeometry();
        }
    }
    // Также обновляем overlay при paintEvent самого widget'а (для ListView)
    else if (obj == widget() && event->type() == QEvent::Paint) {
        // Для ListView принудительно обновляем overlay после каждого paintEvent widget'а
        QTimer::singleShot(0, this, [this]() {
            if (m_overlay) {
                m_overlay->raise();
                m_overlay->stackUnder(nullptr);
                m_overlay->update();
            }
            // Принудительно обновляем QScrollArea для отрисовки градиентов в paintEvent
            update();
        });
        QTimer::singleShot(5, this, [this]() {
            if (m_overlay) {
                m_overlay->raise();
                m_overlay->stackUnder(nullptr);
                m_overlay->update();
            }
            // Принудительно обновляем QScrollArea для отрисовки градиентов в paintEvent
            update();
        });
    }
    return QScrollArea::eventFilter(obj, event);
}

void QFadingScrollArea::paintEvent(QPaintEvent *event)
{
    QScrollArea::paintEvent(event);
    // Для ListView градиенты рисуются через overlay, а не в paintEvent QScrollArea
}

void QFadingScrollArea::paintFadeOverlay(QPainter *painter)
{
    if (!painter || !m_overlay)
        return;

    const int w = m_overlay->width();
    const int h = m_overlay->height();
    if (w <= 0 || h <= 0)
        return;

    // Проверяем, можно ли скроллить
    // Для ListView проверяем isScrollable, но не возвращаемся, если false
    // чтобы градиенты всегда рисовались, если включены
    bool scrollable = isScrollable();
    bool isList = isListView();
    
    if (!scrollable && !isList)
        return;

    int fade = std::min(m_fadeHeight, h / 2);
    if (fade <= 0)
        return;

    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);

    // Цвет фона — из палитры viewport'а или родительского виджета
    QWidget *vp = viewport();
    QColor base = vp->palette().color(QPalette::Base);
    if (!base.isValid() || base.alpha() == 0) {
        base = palette().color(QPalette::Base);
    }
    if (!base.isValid() || base.alpha() == 0) {
        base = palette().color(QPalette::Window);
    }
    // Если всё ещё не валидный, используем белый по умолчанию
    if (!base.isValid()) {
        base = Qt::white;
    }

    QColor opaque = base;
    opaque.setAlpha(255);

    QColor transparent = base;
    transparent.setAlpha(0);

    // Верхний градиент: показываем только если не в самом верху
    // Для ListView всегда показываем, если включены и можно скроллить
    bool showTop = isList ? (scrollable && shouldShowTopFade()) : shouldShowTopFade();
    if (showTop) {
        QLinearGradient gradTop(0, 0, 0, fade);
        gradTop.setColorAt(0.0, opaque);
        gradTop.setColorAt(1.0, transparent);
        painter->fillRect(0, 0, w, fade, gradTop);
    }

    // Нижний градиент: показываем только если не в самом низу
    // Для ListView всегда показываем, если включены и можно скроллить
    bool showBottom = isList ? (scrollable && shouldShowBottomFade()) : shouldShowBottomFade();
    if (showBottom) {
        QLinearGradient gradBottom(0, h - fade, 0, h);
        gradBottom.setColorAt(0.0, transparent);
        gradBottom.setColorAt(1.0, opaque);
        painter->fillRect(0, h - fade, w, fade, gradBottom);
    }
}
