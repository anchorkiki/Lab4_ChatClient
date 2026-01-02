#include "chatwindow.h"
#include "ui_chatwindow.h"
#include <QKeyEvent>
#include <QEvent>
#include <QTimer>

ChatWindow::ChatWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatWindow)  // 初始化UI指针
    , tcpSocket(nullptr)
{
    ui->setupUi(this);  // 加载UI文件
    this->setWindowTitle("聊天窗口");

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

    tcpSocket->write(message.toUtf8());
    ui->messageDisplay->append(QString("我：%1").arg(message));
    ui->textEdit->clear();
}

// 读取服务器消息
void ChatWindow::read_server_message()
{
    QByteArray data = tcpSocket->readAll();
    QString message = QString::fromUtf8(data);
    ui->messageDisplay->append(message);
}

// 处理连接状态变化
void ChatWindow::handle_connection_state(QAbstractSocket::SocketState state)
{
    if (state == QAbstractSocket::UnconnectedState) {
        ui->messageDisplay->append("与服务器断开连接，即将关闭聊天窗口...");
        QTimer::singleShot(1500, this, &ChatWindow::close);
    }
}
