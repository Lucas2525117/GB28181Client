#include "GB28181Client.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    GB28181Client w;
    w.show();
    return a.exec();
}
