#include <QApplication>
#include <QStyleFactory>

#include "window.h"

int main( int argc, char* argv[] )
{
    QApplication a( argc, argv );
    a.setStyle(QStyleFactory::create("GTK+"));
    Window w( argc, argv );
    w.show();
    return a.exec();
}
