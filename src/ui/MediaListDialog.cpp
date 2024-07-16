#include "ui/MediaListDialog.hpp"
#include "./ui_MediaListDialog.h"
#include <QFileDialog>


MediaListDialog::MediaListDialog(QWidget *parent)
    : ui(new Ui::MediaListDialog)
    , _db(new PlayerDB("data.db"))
    , _audio_model(new QStringListModel)
    , _video_model(new QStringListModel)
    , _audio_details_model(new QStringListModel)
    , _video_details_model(new QStringListModel)
    , _menu(new ListContextMenu)
{
    ui->setupUi(this);
    init();
}

MediaListDialog::~MediaListDialog()
{
    delete ui;
    delete _db;
    delete _audio_model;
    delete _video_model;
    delete _audio_details_model;
    delete _video_details_model;
    delete _menu;
}

void MediaListDialog::init()
{
    QStringList all_lists = _db->play_lists();
    for (const QString &s : all_lists)
    {
        if (s.front() == 'A')
        {
            _audio_list.append(s.right(s.length() - 1));
        }
        else if (s.front() == 'V')
        {
            _video_list.append(s.right(s.length() - 1));
        }
    }
    _audio_model->setStringList(_audio_list);
    _video_model->setStringList(_video_list);

    ui->audio_list->setModel(_audio_model);
    ui->video_list->setModel(_video_model);
    ui->audio_details->setModel(_audio_details_model);
    ui->video_details->setModel(_video_details_model);

    QObject::connect(_audio_model, &QStringListModel::dataChanged, this, &MediaListDialog::rename_list);
    QObject::connect(_video_model, &QStringListModel::dataChanged, this, &MediaListDialog::rename_list);
    QObject::connect(ui->audio_list, &QListView::clicked, this, [this](const QModelIndex& index){if (_audio_model->rowCount() == 0){return;} _index = index; show_details(index);});
    QObject::connect(ui->video_list, &QListView::clicked, this, [this](const QModelIndex& index){if (_video_model->rowCount() == 0){return;} _index = index; show_details(index);});
    QObject::connect(ui->select_a_button, &QToolButton::clicked, this, [this](){emit list_selected(&_audio_details_list, &_audio_path, 0); close();});
    QObject::connect(ui->select_v_button, &QToolButton::clicked, this, [this](){emit list_selected(&_video_details_list, &_video_path, 1); close();});
    QObject::connect(ui->audio_details, &QListView::customContextMenuRequested, this, [this](const QPoint &pos){_hover_pos = pos; _menu->exec(QCursor::pos());});
    QObject::connect(ui->video_details, &QListView::customContextMenuRequested, this, [this](const QPoint &pos){_hover_pos = pos; _menu->exec(QCursor::pos());});
    QObject::connect(_menu, &ListContextMenu::action_code, this, &MediaListDialog::listMenuEvent);
}

void MediaListDialog::new_list()
{
    QString name;
    switch (ui->tabWidget->currentIndex())
    {
    case 0:
        name = std::string("ANew_Audio_List_").append(std::to_string(_audio_model->rowCount())).c_str();
        _db->build_list(name.toStdString().c_str());
        _audio_list.append(name.right(name.length() - 1));
        _audio_model->insertRow(_audio_model->rowCount());
        _audio_model->setData(_audio_model->index(_audio_model->rowCount() - 1), name.right(name.length() -1));
        break;
    case 1:
        name = std::string("VNew_Video_List_").append(std::to_string(_video_model->rowCount())).c_str();
        _db->build_list(name.toStdString().c_str());
        _video_list.append(name.right(name.length() - 1));
        _video_model->insertRow(_video_model->rowCount());
        _video_model->setData(_video_model->index(_video_model->rowCount() - 1), name.right(name.length() - 1));
        break;
    default:
        break;
    }
}

void MediaListDialog::append_files()
{
    if (!_index.isValid())
    {
        return;
    }
    QFileDialog *dialog = new QFileDialog();
    dialog->setModal(true);
    dialog->setFileMode(QFileDialog::ExistingFiles);
    QStringList files;
    switch (ui->tabWidget->currentIndex())
    {
    case 0:
        files = dialog->getOpenFileNames(dialog, nullptr, _db->get_param("'audio_path'"), "Audios (*.mp3 *.aac *.wma *.wav *.ape *.flac *.ogg);;All Files (*)");
        break;
    case 1:
        files = dialog->getOpenFileNames(dialog, nullptr, _db->get_param("'video_path'"), "Videos (*.mp4 *.flv *.avi *.mov *.mkv);;All Files (*)");
        break;
    default:
        files = dialog->getOpenFileNames(dialog);
        break;
    }
     if (!files.isEmpty())
    {
        switch (ui->tabWidget->currentIndex())
        {
        case 0:
            for (const QString& file : files)
            {
                QFileInfo info(file);
                if (!_audio_path.isEmpty() && _audio_details_list.lastIndexOf(info.fileName()) != -1
                        && _audio_path.lastIndexOf(info.canonicalPath()) == _audio_details_list.lastIndexOf(info.fileName()))
                {
                    continue;
                }
                _audio_path.append(info.canonicalPath());
                _audio_details_list.append(info.fileName());
            }
            _audio_details_model->setStringList(_audio_details_list);
            _db->append_medias('A' + _index.data().toString(), _audio_details_list, _audio_path);
            break;
        case 1:
            for (const QString& file : files)
            {
                QFileInfo info(file);
                if (!_video_path.isEmpty() && _video_details_list.lastIndexOf(info.fileName()) != -1 
                        && _video_path.lastIndexOf(info.canonicalPath()) == _video_details_list.lastIndexOf(info.fileName()))
                {
                    continue;
                }
                _video_path.append(info.canonicalPath());
                _video_details_list.append(info.fileName());
            }
            _video_details_model->setStringList(_video_details_list);
            _db->append_medias('V' + _index.data().toString(), _video_details_list, _video_path);
            break;
        default:
            break;
        }
    }
    delete dialog;
}

