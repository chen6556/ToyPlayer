#include "ui/mainwindow.hpp"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QDir>
#include <QAudioDevice>
#include "ui/MediaListDialog.hpp"
#include "ui/ListSelectDialog.hpp"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , _player(new QMediaPlayer)
    , _audio_output(new QAudioOutput)
    , _video_widget(new VideoWidget)
    , _audio_model(new QStringListModel)
    , _video_model(new QStringListModel)
    , _video_dock(new VideoDock)
    , _db(new PlayerDB("data.db"))
    , _menu(new ListContextMenu)
{
    ui->setupUi(this);
    init();
    recover_state();
}

MainWindow::~MainWindow()
{
    store_state();
    delete ui;
    delete _player;
    delete _audio_output;
    delete _video_widget;
    delete _audio_model;
    delete _video_model;
    delete _video_dock;
    delete _db;
    delete _menu;
}

void MainWindow::init()
{
    _audio_output->setDevice(QAudioDevice());
    _player->setAudioOutput(_audio_output);
    _timer.start(1000);

    ui->verticalLayout_3->insertWidget(1, _video_widget);
    _video_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _video_dock->move((QGuiApplication::primaryScreen()->size().width() - _video_dock->width()) / 2,
                        QGuiApplication::primaryScreen()->size().height() - _video_dock->height() - 2);

    ui->time_label->setText(_time.currentTime().toString("hh:mm:ss"));
    _db->get_medias("stored_audio_list", _audio_list, _audio_path);
    _db->get_medias("stored_video_list", _video_list, _video_path);
    _audio_model->setStringList(_audio_list);
    _video_model->setStringList(_video_list);
    ui->audio_list->setModel(_audio_model);
    ui->video_list->setModel(_video_model);
    _player->setVideoOutput(_video_widget);
    // 进度条以秒为单位

    QObject::connect(&_timer, &QTimer::timeout, this, &MainWindow::refresh_time);
    QObject::connect(_player, &QMediaPlayer::mediaStatusChanged, this, &MainWindow::mediaStatusChangedEvent);
    QObject::connect(_player, &QMediaPlayer::positionChanged, this, &MainWindow::playPositionChangedEvent);
    QObject::connect(ui->audio_slider, &QSlider::sliderPressed, _player, &QMediaPlayer::pause);
    QObject::connect(ui->audio_slider, &QSlider::sliderPressed, this, [this]()
                        {
                            QObject::connect(ui->audio_slider, &QSlider::sliderMoved, this, &MainWindow::change_played_time_by_slider_moved);
                        });
    QObject::connect(ui->video_slider, &QSlider::sliderPressed, _player, &QMediaPlayer::pause);
    QObject::connect(ui->video_slider, &QSlider::sliderPressed, this, [this]()
                        {
                            QObject::connect(ui->video_slider, &QSlider::sliderMoved, this, &MainWindow::change_played_time_by_slider_moved);
                        });
    QObject::connect(ui->full_screen_button, &QToolButton::clicked, this, [this](){ _video_widget->setFullScreen(true);});
    QObject::connect(_video_widget, &VideoWidget::keyPress, this, &MainWindow::videoWidgetKeyPressEvent);
    QObject::connect(_video_widget, &VideoWidget::mousePress, this, &MainWindow::videoWidgetMousePressEvent);
    QObject::connect(_video_dock, &VideoDock::order, this, &MainWindow::videoDockOrderEvent);
    QObject::connect(ui->audio_list, &QListView::customContextMenuRequested, this, [this](const QPoint &pos){_hover_pos = pos; _menu->exec(QCursor::pos());});
    QObject::connect(ui->video_list, &QListView::customContextMenuRequested, this, [this](const QPoint &pos){_hover_pos = pos; _menu->exec(QCursor::pos());});
    QObject::connect(_menu, &ListContextMenu::action_code, this, &MainWindow::listMenuEvent);
    QObject::connect(_video_dock->slider(), &QSlider::sliderPressed, _player, &QMediaPlayer::pause);
    QObject::connect(_video_dock->slider(), &QSlider::sliderPressed, this, [this]()
                        {
                            QObject::connect(_video_dock->slider(), &QSlider::sliderMoved, this, &MainWindow::change_played_time_by_slider_moved);
                        });
    QObject::connect(_video_dock->slider(), &QSlider::sliderReleased, this, &MainWindow::change_pos);
    QObject::connect(_video_dock->rate(), &QDoubleSpinBox::valueChanged, this, &MainWindow::set_player_rate);
}

