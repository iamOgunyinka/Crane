import bb.cascades 1.2
import bb.cascades.pickers 1.0

Page {
    property variant some_settings;
    
    titleBar: TitleBar {
        kind: TitleBarKind.Default
        title: "Settings"
    }
    Container {
        topPadding: 20
        leftPadding: 20
        rightPadding: 20
        bottomPadding: 20
        bottomMargin: 20
        
        Header {
            title: "Download Settings"
        }
        Container {
            topPadding: 20
            attachedObjects: [
                FilePicker {
                    id: filePicker
                    title: "Select Directory"
                    type: FileType.Other
                    mode: FilePickerMode.SaverMultiple
                    directories: [ "/accounts/1000/shared/" ];
                    onFileSelected: {
                        var new_location = "file://" + selectedFiles[0];
                        settings.location = new_location
                        locationLabel.text = new_location
                        
                        console.log( "New location is: ", new_location );
                    }
                }
            ]
            Container {
                layout: StackLayout {
                    orientation: LayoutOrientation.LeftToRight
                }
                
                Label {
                    text: "Location"
                    layoutProperties: StackLayoutProperties {
                        spaceQuota: 8
                    }
                }
                Button {
                    text: "Change"
                    onClicked: {
                        filePicker.open()
                    }
                    layoutProperties: StackLayoutProperties {
                        spaceQuota: 2
                    }
                }
            }
            Label {
                id: locationLabel
                onCreationCompleted: {
                    text = settings.location
                }
            }
        }
        Divider {}
        Container {
            topPadding: 20
            Label {
                id: downloadNumberLabel
                text: "Numbers of Downloads"
                horizontalAlignment: HorizontalAlignment.Left
            }
            DropDown {
                id: downloadDropDown
                options: [
                    Option {
                        text: "1 file"
                        value: 1
                    },
                    Option {
                        text: "2 files"
                        value: 2
                    },
                    Option {
                        text: "3 files"
                        value: 3
                    }
                ]
                onSelectedOptionChanged: {
                    downloadNumberLabel.text = "Numbers of Downloads( " + selectedOption.value + " )";
                    settings.max_download = selectedOption.value;
                    console.log( "Number of downloads ", settings.max_download );
                }
                onCreationCompleted: {
                    switch ( settings.max_download ){
                        case 1:
                            setSelectedIndex(0);
                            break;
                        case 2:
                            setSelectedIndex(1);
                            break;
                        case 3:
                            setSelectedIndex(2);
                            break;
                        default:
                            setSelectedIndex(3);
                            break;
                    }
                }
            }
        }
        Container {
            topPadding: 20
            Label {
                id: threadNumberLabel
                text: "Numbers of thread per download"
            }
            DropDown {
                horizontalAlignment: HorizontalAlignment.Right
                
                id: threadNumberDropdown
                options: [
                    Option {
                        text: "1 thread"
                        value: 1
                    },
                    Option {
                        text: "2 threads"
                        value: 2
                    },
                    Option {
                        text: "3 threads"
                        value: 3
                    },
                    Option {
                        text: "4 threads"
                        value: 4
                    }
                ]
                onSelectedOptionChanged: {
                    threadNumberLabel.text = "Numbers of thread per download( " + selectedOption.text + " )"
                    settings.max_thread = selectedOption.value
                }
                onCreationCompleted: {
                    switch ( settings.max_thread ){
                        case 1:
                            setSelectedIndex(0);
                            break;
                        case 2:
                            setSelectedIndex(1);
                            break;
                        case 3:
                            setSelectedIndex(2);
                            break;
                        default:
                            setSelectedIndex(3);
                            break;
                    }
                }
            }
        }
        
        Container {
            topPadding: 20
            layout: StackLayout {
                orientation: LayoutOrientation.LeftToRight
            }
            Label {
                text: "Notify after download completes"
                layoutProperties: StackLayoutProperties {
                    spaceQuota: 20
                }
            }
            ToggleButton {
                layoutProperties: StackLayoutProperties {
                    spaceQuota: 80
                }
                id: notificationOption
                onCheckedChanged: {
                    settings.app_notify = ( checked == true ? 1 : 0 )
                }
                onCreationCompleted: {
                    checked = ( settings.app_notify == 1 ? true : false )
                }
            }
        }
        Container {
            topPadding: 20
            layout: StackLayout {
                orientation: LayoutOrientation.LeftToRight
            }
            Label {
                layoutProperties: StackLayoutProperties {
                    spaceQuota: 20
                }
                text: "Pause download(s) on error"
            }
            ToggleButton {
                layoutProperties: StackLayoutProperties {
                    spaceQuota: 80
                }
                horizontalAlignment: HorizontalAlignment.Right
                id: pauseOnErrorOption
                onCreationCompleted: {
                    checked = ( settings.pause_on_error == 1 ? true : false )
                }
                onCheckedChanged: {
                    settings.pause_on_error = ( checked == true ? 1 : 0 )
                }
            }
        }
    }
}