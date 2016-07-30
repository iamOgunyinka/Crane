import bb.cascades 1.2
import custom_ad.smaatosdk 1.0

Page {
    property variant data;
    
    id: root
    titleBar: TitleBar {
        kind: TitleBarKind.Default
        title: "Download Details"
    }
    Container {
        topPadding: 20
        bottomPadding: 20
        rightPadding: 20
        leftPadding: 20
        attachedObjects: [
            TextStyleDefinition {
                fontWeight: FontWeight.Bold
                base: SystemDefaults.TextStyles.BodyText
                id: allTextDef
            }
        ]
        ScrollView {
            topMargin: 20
            leftMargin: 20
            rightMargin: 20
            
            Container {
                SSmaatoAdView {
                    id: adView
                    coppa: 1
                    format: 1
                    viewSize: SSmaatoAdView.AdViewSizeNormal
                    preferredWidth: 768
                    preferredHeight: 128
                }
                Divider {}
                Container {
                    Label {
                        text: "Filename"
                        textStyle.base: allTextDef.style
                    }
                    Label {
                        text: data.filename
                    }
                }
                Divider {}
                Container {
                    layout: StackLayout {
                        orientation: LayoutOrientation.LeftToRight
                    }
                    Label {
                        text: "File Size"
                        textStyle.base: allTextDef.style
                    }
                    Label {
                        text: data.total_size
                    }
                }
                Divider {}
                Container {
                    layout: StackLayout {
                        orientation: LayoutOrientation.LeftToRight
                    }
                    Label {
                        text: "Downloaded Size"
                        textStyle.base: allTextDef.style
                    }
                    Label {
                        text: data.downloaded_size
                    }
                }
                Divider {}
                Container {
                    Label {
                        text: "Location on phone"
                        textStyle.base: allTextDef.style
                    }
                    Label {
                        text: data.path
                    }
                }
                Divider {}
                Container {
                    layout: StackLayout {
                        orientation: LayoutOrientation.LeftToRight
                    }
                    Label {
                        text: "Resumable"
                        textStyle.base: allTextDef.style
                    }
                    Label {
                        text: ( data.resumable == 1 ? "Yes" : "No" )
                    }
                }
                Divider {}
                Container {
                    layout: StackLayout {
                        orientation: LayoutOrientation.LeftToRight
                    }
                    Label {
                        text: "Time Started"
                        textStyle.base: allTextDef.style
                    }
                    Label {
                        text: data.time_started
                    }
                }
                Divider {}
                Container {
                    layout: StackLayout {
                        orientation: LayoutOrientation.LeftToRight
                    }
                    Label {
                        text: "Time completed"
                        textStyle.base: allTextDef.style
                    }
                    Label {
                        text: data.time_ended
                    }
                }
                Divider {}
                Label {
                    text: "Web address"
                    textStyle.base: allTextDef.style
                }
                Container {
                    Label {
                        id: url_text
                        text: data.original_url
                    }
                    Button {
                        text: "Copy( web address )"
                        onClicked: {
                            _clipboard.setClipboardText( url_text.text );
                        }
                    }
                }
            }
        }
    }
}