void MainWindow::play()
{
    switch (ui->tabWidget->currentIndex())
    {
    case 0:
        if (_audio_model->rowCount() == 0)
        {
            return;
        }
        if (!_index.isValid())
        {
            _index = _audio_model->index(0);
            ui->audio_list->setCurrentIndex(_index);
            ui->audio_info->setText(_index.data().toString());
            _player->setSource(_audio_path.at(_index.row()) + '/' + _index.data().toString());
        }
        break;
    case 1:
        if (_video_model->rowCount() == 0)
        {
            return;
        }
        if (!_index.isValid())
        {
            _index = _video_model->index(0);
            ui->video_list->setCurrentIndex(_index);
            ui->video_info->setText(_index.data().toString());
            _player->setSource(_video_path.at(_index.row()) + '/' + _index.data().toString());
        }
        break;
    default:
        break;
    }
    if (_player->playbackState() == QMediaPlayer::PlayingState)
    {
        _player->pause();
    }
    else
    {
        _player->play();
    }
}

void MainWindow::last()
{
    switch (ui->tabWidget->currentIndex())
    {
    case 0:
        if (_audio_model->rowCount() == 0)
        {
            break;
        }
        if (_index.row() <=  0)
        {
            _index = _audio_model->index(_audio_model->rowCount() - 1);
        }
        else
        {
            _index = _audio_model->index(_index.row() - 1);
        }
        select(_index);
        break;
    case 1:
        if (_video_model->rowCount() == 0)
        {
            break;
        }
        if (_index.row() <=  0)
        {
            _index = _video_model->index(_video_model->rowCount() - 1);
        }
        else
        {
            _index = _video_model->index(_index.row() - 1);
        }
        select(_index);
        break;
    default:
        break;
    }
}

void MainWindow::next()
{
    _player->stop();
    switch (ui->tabWidget->currentIndex())
    {
    case 0:
        if (_audio_model->rowCount() == 0)
        {
            break;
        }
        if (_index.row() >= _audio_model->rowCount() - 1)
        {
            _index = _audio_model->index(0);
        }
        else
        {
            _index = _audio_model->index(_index.row() + 1);
        }
        select(_index);
        break;
    case 1:
        if (_video_model->rowCount() == 0)
        {
            break;
        }
        if (_index.row() >=  _video_model->rowCount() - 1)
        {
            _index = _video_model->index(0);
        }
        else
        {
            _index = _video_model->index(_index.row() + 1);
        }
        select(_index);
        break;
    default:
        break;
    }
}

void MainWindow::select(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return;
    }
    _index = index;
    _player->stop();
    switch (ui->tabWidget->currentIndex())
    {
    case 0:
        ui->audio_info->setText(_index.data().toString());
        ui->audio_list->setCurrentIndex(_index);
        _player->setSource(_audio_path.at(_index.row()) + '/' + _index.data().toString());
        break;
    case 1:
        ui->video_info->setText(_index.data().toString());
        ui->video_list->setCurrentIndex(_index);
        _player->setSource(_video_path.at(_index.row()) + '/' + _index.data().toString());
        break;
    default:
        break;
    }
    _player->play();
}

void MainWindow::refresh_time()
{
    ui->time_label->setText(_time.currentTime().toString("hh:mm:ss"));
    _timer.start(1000);
}

