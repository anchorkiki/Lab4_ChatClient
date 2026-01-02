#include "connectwindow.h"
#include "ui_connectwindow.h"
#include "chatwindow.h"

ConnectWindow::ConnectWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ConnectWindow)  // 初始化 UI
{
    ui->setupUi(this);  // 加载 UI（核心步骤）
    this->setWindowTitle("服务器连接");

    // 初始化 Socket（和你现有 Widget 逻辑一致）
    tcpSocket = new QTcpSocket(this);

    // 绑定信号槽
    // 注：on_connectBtn_clicked 是 UI 自动关联的槽（命名规则：on_对象名_信号名）
    connect(tcpSocket, &QTcpSocket::stateChanged, this, &ConnectWindow::handle_connection_state);

    // 初始化默认值
    ui->ipEdit->setText("127.0.0.1");
    ui->portEdit->setText("1234");
    ui->statusDisplay->setReadOnly(true); // 确保状态框只读
}

ConnectWindow::~ConnectWindow()
{
    delete ui;  // 释放 UI 资源
    if (tcpSocket->state() == QTcpSocket::ConnectedState) {
        tcpSocket->disconnectFromHost();
    }
}

void ConnectWindow::on_connectBtn_clicked()
{
    // 拦截「正在连接」和「已连接」状态，避免重复调用connectToHost
    if (tcpSocket->state() == QTcpSocket::ConnectingState ||
        tcpSocket->state() == QTcpSocket::ConnectedState) {
        ui->statusDisplay->append("提示：已在连接中/已连接，无需重复操作！");
        return;
    }

    // 校验IP/端口
    QString ip = ui->ipEdit->text().trimmed();
    int port = ui->portEdit->text().toInt();
    if (ip.isEmpty() || port <= 0 || port > 65535) {
        ui->statusDisplay->append("错误：IP或端口输入无效！");
        return;
    }

    // 连接服务器
    tcpSocket->connectToHost(ip, port);
    ui->statusDisplay->append(QString("正在连接 %1:%2...").arg(ip).arg(port));
    ui->connectBtn->setText("取消连接"); // 优化：连接中显示「取消连接」
}

// 处理连接状态变化
void ConnectWindow::handle_connection_state(QAbstractSocket::SocketState state)
{
    switch (state) {
    case QAbstractSocket::ConnectedState: {
        ui->statusDisplay->append("连接成功！即将进入聊天窗口...");
        ChatWindow *chatWin = new ChatWindow();
        chatWin->setTcpSocket(tcpSocket); // 先设置Socket，再改父对象
        tcpSocket->setParent(chatWin);    // 让ChatWindow管理Socket生命周期
        chatWin->show();
        this->hide();

        // 聊天窗口关闭后恢复连接窗口
        connect(chatWin, &ChatWindow::destroyed, this, [=]() {
            this->show();
            tcpSocket = new QTcpSocket(this); // 重建Socket
            connect(tcpSocket, &QTcpSocket::stateChanged, this, &ConnectWindow::handle_connection_state);
            ui->connectBtn->setText("连接服务器");
            ui->statusDisplay->append("聊天窗口已关闭，可重新连接服务器");
        });
        break;
    }
    case QAbstractSocket::ConnectingState:
        ui->connectBtn->setText("取消连接"); // 连接中按钮显示「取消连接」
        break;
    case QAbstractSocket::UnconnectedState:
        ui->statusDisplay->append("已与服务器断开连接");
        ui->connectBtn->setText("连接服务器"); // 断开后还原按钮
        break;
    default:
        if (tcpSocket->error() != QAbstractSocket::UnknownSocketError) {
            ui->statusDisplay->append("连接失败：" + tcpSocket->errorString());
            ui->connectBtn->setText("连接服务器"); // 出错后还原按钮
        }
        break;
    }
}
