#include "applicationui.hpp"

#include <bb/cascades/Application>

#include <QLocale>
#include <QTranslator>

#include <Qt/qdeclarativedebug.h>

using namespace bb::cascades;

static QString getValue()
{
    QSharedPointer<CraneDM::Settings> p_settings( new CraneDM::Settings );
    return p_settings->appTheme();
}

Q_DECL_EXPORT int main(int argc, char **argv)
{
    qputenv( "CASCADES_THEME", getValue().toUtf8() );
    Application app( argc, argv );

    // Create the Application UI object, this is where the main.qml file
    // is loaded and the application scene is set.
    CraneDM::ApplicationUI appui;

    // Enter the application main event loop.
    return Application::exec();
}