void MainWindow::open_files()
{
    QFileDialog* dialog = new QFileDialog();
    dialog->setModal(true);
    dialog->setFileMode(QFileDialog::ExistingFiles);
    QStringList files;
    switch (ui->tabWidget->currentIndex() )
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
                if (!_audio_path.isEmpty() && _audio_list.lastIndexOf(info.fileName()) != -1
                        && _audio_path.lastIndexOf(info.canonicalPath()) == _audio_list.lastIndexOf(info.fileName()))
                {
                    continue;
                }
                _audio_path.append(info.canonicalPath());
                _audio_list.append(info.fileName());
            }
            _audio_model->setStringList(_audio_list);
            _db->set_param("'audio_path'", ("\"" + _audio_path.back() + "\"").toStdString().c_str());
            break;
        case 1:
            for (const QString& file : files)
            {
                QFileInfo info(file);
                if (!_video_path.isEmpty() && _video_list.lastIndexOf(info.fileName()) != -1 
                        && _video_path.lastIndexOf(info.canonicalPath()) == _video_list.lastIndexOf(info.fileName()))
                {
                    continue;
                }
                _video_path.append(info.canonicalPath());
                _video_list.append(info.fileName());
            }
            _video_model->setStringList(_video_list);
            _db->set_param("'video_path'", ("\"" + _video_path.back() + "\"" ).toStdString().c_str());
            break;
        default:
            break;
        }
    }
    delete dialog;
}

void MainWindow::open_folder()
{
    QFileDialog* dialog = new QFileDialog();
    dialog->setModal(true);
    dialog->setFileMode(QFileDialog::Directory);
    switch (ui->tabWidget->currentIndex())
    {
    case 0:
        dialog->setDirectory(_db->get_param("'audio_path'"));
        break;
    case 1:
        dialog->setDirectory(_db->get_param("'video_path'"));
        break;
    default:
        break;
    }

    QString path = dialog->getExistingDirectory();

    if (!path.isEmpty())
    {
        QDir dir(path);
        
        switch (ui->tabWidget->currentIndex())
        {
        case 0:
            _audio_list = dir.entryList({"*.mp3", "*.aac", "*.wma", "*.wav", "*.ape", "*.flac", "*.ogg"}, QDir::Files | QDir::Readable, QDir::Name);
            _audio_path.clear();
            for (size_t i = 0, count = _audio_list.size(); i < count; ++i)
            {
                _audio_path.append(path);
            }
            _audio_model->setStringList(_audio_list);
            _db->set_param("'audio_path'", ("\"" + path + "\"").toStdString().c_str());
            break;
        case 1:
            _video_list = dir.entryList({"*.mp4", "*.flv", "*.avi", "*.mov", "*.mkv"}, QDir::Files | QDir::Readable, QDir::Name);
            _video_path.clear();
            for (size_t i = 0, count = _video_list.size(); i < count; ++i)
            {
                _video_path.append(path);
            }
            _video_model->setStringList(_video_list);
            _db->set_param("'video_path'", ("\"" + path + "\"").toStdString().c_str());
        default:
            break;
        }
    }

    delete dialog;
}

void MainWindow::show_list()
{
    switch (ui->tabWidget->currentIndex())
    {
    case 0:
        ui->audio_list->setHidden(ui->show_list_checkbox->checkState() == Qt::Unchecked);
        break;
    case 1:
        ui->video_list->setHidden(ui->show_list_checkbox->checkState() == Qt::Unchecked);
        break;
    default:
        break;
    }
}

void MainWindow::open_list()
{
    MediaListDialog *dialog = new MediaListDialog();
    QObject::connect(dialog, &MediaListDialog::list_selected, this, &MainWindow::reset_list);
    dialog->exec();
    QObject::disconnect(dialog, &MediaListDialog::list_selected, this, &MainWindow::reset_list);
    delete dialog;
}





