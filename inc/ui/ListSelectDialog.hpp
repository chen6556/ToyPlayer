#pragma once
#include <QDialog>
#include <QStringListModel>
#include <QModelIndex>
#include "base/PlayerDB.hpp"


QT_BEGIN_NAMESPACE
namespace Ui { class ListSelectDialog; }
QT_END_NAMESPACE

class ListSelectDialog: public QDialog
{
    Q_OBJECT

public:
    ListSelectDialog(const int& code, QWidget* parent = nullptr);
    ~ListSelectDialog();

public: signals:
    void append_to(const QString&);

private:
    void init(const int& code);

private:
    Ui::ListSelectDialog *ui;
    PlayerDB *_db;
    QStringListModel *_model;

    QModelIndex _index;
};