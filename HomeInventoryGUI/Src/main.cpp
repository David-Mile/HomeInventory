#include "HomeInventoryGUI.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    HomeInventoryGUI window;
    window.show();
    return app.exec();
}