void MediaListDialog::drop_list()
{
    QModelIndex index;
    switch (ui->tabWidget->currentIndex())
    {
    case 0:
        index = ui->audio_list->currentIndex();
        if (!index.isValid())
        {
            break;
        }
        _db->remove_list(('A'+index.data().toString()).toStdString().c_str());
        _audio_list.removeAt(index.row());
        _audio_model->removeRow(index.row());
        break;
    case 1:
        index = ui->video_list->currentIndex();
        if (!index.isValid())
        {
            break;
        }
        _db->remove_list(('V'+index.data().toString()).toStdString().c_str());
        _video_list.removeAt(index.row());
        _video_model->removeRow(index.row());
        break;
    default:
        break;
    }
}

void MediaListDialog::rename_list(const QModelIndex &index)
{
    switch (ui->tabWidget->currentIndex())
    {
    case 0:
        _db->rename_list(('A'+_audio_list[index.row()]).toStdString().c_str(), ('A'+index.data().toString()).toStdString().c_str());
        _audio_list[index.row()] = index.data().toString();
        break;
    case 1:
        _db->rename_list(('V'+_video_list[index.row()]).toStdString().c_str(), ('V'+index.data().toString()).toStdString().c_str());
        _video_list[index.row()] = index.data().toString();
        break;
    default:
        break;
    }
}

void MediaListDialog::show_details(const QModelIndex &index)
{
    switch (ui->tabWidget->currentIndex())
    {
    case 0:
        _db->get_medias('A'+index.data().toString(), _audio_details_list, _audio_path);
        _audio_details_model->setStringList(_audio_details_list);
        break;
    case 1:
        _db->get_medias('V'+index.data().toString(), _video_details_list, _video_path);
        _video_details_model->setStringList(_video_details_list);
        break;    
    default:
        break;
    }
}






void MediaListDialog::new_list(const int& index, const QStringList& files, const QStringList& paths)
{
    QString name;
    ui->tabWidget->setCurrentIndex(index);
    switch (index)
    {
    case 0:
        name = std::string("ANew_Audio_List_").append(std::to_string(_audio_model->rowCount())).c_str();
        _db->build_list(name.toStdString().c_str());
        _audio_list.append(name.right(name.length() - 1));
        _audio_model->insertRow(_audio_model->rowCount());
        _audio_model->setData(_audio_model->index(_audio_model->rowCount() - 1), name.right(name.length() -1));
        _index = _audio_model->index(_audio_model->rowCount() - 1);
        for (QStringList::const_iterator file_it = files.begin(), file_end = files.end(), path_it = paths.begin(), path_end = paths.end(); file_it != file_end; ++file_it, ++path_it)
        {
            if (!_audio_path.isEmpty() && _audio_list.lastIndexOf(*file_it) != -1
                    && _audio_path.lastIndexOf(*path_it) == _audio_list.lastIndexOf(*file_it))
            {
                continue;
            }
            _audio_path.append(*path_it);
            _audio_details_list.append(*file_it);
        }
        _audio_details_model->setStringList(_audio_details_list);
        _db->append_medias('A' + _index.data().toString(), _audio_details_list, _audio_path);
        ui->audio_list->setCurrentIndex(_index);
        break;
    case 1:
        name = std::string("VNew_Video_List_").append(std::to_string(_video_model->rowCount())).c_str();
        _db->build_list(name.toStdString().c_str());
        _video_list.append(name.right(name.length() - 1));
        _video_model->insertRow(_video_model->rowCount());
        _video_model->setData(_video_model->index(_video_model->rowCount() - 1), name.right(name.length() - 1));
        _index = _video_model->index(_video_model->rowCount() - 1);
        for (QStringList::const_iterator file_it = files.begin(), file_end = files.end(), path_it = paths.begin(), path_end = paths.end(); file_it != file_end; ++file_it, ++path_it)
        {
            if (!_video_path.isEmpty() && _video_list.lastIndexOf(*file_it) != -1
                    && _video_path.lastIndexOf(*path_it) == _video_list.lastIndexOf(*file_it))
            {
                continue;
            }
            _video_path.append(*path_it);
            _video_details_list.append(*file_it);
        }
        _video_details_model->setStringList(_video_details_list);
        _db->append_medias('V' + _index.data().toString(), _video_details_list, _video_path);
        ui->video_list->setCurrentIndex(_index);
        break;
    default:
        break;
    }
}




void MediaListDialog::listMenuEvent(const int& code)
{
    QModelIndex index;
    size_t row;
    switch (ui->tabWidget->currentIndex())
    {
    case 0:
        index = ui->audio_details->indexAt(_hover_pos);
        row = index.row();
        switch (code)
        {
        case 1:
            if (!index.isValid())
            {
                return;
            }
            _db->remove_media('A'+_index.data().toString(), _audio_details_list.at(row), _audio_path.at(row));
            _audio_details_model->removeRow(row);
            _audio_details_list.remove(row);
            _audio_path.remove(row);
            break;
        case 3:
            append_files();
            break;
        default:
            break;
        }
        break;
    case 1:
        index = ui->video_details->indexAt(_hover_pos);
        row = index.row();
        switch (code)
        {
        case 1:
            if (!index.isValid())
            {
                return;
            }
            _db->remove_media('V'+_index.data().toString(), _video_details_list.at(row), _video_path.at(row));
            _video_details_model->removeRow(row);
            _video_details_list.remove(row);
            _video_path.remove(row);
            break;
        case 3:
            append_files();
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}