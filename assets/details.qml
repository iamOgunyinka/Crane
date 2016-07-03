import bb.cascades 1.2

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
        Container {
            layout: StackLayout {
                orientation: LayoutOrientation.LeftToRight
            }
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
                text: ( data.resumable == 1 ? "true" : "false" )
            }
        }
        Divider {}
        Label {
            text: "Web address"
            textStyle.base: allTextDef.style
        }
        Container {
            layout: StackLayout {
                orientation: LayoutOrientation.LeftToRight
            }
            Label {
                id: url_text
                text: data.original_url
            }
            Button {
                text: "Copy"
                onClicked: {
                    _clipboard.setClipboardText( url_text.text );
                }
            }
        }
    }
}
