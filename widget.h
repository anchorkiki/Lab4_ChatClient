#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>  // 通信套接字（和服务器通信）

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    // 连接服务器按钮点击事件
    void on_connectBtn_clicked();

    // 发送消息按钮点击事件
    void on_sendBtn_clicked();

    // 读取服务器发送的消息
    void read_server_message();

    // 处理连接状态变化（连接成功/失败/断开）
    void handle_connection_state(QAbstractSocket::SocketState state);

private:
    Ui::Widget *ui;
    QTcpSocket *tcpSocket;  // 和服务器通信的套接字
};

#endif // WIDGET_H
