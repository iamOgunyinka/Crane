#include "applicationui.hpp"

#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/AbstractPane>
#include <bb/cascades/LocaleHandler>

#include "Settings.hpp"
#include "DownloadManager.hpp"
#include "CraneDataModel.hpp"

using namespace bb::cascades;
using namespace bb::system;

ApplicationUI::ApplicationUI() :
                QObject()
{
    // prepare the localization
    m_pTranslator =     new QTranslator(this);
    m_pLocaleHandler =  new LocaleHandler(this);
    m_pInvokeManager =  new bb::system::InvokeManager( this );
    m_pSettings =       new Settings( this );
    m_pDownloadManager= new CraneDownloader;

    bool res = QObject::connect(m_pLocaleHandler, SIGNAL(systemLanguageChanged()), this, SLOT(onSystemLanguageChanged()));
    // This is only available in Debug builds
    Q_ASSERT(res);
    // Since the variable is not used in the app, this is added to avoid a
    // compiler warning
    Q_UNUSED(res);

    // initial load
    onSystemLanguageChanged();

    // Create scene document from main.qml asset, the parent is set
    // to ensure the document gets destroyed properly at shut down.
    bool result = QObject::connect( m_pInvokeManager, SIGNAL(invoked(bb::system::InvokeRequest)),
            this, SLOT(onInvokeRequest( bb::system::InvokeRequest const & )));

    Q_ASSERT( result );

    switch( m_pInvokeManager->startupMode() ){
        case ApplicationStartupMode::LaunchApplication:
            initFullUI();
            break;
        default:
            break;
    }
}

void ApplicationUI::onInvokeRequest( bb::system::InvokeRequest const & request )
{
    QString target = request.target();
    QString action = request.action();
    QString mimeType = request.mimeType();
    QString uri = request.uri().toString();

    Q_UNUSED( target );
    Q_UNUSED( action );
    Q_UNUSED( mimeType );

    qDebug() << "OnInvoked Called with " << uri;
    initFullUI();
}

void ApplicationUI::initFullUI()
{
    CraneDataModel         *data_model = new CraneDataModel;
    CraneFilteredDataModel *filtered_model = new CraneFilteredDataModel( data_model, this );
    ApplicationClipBoard   *clipboard = new ApplicationClipBoard( this );

    QObject::connect( m_pDownloadManager, SIGNAL( statusChanged( QString ) ), filtered_model, SLOT( refreshView() ) );
    QObject::connect( m_pDownloadManager, SIGNAL( status( QString ) ), filtered_model, SLOT( refreshView() ) );

    QmlDocument *qml = QmlDocument::create( "asset:///main.qml" ).parent(this);

    qml->setContextProperty( "model_", filtered_model );
    qml->setContextProperty( "download_manager", m_pDownloadManager );
    qml->setContextProperty( "settings", m_pSettings );
    qml->setContextProperty( "_clipboard", clipboard );
    // Create root object for the UI
    AbstractPane *root = qml->createRootObject<AbstractPane>();

    // Set created root object as the application scene
    Application::instance()->setScene(root);
}

void ApplicationUI::onSystemLanguageChanged()
{
    QCoreApplication::instance()->removeTranslator(m_pTranslator);
    // Initiate, load and install the application translation files.
    QString locale_string = QLocale().name();
    QString file_name = QString("ProjectGlass_%1").arg(locale_string);
    if (m_pTranslator->load(file_name, "app/native/qm")) {
        QCoreApplication::instance()->installTranslator(m_pTranslator);
    }
}
