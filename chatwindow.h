#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QWidget>
#include <QTcpSocket>

namespace Ui {
class ChatWindow;
}

class ChatWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWindow(QWidget *parent = nullptr);
    ~ChatWindow();

    void setTcpSocket(QTcpSocket *socket);
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void on_sendBtn_clicked();       // 对应UI中sendBtn的clicked信号
    void read_server_message();
    void handle_connection_state(QAbstractSocket::SocketState state);

private:
    Ui::ChatWindow *ui;  // 仅保留UI指针
    QTcpSocket *tcpSocket;
};

#endif // CHATWINDOW_H
