import bb.cascades 1.2
import bb.system 1.2

TabbedPane
{
    property variant global_settings;
    
    id: root
    showTabsOnActionBar: true
    Menu.definition: MenuDefinition {
        helpAction: HelpActionItem {
        
        }
        settingsAction: SettingsActionItem {
            onTriggered: {
                var settings = settingsPage.createObject();
                settings.some_settings = global_settings
                navPane.push( settings )
            }
        }
    }
    attachedObjects: [
        ComponentDefinition {
            id: settingsPage
            source: "asset:///settings.qml"
        }
    ]
    Tab
    {
        id: homepageTab
        title: "Downloads"
        NavigationPane
        {
            id: navPane
            onPopTransitionEnded: {
                page.destroy()
            }
            Page
            {
                titleBar: TitleBar {
                    title: "Merry Downloads"
                    appearance: TitleBarAppearance.Default
                    kind: TitleBarKind.Default
                    acceptAction: ActionItem {
                        title: "-"
                        onTriggered: {
                            Application.minimize();
                        }
                    }
                    dismissAction: ActionItem {
                        title: "+"
                        onTriggered: {
                            addNewDownload.show()
                        }
                    }
                }
                attachedObjects: [
                    SystemPrompt {
                        id: addNewDownload
                        body: "Enter website address"
                        title: "New Download"
                        onFinished: {
                            switch( value ){
                                case SystemUiResult.ConfirmButtonSelection :
                                    network.addNewUrl( addNewDownload.inputFieldTextEntry(), 3, 
                                    "/accounts/1000/shared/downloads" );
                                    break;
                                default :
                                    break;
                            }
                        }
                    }
                ]
                actions: [
                    ActionItem {
                        title: "All"
                    },
                    ActionItem {
                        title: "Documents"
                    },
                    ActionItem {
                        title: "Images"
                    },
                    ActionItem {
                        title: "Music"
                    },
                    ActionItem {
                        title: "Video"
                    },
                    ActionItem {
                        title: "Programs"
                    },
                    ActionItem {
                        title: "Archives"
                    },
                    ActionItem {
                        title: "Others"
                    }
                ]
                Container {
                    topPadding: 20
                    rightPadding: 20
                    leftPadding: 20
                    SegmentedControl
                    {
                        Option 
                        {
                            id: queueOption
                            text: "Queue"
                        }
                        Option
                        {
                            id: finishedOption
                            text: "Finished"
                        }
                    }
                }
            }
        }
    }
    
    Tab
    {
        id: browserTab
        title: "Browser"
        
        NavigationPane
        {
            id: browserPane
            Page {
                Container {
                    Label 
                    {
                        text: "Work-in-Progress"
                        textStyle.textAlign: TextAlign.Center
                    }
                }
            }
        }
    }
}