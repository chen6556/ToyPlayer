#include "ui/ListSelectDialog.hpp"
#include "./ui_ListSelectDialog.h"


ListSelectDialog::ListSelectDialog(const int& code, QWidget* parent)
    :QDialog(parent)
    , ui(new Ui::ListSelectDialog)
    , _db(new PlayerDB("data.db"))
    , _model(new QStringListModel)
{
    ui->setupUi(this);
    init(code);
}

ListSelectDialog::~ListSelectDialog()
{
    delete _db;
    delete _model;
}

void ListSelectDialog::init(const int& code)
{
    QStringList audio_lists, video_lists, all_lists = _db->play_lists();
    for (const QString &s : all_lists)
    {
        if (s.front() == 'A')
        {
            audio_lists.append(s.right(s.length() - 1));
        }
        else if (s.front() == 'V')
        {
            video_lists.append(s.right(s.length() - 1));
        }
    }
    switch (code)
    {
    case 0:
        _model->setStringList(audio_lists);
        break;
    case 1:
        _model->setStringList(video_lists);
        break;
    default:
        break;
    }
    ui->listView->setModel(_model);

    QObject::connect(ui->listView, &QListView::clicked, [this](const QModelIndex& index)
                                                        {
                                                            _index = index;
                                                            ui->label->setText(" 添加到：" + index.data().toString());
                                                        });
    QObject::connect(this, &ListSelectDialog::accepted, [this](){emit append_to(_index.data().toString());});
}