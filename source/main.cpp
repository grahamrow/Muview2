#include <QApplication>
#include <QStyleFactory>

#include "window.h"

int main( int argc, char* argv[] )
{
    QApplication app( argc, argv );

    // Parse command line arguments
    QApplication::setApplicationName("Muview");
    QApplication::setApplicationVersion("2.1.1");
    app.setStyle(QStyleFactory::create("GTK+"));

    Window w( app.arguments() );
    w.show();

    return app.exec();
}