void MainWindow::change_pos()
{
    qint64 pos;
    QString mins, secs;
    switch (ui->tabWidget->currentIndex())
    {
    case 0:
        pos = ui->audio_slider->sliderPosition() * 1000;
        ui->audio_slider->setValue(pos);
        _player->setPosition(pos);
        _player->play();
        mins = std::to_string(pos / 60000).c_str();
        secs = std::to_string(pos / 1000 % 60).c_str();
        ui->a_played_time_label->setText(mins + ':' + secs);
        QObject::disconnect(ui->audio_slider, &QSlider::sliderMoved, this, &MainWindow::change_played_time_by_slider_moved);
        break;
    case 1:
        if (_video_dock->isVisible())
        {
            pos = _video_dock->slider()->sliderPosition() * 1000;
            _video_dock->slider()->setValue(pos);
        }
        else
        {
            pos = ui->video_slider->sliderPosition() * 1000;
        }
        ui->video_slider->setValue(pos);
        _player->setPosition(pos);
        _player->play();
        mins = std::to_string(pos / 60000).c_str();
        secs = std::to_string(pos / 1000 % 60).c_str();
        ui->v_played_time_label->setText(mins + ':' + secs);
        if (_video_dock->isVisible())
        {
            QObject::disconnect(_video_dock->slider(), &QSlider::sliderMoved, this, &MainWindow::change_played_time_by_slider_moved);
        }
        else
        {
            QObject::disconnect(ui->video_slider, &QSlider::sliderMoved, this, &MainWindow::change_played_time_by_slider_moved);
        }
        break;
    default:
        break;
    }
}

void MainWindow::change_played_time_by_slider_moved(const int& value)
{
    QString mins(std::to_string(value / 60).c_str()), secs(std::to_string(value % 60).c_str());
    switch (ui->tabWidget->currentIndex())
    {
    case 0:
        ui->a_played_time_label->setText(mins + ':' + secs);
        break;
    case 1:
        ui->v_played_time_label->setText(mins + ':' + secs);
        if (_video_dock->isVisible())
        {
            _video_dock->played_time_label()->setText(mins + ':' + secs);
        }
        break;
    default:
        break;
    }
}

void MainWindow::reset_list(const QStringList *files, const QStringList *paths, const int &type)
{
    switch (type)
    {
    case 0:
        _audio_list = *files;
        _audio_path = *paths;
        _audio_model->setStringList(_audio_list);
        break;
    case 1:
        _video_list = *files;
        _video_path = *paths;
        _video_model->setStringList(_video_list);
        break;    
    default:
        break;
    }
}

void MainWindow::set_player_rate(const double &value)
{
    switch (ui->tabWidget->currentIndex())
    {
    case 0:
        _player->setPlaybackRate(ui->audio_rate->value());
        break;
    case 1:
        if (_video_dock->isVisible())
        {
            _player->setPlaybackRate(_video_dock->rate()->value());
            ui->video_rate->setValue(_video_dock->rate()->value());
        }
        else
        {
            _player->setPlaybackRate(ui->video_rate->value());
            _video_dock->rate()->setValue(ui->video_rate->value());
        }
        break;
    default:
        break;
    }
}

void MainWindow::store_state()
{
    _db->clear_list("stored_audio_list");
    _db->clear_list("stored_video_list");
    _db->append_medias("stored_audio_list", _audio_list, _audio_path);
    _db->append_medias("stored_video_list", _video_list, _video_path);
    switch (ui->tabWidget->currentIndex())
    {
    case 0:
        _db->set_param("'audio_playmode'", std::to_string(static_cast<int>(_play_mode)).c_str());
        _db->set_param("'video_playmode'", std::to_string(static_cast<int>(_memo.play_mode)).c_str());
        break;
    case 1:
        _db->set_param("'audio_playmode'", std::to_string(static_cast<int>(_memo.play_mode)).c_str());
        _db->set_param("'video_playmode'", std::to_string(static_cast<int>(_play_mode)).c_str());
        break;
    default:
        break;
    }
    _db->set_param("'autoplay'", ui->auto_play->isChecked() ? "1" : "0");
    _db->set_param("'last_widget_index'", std::to_string(ui->tabWidget->currentIndex()).c_str());
    _db->set_param("'last_pos'", std::to_string(_player->position()).c_str());
    _db->set_param("'last_model_index'", std::to_string(_index.row()).c_str());
    _db->set_param("'audio_rate'", std::to_string(ui->audio_rate->value()).c_str());
    _db->set_param("'video_rate'", std::to_string(ui->video_rate->value()).c_str());
}

