#include "ui/ListContextMenu.hpp"

ListContextMenu::ListContextMenu(QWidget *parent)
    : QMenu(parent)
    , _remove(new QAction("移除"))
    , _clear(new QAction("清空"))
    , _append(new QAction("添加"))
    , _save(new QAction("保存为播放列表"))
    , _append_to(new QAction("添加到"))
{
    init();
}

ListContextMenu::~ListContextMenu()
{
    delete _remove;
    delete _clear;
    delete _append;
    delete _save;
    delete _append_to;
}

void ListContextMenu::init()
{
    this->addActions({_append, _append_to, _save, _remove, _clear});
    
    QObject::connect(_remove, &QAction::triggered, this, [this](){emit action_code(1);});
    QObject::connect(_clear, &QAction::triggered, this, [this](){emit action_code(2);});
    QObject::connect(_append, &QAction::triggered, this, [this](){emit action_code(3);});
    QObject::connect(_save, &QAction::triggered, this, [this](){emit action_code(4);});
    QObject::connect(_append_to, &QAction::triggered, this, [this](){emit action_code(5);});
}