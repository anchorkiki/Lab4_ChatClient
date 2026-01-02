#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QWidget>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QListWidgetItem>

namespace Ui {
class ChatWindow;
}

class ChatWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWindow(const QString &username = "", QWidget *parent = nullptr);
    ~ChatWindow();

    void setTcpSocket(QTcpSocket *socket);
    void setUserName(const QString &name);
    bool eventFilter(QObject *watched, QEvent *event) override;
    void displayMessage(const QJsonObject &jsonObj);

private slots:
    void on_sendBtn_clicked();       // 对应UI中sendBtn的clicked信号
    void read_server_message();
    void handle_connection_state(QAbstractSocket::SocketState state);

private:
    void updateOnlineUserList(const QStringList &users);  // 更新用户列表

private:
    Ui::ChatWindow *ui;  // 仅保留UI指针
    QTcpSocket *tcpSocket;
    QString userName;    // 添加用户名成员变量
    QStringList onlineUsers;  // 存储在线用户列表
};

#endif // CHATWINDOW_H