void MainWindow::recover_state()
{
    PlayMode audio_playmode = static_cast<PlayMode>(_db->get_param("'audio_playmode'").toInt());
    PlayMode video_playmode = static_cast<PlayMode>(_db->get_param("'video_playmode'").toInt());
    _db->get_param("'video_playmode'").toInt();
    ui->auto_play->setChecked(_db->get_param("'autoplay'").toInt());
    ui->tabWidget->setCurrentIndex(_db->get_param("'last_widget_index'").toInt());
    ui->show_list_checkbox->setChecked(true);
    ui->audio_rate->setValue(_db->get_param("'audio_rate'").toDouble());
    ui->video_rate->setValue(_db->get_param("'video_rate'").toDouble());
    switch (ui->tabWidget->currentIndex())
    {
    case 0:
        _play_mode = audio_playmode;
        _memo.play_mode = video_playmode;
        _index = _audio_model->index(_db->get_param("'last_model_index'").toInt());
        _memo.audio_pos = _db->get_param("'last_pos'").toLongLong();
        _player->setPlaybackRate(ui->audio_rate->value());
        break;
    case 1:
        _play_mode = video_playmode;
        _memo.play_mode = audio_playmode;
        _index = _video_model->index(_db->get_param("'last_model_index'").toInt());
        _memo.video_pos = _db->get_param("'last_pos'").toLongLong();
        _player->setPlaybackRate(ui->video_rate->value());
        break;
    default:
        break;
    }
    QStringList texts({"单曲", "列表", "单曲循环", "列表循环"});
    ui->audio_play_mode->setText(texts.at(static_cast<int>(audio_playmode)));
    ui->video_play_mode->setText(texts.at(static_cast<int>(video_playmode)));    
    if (ui->auto_play->isChecked() && _index.isValid())
    {   
        select(_index);
    }
}

void MainWindow::append_to(const QString& list)
{
    QModelIndex index;
    switch (ui->tabWidget->currentIndex())
    {
    case 0:
        index = ui->audio_list->indexAt(_hover_pos);
        _db->append_medias(list.toStdString().c_str(), {index.data().toString()}, {_audio_path.at(index.row())});
        break;
    case 1:
        index = ui->video_list->indexAt(_hover_pos);
        _db->append_medias(list.toStdString().c_str(), {index.data().toString()}, {_video_path.at(index.row())});
        break;
    default:
        break;
    }
}






void MainWindow::playPositionChangedEvent(const qint64& pos)
{
    QString mins(std::to_string(pos / 60000).c_str());
    QString secs(std::to_string(pos / 1000 % 60).c_str());

    switch (ui->tabWidget->currentIndex())
    {
    case 0:
        ui->a_played_time_label->setText(mins + ':' + secs);
        ui->audio_slider->setValue(pos / 1000);
        break;
    case 1:
        ui->v_played_time_label->setText(mins + ':' + secs);
        ui->video_slider->setValue(pos / 1000);
        if (_video_dock->isVisible())
        {
            _video_dock->played_time_label()->setText(mins + ':' + secs);
            _video_dock->slider()->setValue(pos / 1000);
        }
    default:
        break;
    }
}

void MainWindow::tabWidgetChangedEvent(const int& index)
{
    _player->pause();
    std::swap(_memo.index, _index);
    std::swap(_memo.play_mode, _play_mode);
    switch (index)
    {
    case 0:
        _memo.video_pos = _player->position();
        if (_index.isValid())
        {
            _player->setSource(_audio_path.at(_index.row()) + '/' + _index.data().toString());
        }
        ui->show_list_checkbox->setChecked(ui->audio_list->isVisible());
        _player->setPlaybackRate(ui->audio_rate->value());
        break;
    case 1:
        _memo.audio_pos = _player->position();
        if (_index.isValid())
        {
            _player->setSource(_video_path.at(_index.row()) + '/' + _index.data().toString());
        }
        ui->show_list_checkbox->setChecked(ui->video_list->isVisible());
        _player->setPlaybackRate(ui->video_rate->value());
        break;
    default:
        break;
    }
}

