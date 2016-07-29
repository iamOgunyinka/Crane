import bb.cascades 1.2

Page {
    titleBar: TitleBar {
        kind: TitleBarKind.Default
        title: "Using Crane"
    }
    attachedObjects: [
        TextStyleDefinition {
            fontWeight: FontWeight.Bold
            base: SystemDefaults.TextStyles.BodyText
            fontStyle: FontStyle.Italic
            id: allTextDef
        }
    ]
    Container {
        rightPadding: 10
        leftPadding: 10
        Header {
            title: "Frequently Asked Questions."
        }
        ScrollView {
            topMargin: 20
            leftMargin: 20
            rightMargin: 20
            
            Container {
                Label {
                    text: "Q: What is Crane Download Manager?"
                    textStyle.base: allTextDef.style
                }
                TextArea {
                    text: "Crane Download Manager is an app that allows you to download big/small files in a more manageable way. " +
                    "Native browsers comes with built-in download manager but oftentimes there are no easy ways to manage downloads in" +
                    " a more compact way like we'd want."
                    editable: false
                    textStyle.textAlign: TextAlign.Justify
                }
                Label {
                    text: "Q: How does Crane DM allow me manage my downloads?"
                    textStyle.base: allTextDef.style
                }
                TextArea {
                    text: "In built-in browsers, you may start and stop downloads directly, but what happens when your phone" +
                    " is dying off or there was a network problem when your download is just halfway through? In built-in browsers, " +
                    "there are two options: (1) forfeit the download or (2) pray-hope miracle happens." +
                    " This is where Crane Download Manager come into play. Like every other paid/free Download managers, " + 
                    "you can pause your download(s) when your battery is dying off or you realize your network had just " + 
                    "fluctuated. Note that not all servers support resuming paused downloads, so check the details for the " +
                    "download( see next questions )."
                    editable: false
                    textStyle.textAlign: TextAlign.Justify
                }
                Label {
                    text: "Q: How do I start a new download?"
                    textStyle.base: allTextDef.style
                }
                TextArea {
                    text: "There are two ways of adding a new download.\n\t" +
                    "(1) When surfing the web, you can directly copy the link to a" +
                    " file, then proceed to opening the App, on the top of the front-page is a '+' icon. If the app settings is " +
                    "configured to automatically paste URL from clipboard, when the pop-up shows, just click on OK and wait for " +
                    "few seconds for the app to gather enough information from the URL. Otherwise, paste the link and click on" +
                    " OK.\n\t" +
                    "(2) With the built-in Blackberry browser, when you hold on to the download link or when your media starts " +
                    "playing, simply \"Share\" with \"Crane Download Manager\"."
                    editable: false
                    textStyle.textAlign: TextAlign.Justify
                }
                Label {
                    text: "Q: How do I check the download details?"
                    textStyle.base: allTextDef.style
                }
                TextArea {
                    text: "Click on the specific download in the list of items presented to you?"
                    editable: false
                    textStyle.textAlign: TextAlign.Justify
                }
                Label {
                    text: "Q: How do I pause or stop a download?"
                    textStyle.base: allTextDef.style
                }
                TextArea {
                    text: "In the list of download items on the front page, hold still on to the download. You'll be presented" +
                    " with a list of context actions that slides in from the right hand side of the screen, you can then resume" +
                    "(play button), stop or pause downloads. If a download is in progress, when you try to resume/start that same" +
                    " download, the command is ignored. Right now, the stop and pause command do the same thing."
                    editable: false
                    textStyle.textAlign: TextAlign.Justify
                }
                Label {
                    text: "Q: How do I change the theme?"
                    textStyle.base: allTextDef.style
                }
                TextArea {
                    text: "Go to \"Settings\". Select the app theme under the \"Application Settings\" header and choose between " +
                    "the \"Dark\" and the \"Bright\" theme. For backward compatibility with Blackberry OS 10.1.x and 10.2.x, " +
                    "I decided that changes to the app theme only needs to be visible when the app is restarted."
                    editable: false
                    textStyle.textAlign: TextAlign.Justify
                }
                
                Label {
                    text: "Q: Why are some downloads erroneous?"
                    textStyle.base: allTextDef.style
                }
                TextArea {
                    text: "Downloading files from different sources can be a very difficult task. Programmatically, there is a " +
                    "'tag' associated with every document fetched on the web, it's called an HTTP Header. It contains almost all " +
                    "the necessary information for the file you're fetching/downloading, such as its name, its location etcetera." +
                    " Some servers that hosts these files sometimes choose not to deliver a handful header, which makes " +
                    "downloading these files extremely difficult. Naively, I depended on the availability of these headers, and " +
                    "this caused unncessary crashes in previous versions of CDM. Now, I just chose stability over crashes when " +
                    "a header isn't helpful OR when unnecessary redirections occur. Bear with when CDM doesn't download the " +
                    "files you want, I advice you resort to your browser for help. Apologies."
                    
                    editable: false
                    textStyle.textAlign: TextAlign.Justify
                }
                Label {
                    text: "Q: Oh my! Why does the app behave weirdly with FTP?"
                    textStyle.base: allTextDef.style
                }
                TextArea {
                    text: "*sigh* Apologies. The support for FTP is really poor in the native library used to develop this " + 
                    "application. The first version of the app doesn't support it at all, it took lots of hack to get the few " +
                    " features to work and since the native library has been put on hold, please endure the pains because it " +
                    " doesn't look like it'll go away anytime soon."
                    editable: false
                    textStyle.textAlign: TextAlign.Justify
                }
                Label {
                    text: "Q: How often are the updates?"
                    textStyle.base: allTextDef.style
                }
                TextArea {
                    text: "This will probably be the last update. I'm cooking up some real cross-platform apps and this is " +
                    "gonna be my full-time job in the coming months."
                    editable: false
                    textStyle.textAlign: TextAlign.Justify
                }
                
                Label {
                    text: "Bobby: \"I want a change, you Idjit!\""
                    textStyle.base: allTextDef.style
                }
                TextArea {
                    text: "I try my best to read all reviews and work on them, but to hasten your request, please push a mail to: " +
                    "ogunyinkajoshua@yahoo.com OR speak with me directly on: https://facebook.com/TheRealOgunyinka/"
                    editable: false
                    textStyle.textAlign: TextAlign.Justify
                }
            }
        }
    }
}
