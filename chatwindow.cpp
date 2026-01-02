#include "chatwindow.h"
#include "ui_chatwindow.h"
#include <QKeyEvent>
#include <QEvent>
#include <QTimer>
#include <QDateTime>
#include <QJsonArray>

ChatWindow::ChatWindow(const QString &username, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatWindow)  // 初始化UI指针
    , tcpSocket(nullptr)
    , userName(username)      // 初始化用户名
{
    ui->setupUi(this);  // 加载UI文件
    this->setWindowTitle(QString("聊天窗口 - %1").arg(userName));

    // 初始化UI控件属性（全部通过ui->访问）
    ui->messageDisplay->setReadOnly(true);
    ui->textEdit->installEventFilter(this);
    ui->textEdit->setPlaceholderText("输入消息（Enter发送，Shift+Enter换行）");
    ui->sendBtn->setEnabled(false); // 初始禁用发送按钮
}

ChatWindow::~ChatWindow()
{
    delete ui;  // 释放UI资源（必须）
    if (tcpSocket && tcpSocket->state() == QTcpSocket::ConnectedState) {
        tcpSocket->disconnectFromHost();
    }
}

void ChatWindow::setTcpSocket(QTcpSocket *socket)
{
    tcpSocket = socket;
    // 绑定Socket信号槽
    connect(tcpSocket, &QTcpSocket::readyRead, this, &ChatWindow::read_server_message);
    connect(tcpSocket, &QTcpSocket::stateChanged, this, &ChatWindow::handle_connection_state);
    // 提示用户
    ui->sendBtn->setEnabled(true); // 连接成功后启用发送按钮
}

void ChatWindow::setUserName(const QString &name)
{
    userName = name;
    this->setWindowTitle(QString("聊天窗口 - %1").arg(userName));
}

// 事件过滤器：处理Enter发送消息
bool ChatWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->textEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        // Enter（无Shift）：发送消息
        if (keyEvent->key() == Qt::Key_Return && !keyEvent->modifiers().testFlag(Qt::ShiftModifier)) {
            on_sendBtn_clicked();
            return true; // 拦截事件，避免换行
        }
        // Shift+Enter：正常换行
        else if (keyEvent->key() == Qt::Key_Return && keyEvent->modifiers().testFlag(Qt::ShiftModifier)) {
            return false;
        }
    }
    return QWidget::eventFilter(watched, event);
}

// 发送按钮点击事件
void ChatWindow::on_sendBtn_clicked()
{
    if (!tcpSocket || tcpSocket->state() != QTcpSocket::ConnectedState) {
        ui->messageDisplay->append("未连接服务器，无法发送消息！");
        return;
    }

    QString message = ui->textEdit->toPlainText().trimmed();
    if (message.isEmpty()) return;

    // 创建 JSON 消息
    QJsonObject jsonObj;
    jsonObj.insert("type", "message");
    jsonObj.insert("content", message);
    jsonObj.insert("sender", userName); // 使用成员变量

    QJsonDocument jsonDoc(jsonObj);
    QByteArray jsonData = jsonDoc.toJson(QJsonDocument::Compact);

    tcpSocket->write(jsonData);

    // 显示自己发送的消息
    QJsonObject displayJson;
    displayJson.insert("type", "self_message");
    displayJson.insert("content", message);
    displayJson.insert("time", QDateTime::currentDateTime().toString("HH:mm:ss"));

    displayMessage(displayJson); // 创建一个专门的显示函数
    ui->textEdit->clear();
}

// 读取服务器消息
void ChatWindow::read_server_message()
{
    QByteArray data = tcpSocket->readAll();

    // 尝试解析 JSON
    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &jsonError);

    if (jsonError.error == QJsonParseError::NoError) {
        QJsonObject jsonObj = jsonDoc.object();
        displayMessage(jsonObj);
    } else {
        // 如果不是 JSON，可能是旧服务器，按普通文本显示
        QString message = QString::fromUtf8(data);
        ui->messageDisplay->append(message);
    }
}

// 专门处理 JSON 消息的显示函数
void ChatWindow::displayMessage(const QJsonObject &jsonObj)
{
    QString type = jsonObj.value("type").toString();
    QString content = jsonObj.value("content").toString();
    QString sender = jsonObj.value("sender").toString();
    QString time = jsonObj.value("time").toString();

    if (type == "broadcast") {
        // 显示其他用户的消息
        QString displayText = QString("[%1] %2: %3")
                                  .arg(time)
                                  .arg(sender)
                                  .arg(content);
        ui->messageDisplay->append(displayText);
    } else if (type == "self_message") {
        // 显示自己发送的消息（本地回显）
        QString displayText = QString("[%1] 我: %2")
                                  .arg(time)
                                  .arg(content);
        ui->messageDisplay->append(displayText);
    } else if (type == "user_join") {
        // 显示用户加入通知
        QString username = jsonObj.value("username").toString();
        QString joinTime = jsonObj.value("time").toString();
        ui->messageDisplay->append(QString("[%1] 系统: %2 加入了聊天").arg(joinTime).arg(username));

        // 更新用户列表（如果消息中包含用户列表）
        if (jsonObj.contains("userList")) {
            QJsonArray userArray = jsonObj.value("userList").toArray();
            QStringList userList;
            for (const QJsonValue &value : userArray) {
                userList.append(value.toString());
            }
            updateOnlineUserList(userList);
        }
    } else if (type == "user_leave") {
        // 显示用户离开通知
        QString username = jsonObj.value("username").toString();
        QString leaveTime = jsonObj.value("time").toString();
        ui->messageDisplay->append(QString("[%1] 系统: %2 离开了聊天").arg(leaveTime).arg(username));

        // 更新用户列表（如果消息中包含用户列表）
        if (jsonObj.contains("userList")) {
            QJsonArray userArray = jsonObj.value("userList").toArray();
            QStringList userList;
            for (const QJsonValue &value : userArray) {
                userList.append(value.toString());
            }
            updateOnlineUserList(userList);
        }
    } else if (type == "system") {
        // 显示系统消息
        ui->messageDisplay->append(QString("[系统] %1").arg(content));
    } else if (type == "user_list_update") {
        // 新增：处理用户列表更新消息
        QJsonArray userArray = jsonObj.value("userList").toArray();
        QStringList userList;
        for (const QJsonValue &value : userArray) {
            userList.append(value.toString());
        }
        updateOnlineUserList(userList);
    }
}

// 处理连接状态变化
void ChatWindow::handle_connection_state(QAbstractSocket::SocketState state)
{
    if (state == QAbstractSocket::UnconnectedState) {
        ui->messageDisplay->append("与服务器断开连接，即将关闭聊天窗口...");
        QTimer::singleShot(1500, this, &ChatWindow::close);
    }
}

// 更新在线用户列表
void ChatWindow::updateOnlineUserList(const QStringList &users)
{
    onlineUsers = users;  // 保存用户列表

    ui->onlineUsersList->clear();  // 清空现有列表

    // 将每个用户添加到列表中
    for (const QString &user : users) {
        QListWidgetItem *item = new QListWidgetItem(user);

        // 如果是自己，添加特殊标记
        if (user == userName) {
            item->setText(user + " (我)");
            item->setForeground(Qt::blue);  // 设置为蓝色显示
            item->setFont(QFont("Arial", 10, QFont::Bold));  // 加粗
        }

        ui->onlineUsersList->addItem(item);
    }
}
