#pragma once
#include <QMenu>
#include <QWidget>
#include <QAction>
#include <QPoint>


class ListContextMenu: public QMenu
{
    Q_OBJECT
public:
    ListContextMenu(QWidget *parent = nullptr);
    ~ListContextMenu();

public: signals:
    void action_code(const int&); // 1:移除 2:清空 3:添加 4:保存为播放列表 5:添加到

private:
    void init();

private:
    QAction *_remove;
    QAction *_clear;
    QAction *_append;
    QAction *_save;
    QAction *_append_to;
};