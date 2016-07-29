import bb.cascades 1.2
import bb.system 1.2
import bb.data 1.0
import bb.platform 1.2
import custom_ad.smaatosdk 1.0
import custom_ad.smaatoapi 1.0

TabbedPane
{
    property int number_of_threads;
    property int number_of_downloads;
    property string downloads_directory;
    property int autocopy_clipboard;
    property bool notify_on_completion;
    property string new_image_location
    
    onCreationCompleted: {
        number_of_threads = settings.max_thread;
        number_of_downloads = settings.max_download;
        downloads_directory = settings.location
        autocopy_clipboard = settings.autocopy_from_clipboard
        notify_on_completion = settings.app_notify
        
        switch( Application.themeSupport.theme.colorTheme.style ){
            case VisualStyle.Bright:
                new_image_location = "asset:///images/new_bright.png";
                break;
            case VisualStyle.Dark:
                new_image_location = "asset:///images/new_dark.png";
                break;
        }
        Application.aboutToQuit.connect( notification.clearEffects )
        Application.aboutToQuit.connect( notification.deleteAllFromInbox )
        Application.aboutToQuit.connect( download_manager.aboutToExit )
    }
    id: root
    
    function createDetails( info )
    {
        var detailsSheet = detailsViewSheet.createObject();
        detailsSheet.data = info;
        navPane.push( detailsSheet );
    }
    
    showTabsOnActionBar: true
    Menu.definition: MenuDefinition {
        helpAction: HelpActionItem {
            onTriggered: {
                navPane.push( helpViewSheet.createObject() )
            }
        }
        settingsAction: SettingsActionItem {
            onTriggered: {
                navPane.push( settingsPage.createObject() )
            }
        }
    }
    attachedObjects: [
        ComponentDefinition {
            id: settingsPage
            source: "asset:///settings.qml"
        },
        ComponentDefinition {
            id: detailsViewSheet
            source: "asset:///details.qml"
        },
        ComponentDefinition {
            id: helpViewSheet
            source: "asset:///help.qml"
        }
    ]
    Tab
    {
        id: homepageTab
        title: "Downloads"
        imageSource: "asset:///images/home.png"
        NavigationPane
        {
            id: navPane
            onPopTransitionEnded: {
                number_of_threads = settings.max_thread;
                number_of_downloads = settings.max_download;
                downloads_directory = settings.location
                autocopy_clipboard = settings.autocopy_from_clipboard
                notify_on_completion = settings.app_notify
                
                page.destroy()
            }
            
            function status( message )
            {
                if( busy_wait_activity.running ){
                    busy_wait_activity.stop();
                }
                system_toast.body = message
                system_toast.show();
            }
            
            function notify( filename )
            {
                if( notify_on_completion == 1 ){
                    notification.title = "Download Complete.";
                    notification.body = filename;
                    notification.notify();
                }
            }
            
            function addNewDownload( url_ )
            {
                if( !busy_wait_activity.running ){
                    busy_wait_activity.start();
                }
                download_manager.addNewUrl( url_, number_of_threads, 
                    number_of_downloads, downloads_directory
                );
            }
            
            Page
            {
                titleBar: TitleBar {
                    title: "Crane Download Manager"
                    appearance: TitleBarAppearance.Default
                    kind: TitleBarKind.Default
                    dismissAction: ActionItem {
                        imageSource: new_image_location
                        onTriggered: {
                            if( autocopy_clipboard == 1 ){
                                addNewDownload.inputField.defaultText = _clipboard.clipboardText();
                            }
                            addNewDownload.show()
                        }
                    }
                }
                attachedObjects: [
                    SSmaatoAPI {
                        id: ssmaato_api
                        autoRefreshPeriod: 30
                    },
                    OrientationHandler {
                        id: orientation_handler
                        onOrientationChanged: {
                            if( adView.viewSize == SSmaatoAdView.AdViewSizeFullScreenPortrait || 
                                adView.viewSize == SSmaatoAdView.AdViewSizeFullScreenLandscape )
                            {
                                if( orientation == OrientationSupport.orientation.Landscape ){
                                    adView.viewSize = SSmaatoAdView.AdViewSizeFullScreenLandscape;
                                } else {
                                    adView.viewSize = SSmaatoAdView.AdViewSizeFullScreenPortrait;
                                }
                            }
                            adView.updateAd();
                        }
                    },
                    Notification {
                        id: notification
                        type: NotificationType.Default
                    },
                    SystemPrompt {
                        id: addNewDownload
                        body: "Enter website address"
                        title: "New Download"
                        onFinished: {
                            switch( value ){
                                case SystemUiResult.ConfirmButtonSelection :
                                    navPane.addNewDownload( addNewDownload.inputFieldTextEntry() );
                                    break;
                                default :
                                    break;
                            }
                        }
                    },
                    SystemToast {
                        id: system_toast
                    }
                ]
                onCreationCompleted: {
                    download_manager.downloadStarted.connect( navPane.status );
                    download_manager.error.connect( navPane.status );
                    download_manager.completed.connect( navPane.notify );
                    _invoker.error.connect( navPane.status );
                    _invoker.sharedUrl.connect( navPane.addNewDownload );
                }
                Container {
                    topPadding: 20
                    rightPadding: 20
                    leftPadding: 20
                    id: parety
                    
                    Container {
                        horizontalAlignment: HorizontalAlignment.Center
                        verticalAlignment: VerticalAlignment.Center
                        SSmaatoAdView {
                            id: adView
                            format: 1
                            coppa: 0
                            autoRefreshPeriod: 0
                            gender: SSmaatoAdView.AdViewSizeNormal
                            preferredWidth: 768
                            preferredHeight: 128
                            
                            onCreationCompleted: {
                                ssmaato_api.adFetchFinished.connect( adView.onAdFetchFinished );
                                ssmaato_api.adFetchFailed.connect( adView.onAdFetchFailed )

                                ssmaato_api.fetchAd();
                            }
                        }
                        /*
                        */
                        SegmentedControl
                        {
                            id: segmented_filter
                            onCreationCompleted: {
                                segmented_filter.selectedIndex = 0; // show completed downloads
                            }
                            onSelectedIndexChanged: {
                                model_.changeView( selectedIndex );
                            }
                            
                            Option
                            {
                                id: queueOption
                                text: "All Downloads"
                            }
                            Option
                            {
                                id: finishedOption
                                text: "Completed"
                            }
                        }
                        ActivityIndicator {
                            id: busy_wait_activity
                            preferredHeight: 100
                            preferredWidth: 100
                            horizontalAlignment: HorizontalAlignment.Center
                            verticalAlignment: VerticalAlignment.Center
                        }
                    }
                    Container {
                        topPadding: 20
                        ListView {
                            id: list_view
                            dataModel: model_
                            property variant selected_download_indexpath;
                            
                            onSelectionChanged: {
                                if( selected ){
                                    selected_download_indexpath = indexPath
                                }
                            }
                            contextActions: [
                                ActionSet {
                                    ActionItem {
                                        title: "Open"
                                        imageSource: "asset:///images/open.png"
                                        onTriggered: {
                                            var data = list_view.dataModel.data( list_view.selected_download_indexpath );
                                            var filename = data.path
                                            _invoker.open( filename );
                                        }
                                    }
                                    ActionItem {
                                        title: "Start"
                                        imageSource: "asset:///images/play.png"
                                        onTriggered: {
                                            var data = list_view.dataModel.data( list_view.selected_download_indexpath )
                                            var url_ = data.original_url
                                            list_view.start( url_ );
                                        }
                                    }
                                    ActionItem {
                                        title: "Pause"
                                        imageSource: "asset:///images/pause.png"
                                        onTriggered: {
                                            var data = list_view.dataModel.data( list_view.selected_download_indexpath )
                                            var url_ = data.original_url
                                            list_view.stop( url_, true );
                                        }
                                    }
                                    ActionItem {
                                        title: "Stop"
                                        imageSource: "asset:///images/stop.png"
                                        onTriggered: {
                                            var data = list_view.dataModel.data( list_view.selected_download_indexpath )
                                            var url_ = data.original_url
                                            list_view.stop( url_, false );
                                        }
                                    }
                                    ActionItem {
                                        title: "Remove item from list";
                                        imageSource: "asset:///images/delete.png"
                                        onTriggered: {
                                            var data = list_view.dataModel.data( list_view.selected_download_indexpath )
                                            var url_ = data.original_url;
                                            list_view.remove_item( url_, false )
                                            list_view.dataModel.removeItem( list_view.selected_download_indexpath );
                                        }
                                    }
                                    DeleteActionItem {
                                        title: "Remove item and delete file"
                                        imageSource: "asset:///images/delete.png"
                                        onTriggered: {
                                            var data = list_view.dataModel.data( list_view.selected_download_indexpath )
                                            var url_ = data.original_url;
                                            list_view.remove_item( url_, true )
                                            list_view.dataModel.removeItem( list_view.selected_download_indexpath );
                                        }
                                    }
                                }
                            ]
                            onTriggered: {
                                if( indexPath.length == 1 ){
                                    var data = model_.data( indexPath );
                                    root.createDetails( data );
                                }
                            }
                            
                            function stop( url_, toPause )
                            {
                                download_manager.stopDownload( url_, toPause );
                            }
                            
                            function start( url_ ){
                                download_manager.addNewUrl( url_, number_of_threads, number_of_downloads, downloads_directory );
                            }
                            
                            function remove_item( url_, removeFile )
                            {
                                download_manager.removeItem( url_, removeFile );
                            }
                            
                            listItemComponents: [
                                ListItemComponent {
                                    type: "item"
                                    id: list_item
                                    CustomListItem {
                                        id: customList
                                        property variant selected_download
                                        
                                        Container {
                                            layout: StackLayout {
                                                orientation: LayoutOrientation.LeftToRight
                                            }
                                            Container {
                                                layoutProperties: StackLayoutProperties {
                                                    spaceQuota: 2
                                                }
                                                ImageView {
                                                    id: download_item_logo
                                                    imageSource: ListItemData.image
                                                }
                                                Label {
                                                    id: download_item_downloaded_size
                                                    text: ListItemData.downloaded_size
                                                }
                                            }
                                            Container {
                                                layoutProperties: StackLayoutProperties {
                                                    spaceQuota: 6
                                                }
                                                Label {
                                                    id: download_item_filename
                                                    text: ListItemData.filename
                                                }
                                                ProgressIndicator {
                                                    fromValue: 1
                                                    toValue: 100
                                                    value: ListItemData.percentage
                                                }
                                                Label {
                                                    text: ListItemData.speed
                                                }
                                                rightMargin: 20
                                            }
                                            Container {
                                                layoutProperties: StackLayoutProperties {
                                                    spaceQuota: 2
                                                }
                                                Label {
                                                    id: download_item_status
                                                    text: ListItemData.status
                                                }
                                                Label {
                                                    id: download_item_total_size
                                                    text: ListItemData.total_size
                                                }
                                            }
                                        }
                                    }
                                }
                            ]
                        }
                    }
                }
            }
        }
    }
} // homepageTab
