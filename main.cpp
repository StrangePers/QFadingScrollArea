#include "QFadingScrollArea.h"

#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QListView>
#include <QStringListModel>
#include <QGroupBox>
#include <QScrollArea>

// Пример 1: Использование с QWidget и QVBoxLayout
QWidget* createWidgetExample(QWidget *parent)
{
    auto *content = new QWidget;
    auto *layout = new QVBoxLayout(content);
    layout->setSpacing(10);
    layout->setContentsMargins(20, 20, 20, 20);

    // Добавляем много элементов, чтобы появился скролл
    for (int i = 0; i < 30; ++i) {
        auto *label = new QLabel(QString("Элемент списка номер %1").arg(i + 1));
        label->setStyleSheet("QLabel { "
                             "background-color: #e0e0e0; "
                             "padding: 10px; "
                             "border-radius: 5px; "
                             "min-height: 40px; "
                             "}");
        layout->addWidget(label);
    }

    auto *scroll = new QFadingScrollArea(parent);
    scroll->setWidget(content);

    // Настройка параметров фейда
    scroll->setFadeHeight(32);
    scroll->setFadeTimeout(300);
    scroll->setFadeEnabled(true);

    return scroll;
}

// Пример 2: Использование с QListView
QWidget* createListViewExample(QWidget *parent)
{
    auto *listView = new QListView;
    
    // Создаём модель со списком строк
    QStringList items;
    for (int i = 0; i < 50; ++i) {
        items << QString("Элемент списка %1").arg(i + 1);
    }
    
    auto *model = new QStringListModel(items, listView);
    listView->setModel(model);
    listView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    // Настройка стиля для лучшей видимости
    listView->setStyleSheet(
        "QListView { "
        "background-color: white; "
        "border: 1px solid #ccc; "
        "}"
        "QListView::item { "
        "padding: 8px; "
        "border-bottom: 1px solid #eee; "
        "}"
        "QListView::item:hover { "
        "background-color: #f0f0f0; "
        "}"
        "QListView::item:selected { "
        "background-color: #4CAF50; "
        "color: white; "
        "}"
    );

    // Создаём QFadingScrollArea с QListView
    // Используем конструктор с двумя параметрами: widget и parent
    auto *scroll = new QFadingScrollArea(listView, parent);
    
    // Настройка параметров фейда
    scroll->setFadeHeight(60);
    scroll->setFadeTimeout(400);
    scroll->setFadeEnabled(true);

    return scroll;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QMainWindow window;
    window.setWindowTitle("QFadingScrollArea - Примеры использования");
    window.resize(800, 600);

    auto *centralWidget = new QWidget;
    auto *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Левая панель: пример с QWidget
    auto *leftGroup = new QGroupBox("Пример 1: QWidget + QVBoxLayout");
    auto *leftLayout = new QVBoxLayout(leftGroup);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    
    auto *leftScroll = createWidgetExample(leftGroup);
    leftLayout->addWidget(leftScroll);
    
    // Правая панель: пример с QListView
    auto *rightGroup = new QGroupBox("Пример 2: QListView");
    auto *rightLayout = new QVBoxLayout(rightGroup);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    
    auto *rightScroll = createListViewExample(rightGroup);
    rightLayout->addWidget(rightScroll);

    mainLayout->addWidget(leftGroup, 1);
    mainLayout->addWidget(rightGroup, 1);

    window.setCentralWidget(centralWidget);
    window.show();

    return app.exec();
}

