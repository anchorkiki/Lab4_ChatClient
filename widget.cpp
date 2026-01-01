#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setWindowTitle("聊天客户端");  // 窗口标题

    // 初始化套接字
    tcpSocket = new QTcpSocket(this);

    // 关联“服务器发消息”信号到槽函数
    connect(tcpSocket, &QTcpSocket::readyRead, this, &Widget::read_server_message);

    // 关联“连接状态变化”信号到槽函数
    connect(tcpSocket, &QTcpSocket::stateChanged, this, &Widget::handle_connection_state);
}

Widget::~Widget()
{
    delete ui;
}

// 连接服务器按钮点击事件
void Widget::on_connectBtn_clicked()
{
    if (tcpSocket->state() == QTcpSocket::ConnectedState) {
        // 已连接：断开连接
        tcpSocket->disconnectFromHost();
        ui->connectBtn->setText("连接服务器");
        ui->sendBtn->setEnabled(false);
        return;
    }

    // 获取 IP 和端口（从输入框读取）
    QString ip = ui->ipEdit->text().trimmed();
    int port = ui->portEdit->text().toInt();

    // 连接服务器（IP + 端口）
    tcpSocket->connectToHost(ip, port);

    // 显示连接中状态
    ui->messageDisplay->append("正在连接服务器...");
}

// 发送消息按钮点击事件
void Widget::on_sendBtn_clicked()
{
    // 获取输入框的消息（去除前后空格）
    QString message = ui->messageInput->text().trimmed();
    if (message.isEmpty()) return;  // 空消息不发送

    // 发送消息（转换为 UTF-8 字节数组，避免乱码）
    tcpSocket->write(message.toUtf8());

    // 清空输入框
    ui->messageInput->clear();

    // 在本地显示自己发送的消息（加“我：”前缀）
    ui->messageDisplay->append(QString("我：%1").arg(message));
}

// 读取服务器发送的消息
void Widget::read_server_message()
{
    // 读取服务器消息
    QByteArray messageData = tcpSocket->readAll();
    QString message = QString::fromUtf8(messageData);

    // 显示到消息区域（自动换行）
    ui->messageDisplay->append(message);
}

// 处理连接状态变化
void Widget::handle_connection_state(QAbstractSocket::SocketState state)
{
    switch (state) {
    case QAbstractSocket::ConnectedState:
        // 连接成功
        ui->messageDisplay->append("连接服务器成功！");
        ui->connectBtn->setText("断开连接");
        ui->sendBtn->setEnabled(true);  // 启用发送按钮
        break;
    case QAbstractSocket::UnconnectedState:
        // 断开连接
        ui->messageDisplay->append("与服务器断开连接！");
        ui->connectBtn->setText("连接服务器");
        ui->sendBtn->setEnabled(false);  // 禁用发送按钮
        break;
    case QAbstractSocket::ConnectingState:
        // 连接中（无需额外处理，之前已显示）
        break;
    default:
        if (tcpSocket->error() != QAbstractSocket::UnknownSocketError) {
            ui->messageDisplay->append("连接失败！原因：" + tcpSocket->errorString());
        }
        break;
    }
}
