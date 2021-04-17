#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "chat_message.hpp"
#include "ListViewDelegate.h"
#include "login.h"
#include <QWidget>
#include <QSplitter>
#include <QFormLayout>
#include <list>
#include <QFileDialog>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , socket(new QTcpSocket)
{
    ui->setupUi(this);
    auto *splitter = new QSplitter(Qt::Horizontal);

    auto *rightwidget = new QWidget;
    auto *left_widget = new QWidget;
    auto *rightlayout = new QFormLayout;
    auto *leftlayout = new QFormLayout;

    menu = new QMenuBar;
    search_user_line = new QLineEdit;
    user_button = new QPushButton("+");
    erase_user = new QPushButton("-");
    option_menu = new QMenu("Options");
    //connect(helpAction, SIGNAL(triggered()), this, SLOT(help()));
    QAction *logout_ = option_menu->addAction("Logout");
    QAction *erase_messages_ = option_menu->addAction("Erase Messages");
    menu->addMenu(option_menu);

    //************Username Components*****************

    username_view = new QListView;
    stringList = new StringList;
    username_view->setModel(stringList);
    left_widget->setLayout(leftlayout);
    rightwidget->setLayout(rightlayout);

    //******************************************

    connect_button = new QPushButton("Connect");

    //*******************Message_View***************************

    message_view = new QListView;
    message_view->setResizeMode(QListView::Adjust);
    message_view->setWordWrap(true);
    message_view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    message_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    message_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    message_view->setModel(&standard_model);
    message_view->setMinimumSize(400,450);
    message_view->setItemDelegate(new ListViewDelegate());

    //*******************Message_View***************************

    send_button = new QPushButton("Send");
    message_line = new QLineEdit;

    QPixmap pixmap("/home/corey/CLionProjects/ClientChatApp/picture_button.png");
    QIcon ButtonIcon(pixmap);
    picture_button = new QPushButton();
    picture_button->setIcon(ButtonIcon);
    leftlayout->addRow(search_user_line, user_button);
    leftlayout->addWidget(erase_user);
    leftlayout->addRow(username_view);
    rightlayout->addRow(connect_button);
    rightlayout->addRow(message_view);

    //************************************************************

    auto *hBoxLayout = new QHBoxLayout();
    hBoxLayout->addWidget(send_button);
    hBoxLayout->addWidget(picture_button);
    auto *container = new QWidget;
    container->setLayout(hBoxLayout);

    rightlayout->addRow(container, message_line);
    splitter->addWidget(left_widget);
    splitter->addWidget(rightwidget);
    setCentralWidget(splitter);
    setMenuBar(menu);

    auto current_directory = QCoreApplication::applicationDirPath();
    data_handler = new database_handler(current_directory.toStdString());

    connect(connect_button, &QPushButton::clicked, this, &MainWindow::connection);
    connect(logout_, SIGNAL(triggered()), this, SLOT(logout()));
    connect(erase_messages_, SIGNAL(triggered()), this, SLOT(erase_all_messages()));
    connect(send_button, &QPushButton::clicked, this, &MainWindow::sendMessage);
    connect(message_line, &QLineEdit::returnPressed, this, &MainWindow::sendMessage);
    connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(user_button, &QPushButton::clicked, this, &MainWindow::add_user);
    connect(username_view, SIGNAL(clicked(QModelIndex)), this, SLOT(set_recipient(QModelIndex)));
    connect(message_view, SIGNAL(clicked(QModelIndex)), this, SLOT(copy_data(QModelIndex)));
    connect(erase_user, &QPushButton::clicked, this, &MainWindow::erase_user_messages);
    connect(picture_button, &QPushButton::clicked, this, &MainWindow::send_picture);

    username = data_handler->get_username();

    receiver = "";

}

void MainWindow::copy_data(QModelIndex index) {
    standard_model.data(index);
    std::cout << index.data(Qt::UserRole + 1).data();
    QStandardItem *item = standard_model.itemFromIndex(index);
    std::cout << item->text().toStdString() <<std::endl;
}

void MainWindow::connection() {
    std::string messages = data_handler->load_messages();
    std::vector<std::pair<std::string, std::string>> message_list = simple_tokenizer(messages);
    for(const auto& msg: message_list) {
        if (msg.first == "user")
            append_received(QString::fromUtf8(msg.first.c_str()), QString::fromUtf8(msg.second.c_str()));
        else
            append_sent(QString::fromUtf8(msg.second.c_str()));
    }

    //Connect to host
    socket->connectToHost("127.0.0.1", 1234);

    //send username to server
    QString username_message = QString::fromUtf8(username.c_str()) + "\n";
    socket->write(QString(username_message).toUtf8());
}

