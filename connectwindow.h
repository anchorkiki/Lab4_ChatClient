#ifndef CONNECTWINDOW_H
#define CONNECTWINDOW_H

#include <QWidget>
#include <QTcpSocket>

namespace Ui {
class ConnectWindow;
}

class ConnectWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectWindow(QWidget *parent = nullptr);
    ~ConnectWindow();

private slots:
    void on_connectBtn_clicked();  // 连接按钮点击（UI 自动关联）
    void handle_connection_state(QAbstractSocket::SocketState state);

private:
    Ui::ConnectWindow *ui;  // 新增：UI 指针（和你现有 Widget 一致）
    QTcpSocket *tcpSocket;
};

#endif // CONNECTWINDOW_H