void MainWindow::mediaStatusChangedEvent(const QMediaPlayer::MediaStatus& status)
{
    switch (status)
    {
    case QMediaPlayer::MediaStatus::BufferedMedia:
        {
            QString mins(std::to_string(_player->duration() / 60000).c_str());
            QString secs(std::to_string(_player->duration() / 1000 % 60).c_str());
            switch (ui->tabWidget->currentIndex())
            {
            case 0:
                ui->a_total_time_label->setText(" / " + mins + ':' + secs);
                ui->audio_slider->setMaximum(_player->duration() / 1000);
                _player->setPosition(_memo.audio_pos);
                mins = std::to_string(_memo.audio_pos / 60000).c_str();
                secs = std::to_string(_memo.audio_pos / 1000 % 60).c_str();
                ui->a_played_time_label->setText(mins + ':' + secs);
                ui->audio_slider->setValue(_memo.audio_pos / 1000);
                break;
            case 1:
                ui->v_total_time_label->setText(" / " + mins + ':' + secs);
                ui->video_slider->setMaximum(_player->duration() / 1000);
                _video_dock->tatol_time_label()->setText(mins + ':' + secs);
                _video_dock->slider()->setMaximum(_player->duration() / 1000);
                _player->setPosition(_memo.video_pos);
                mins = std::to_string(_memo.video_pos / 60000).c_str();
                secs = std::to_string(_memo.video_pos / 1000 % 60).c_str();
                ui->v_played_time_label->setText(mins + ':' + secs);
                ui->video_slider->setValue(_memo.video_pos / 1000);
            default:
                break;
            }
        }
        break;
    case QMediaPlayer::MediaStatus::EndOfMedia:
        switch (_play_mode)
        {
        case PlayMode::LIST:
            switch (ui->tabWidget->currentIndex())
            {
            case 0:
                if (_index.row() != _audio_model->rowCount() - 1)
                {
                    next();
                }
                break;
            case 1:
                if (_index.row() != _video_model->rowCount() - 1)
                {
                    next();
                }
                break;
            default:
                break;
            }
            break;
        case PlayMode::SINGLELOOP:
            play();
            break;
        case PlayMode::LISTLOOP:
            next();
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

void MainWindow::listMenuEvent(const int& code)
{
    QModelIndex index;
    size_t row;
    switch (ui->tabWidget->currentIndex())
    {
    case 0:
        index = ui->audio_list->indexAt(_hover_pos);
        row = index.row();
        switch (code)
        {
        case 1:
            if (!index.isValid())
            {
                return;
            }
            _audio_model->removeRow(row);
            _audio_list.removeAt(row);
            _audio_path.removeAt(row);
            break;
        case 2:
            _audio_model->removeRows(0, _audio_model->rowCount());
            _audio_list.clear();
            _audio_path.clear();
            _index = _audio_model->index(0);
            break;
        case 3:
            open_files();
            break;
        case 4:
            {
                MediaListDialog *dialog = new MediaListDialog();
                dialog->new_list(0, _audio_list, _audio_path);
                dialog->exec();
                delete dialog;
            }
            break;
        case 5:
            if (index.isValid())
            {
                ListSelectDialog *dialog = new ListSelectDialog(0);
                QObject::connect(dialog, &ListSelectDialog::append_to, this, &MainWindow::append_to);
                dialog->exec();
                QObject::disconnect(dialog, &ListSelectDialog::append_to, this, &MainWindow::append_to);
                delete dialog;
            }
            break;
        default:
            break;
        }
        break;
    case 1:
        index = ui->video_list->indexAt(_hover_pos);
        row = index.row();
        switch (code)
        {
        case 1:
            if (!index.isValid())
            {
                return;
            }
            _video_model->removeRow(row);
            _audio_list.removeAt(row);
            _video_path.removeAt(row);
            break;
        case 2:
            _video_model->removeRows(0, _video_model->rowCount());
            _video_list.clear();
            _video_path.clear();
            _index = _video_model->index(0);
            break;
        case 3:
            open_files();
            break;
        case 4:
            {
                MediaListDialog *dialog = new MediaListDialog();
                dialog->new_list(1, _video_list, _video_path);
                dialog->exec();
                delete dialog;
            }
            break;
        case 5:
            if (index.isValid())
            {
                ListSelectDialog *dialog = new ListSelectDialog(1);
                QObject::connect(dialog, &ListSelectDialog::append_to, this, &MainWindow::append_to);
                dialog->exec();
                QObject::disconnect(dialog, &ListSelectDialog::append_to, this, &MainWindow::append_to);
                delete dialog;
            }
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
    case Qt::Key_Escape:
        _video_widget->setFullScreen(false);
        _video_dock->hide();
        break;
    default:
        break;
    }
}

void MainWindow::videoWidgetKeyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
    case Qt::Key_Space:
        this->play();
        break;
    case Qt::Key_Right:
        this->next();
        break;
    case Qt::Key_Left:
        this->last();
        break;
    default:
        break;
    }
}

void MainWindow::videoWidgetMousePressEvent(QMouseEvent *event)
{
    if (!_video_widget->isFullScreen())
    {
        if (_player->playbackState() == QMediaPlayer::PlayingState)
        {
            _player->pause();
        }
        else
        {
            _player->play();
        }
        return;
    }
    if (QWidget::mapToGlobal(event->pos()).y() > QWidget::mapToGlobal(_video_widget->pos()).y() + _video_widget->height() - _video_dock->height())
    {
        if (_video_dock->isVisible())
        {
            if (_player->playbackState() == QMediaPlayer::PlayingState)
            {
                _player->pause();
            }
            else
            {
                _player->play();
            }
        }
        _video_dock->show();
    }
    else
    {   
        if (_video_dock->isHidden())
        {
            if (_player->playbackState() == QMediaPlayer::PlayingState)
            {
                _player->pause();
            }
            else
            {
                _player->play();
            }
        }
        _video_dock->hide();
    }
}

void MainWindow::videoDockOrderEvent(const int &order)
{
    switch (order)
    {
    case 1:
        this->last();
        break;
    case 2:
        this->play();
        break;
    case 3:
        this->next();
        break;
    case 4:
        _video_widget->setFullScreen(false);
        _video_dock->hide();
        break;
    default:
        break;
    }
}

void MainWindow::playModeChangedEvent()
{
    _play_mode = static_cast<PlayMode>((static_cast<int>(_play_mode) + 1) % 4);
    switch (ui->tabWidget->currentIndex())
    {
    case 0:
        switch (_play_mode)
        {
        case PlayMode::SINGLE:
            ui->audio_play_mode->setText("单曲");
            break;
        case PlayMode::LIST:
            ui->audio_play_mode->setText("列表");
            break;
        case PlayMode::SINGLELOOP:
            ui->audio_play_mode->setText("单曲循环");
            break;
        case PlayMode::LISTLOOP:
            ui->audio_play_mode->setText("列表循环");
            break;
        default:
            break;
        }
        break;
    case 1:
        switch (_play_mode)
        {
        case PlayMode::SINGLE:
            ui->video_play_mode->setText("单曲");
            break;
        case PlayMode::LIST:
            ui->video_play_mode->setText("列表");
            break;
        case PlayMode::SINGLELOOP:
            ui->video_play_mode->setText("单曲循环");
            break;
        case PlayMode::LISTLOOP:
            ui->video_play_mode->setText("列表循环");
            break;
        default:
            break;
        }
        break;
    default:
        break;
    } 
}


