std::vector<std::pair<std::string, std::string>> MainWindow::simple_tokenizer(const std::string& messages)
{
    std::vector<std::pair<std::string, std::string>> message_list;
    if (!messages.empty()) {
        std::string::size_type key_pos = 0;
        std::string::size_type key_end;
        std::string::size_type val_pos;
        std::string::size_type val_end;

        while((key_end = messages.find(':', key_pos)) != std::string::npos)
        {
            if((val_pos = messages.find_first_not_of(": ", key_end)) == std::string::npos)
                break;

            val_end = messages.find('\n', val_pos);
            message_list.emplace_back(messages.substr(key_pos, key_end - key_pos), messages.substr(val_pos, val_end - val_pos));

            key_pos = val_end;
            if(key_pos != std::string::npos)
                ++key_pos;
        }

    }

        return message_list;
}

void MainWindow::send_picture() {
    QString fileName = QFileDialog::getOpenFileName(
            this,tr("Open Image"), "/home/", tr("Image Files (*.png *.jpg *.bmp)"));
    if (!fileName.isEmpty()) {
        //new QStandardItem(fileName);
        QImage img;
        img.load(fileName);
        QImage img_scaled = img.scaled(200,200, Qt::KeepAspectRatio);
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        img_scaled.save(&buffer, "PNG");
        QString base_64 = QString::fromLatin1(byteArray.toBase64().data());
        auto *item = new QStandardItem(base_64);
        item->setData("Picture", Qt::UserRole + 1);
        standard_model.appendRow(item);

        /**QImage img;
        img.load(fileName);
        QImage img_scaled = img.scaled(200,200, Qt::KeepAspectRatio);
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        img_scaled.save(&buffer, "PNG");
        QString base_64 = QString::fromLatin1(byteArray.toBase64().data());
        chat_message msg;

        std::string body = base_64.toStdString();
        std::string json = chat_message::json_write(receiver.toStdString(), username, body, "Picture");
        msg.body_length(json.size());
        std::memcpy(msg.body(), json.c_str(), msg.body_length()+1);
        msg.encode_header();
        if (std::strlen(msg.data()) == 0) {
            return;
        }
        else {
            socket->write((char*)msg.data(), msg.length());
            data_handler->insert_message(receiver.toStdString(), username, body);
        }**/
    }
}


void MainWindow::sendMessage() {

    chat_message msg;

    std::string body = (message_line->text()).toStdString();
    std::string json = chat_message::json_write(receiver.toStdString(), username, body, "Text");
    msg.body_length(json.size());
    std::memcpy(msg.body(), json.c_str(), msg.body_length() + 1);
    msg.encode_header();
    if(std::strlen(msg.data()) == 0) {
        return;
    } else {
        //socket->write(QString(message).toUtf8());
        socket->write((char *)msg.data(), msg.length());
        data_handler->insert_message(receiver.toStdString(), username, body);
    }

    append_sent(message_line->text());
    message_line->clear();

}

void MainWindow::append_sent(const QString& message) {
    auto *item1 = new QStandardItem(message);
    item1->setData("Outgoing", Qt::UserRole + 1);
    standard_model.appendRow(item1);

}

void MainWindow::append_received(const QString& username, const QString& message) {
    if (receiver == username) {
        auto *received_message = new QStandardItem(message);
        received_message->setData("Incoming", Qt::UserRole + 1);
        standard_model.appendRow(received_message);
    }

    else if (!stringList->username_in_view(username)) {
        stringList->append(username);
    }

}

void MainWindow::onReadyRead() {

    QString line;
    chat_message message;

    line = QString::fromUtf8(socket->readAll());
    if (!line.isEmpty()) {
        try {
            int pos = (line.toStdString()).find('{');
            std::string body = line.toStdString().substr(pos);
            std::vector<std::string> json_contents = message.read_json(body);
            data_handler->insert_message(json_contents[1], json_contents[0], json_contents[2]);
            append_received(QString::fromUtf8(json_contents[1].c_str()), QString::fromUtf8(json_contents[2].c_str()));
        }

        catch (std::out_of_range &exception) {
            throw exception;
        }
    }
}

void MainWindow::erase_user_messages() {
    stringList->delete_user(username_view->currentIndex());
    standard_model.clear();

}

void MainWindow::erase_all_messages() {
    data_handler->clear_messages();
    standard_model.clear();
}

void MainWindow::add_user() const {
    stringList->append(search_user_line->text());
}

void MainWindow::logout() {
    this->close();
    auto *log = new login();
    log->resize(500,500);
    log->show();
}

void MainWindow::set_recipient(QModelIndex index) {
    //sets recipient of the message and changes messages
    receiver = stringList->set_recipient(index);
    std::string messages = data_handler->get_messages(receiver.toStdString());
    standard_model.clear();
    std::vector<std::pair<std::string, std::string>> message_list = simple_tokenizer(messages);
    for(const auto& msg: message_list) {
        if (msg.first == username)
            append_received(QString::fromUtf8(msg.first.c_str()), QString::fromUtf8(msg.second.c_str()));
        else
            append_sent(QString::fromUtf8(msg.second.c_str()));
    }
}


MainWindow::~MainWindow()
{
    delete data_handler;
    delete ui;
}

