//
// MainPage.xaml.cpp
// MainPage 类的实现。
//

#include "pch.h"
#include "MainPage.xaml.h"
#include <string>
#include "TCPClient.h"


using namespace SurfaceInk;
using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Input::Inking;
using namespace Windows::UI::Input::Inking::Core;
using namespace Windows::UI::Popups;


TCPClient tcpClient;

MainPage::MainPage()
{
	InitializeComponent();
	inkCanvas->InkPresenter->InputDeviceTypes = CoreInputDeviceTypes::Mouse | CoreInputDeviceTypes::Pen | CoreInputDeviceTypes::Touch;
	inkCanvas->InkPresenter->StrokesCollected += ref new TypedEventHandler<InkPresenter^, InkStrokesCollectedEventArgs^>(this, &MainPage::InkPresenter_StrokesCollected);
}

void MainPage::OnSizeChanged(Object^ sender, SizeChangedEventArgs^ e)
{
    UpdateFrameworkSize();
}

void MainPage::UpdateFrameworkSize() {
    outputGrid->Width = RootGrid->ActualWidth;
    outputGrid->Height = RootGrid->ActualHeight - inkToolBar->ActualHeight; // - ReplayProgress->ActualHeight
    inkCanvas->Width = RootGrid->ActualWidth;
    inkCanvas->Height = RootGrid->ActualHeight - inkToolBar->ActualHeight; // - ReplayProgress->ActualHeight
}

void MainPage::InkPresenter_StrokesCollected(InkPresenter^ sender, InkStrokesCollectedEventArgs^ e)
{
    // Send strokes to remote server.
    /*if (isConnected) {
        strokesToReplay = inkCanvas->InkPresenter->StrokeContainer->GetStrokes();
        const int sz = sizeof(strokesToReplay);
        char msgToSend[sz];
        memcpy(msgToSend, &strokesToReplay, sz);
        send(TcpClient.sockfd, msgToSend, sz, 0);
    }*/
}

void SurfaceInk::MainPage::OnConnect()
{
    // Use string to convert Platform::String to char *.
    std::wstring wsstr(textBox->Text->Data());
    std::string res(wsstr.begin(), wsstr.end());

    // Check connection state.
    int connState = tcpClient.StartTcpConnection(res.c_str());
    if (connState == 0) {
        isConnected = true;
        textBlock->Foreground = ref new SolidColorBrush(Windows::UI::Colors::Green);
        textBlock2->Foreground = ref new SolidColorBrush(Windows::UI::Colors::Green);
        textBlock2->Text = L"Connected to Sever";
    }
    else  if (connState == -1) {
        MessageDialog^ msg = ref new MessageDialog("Socket Initialization Failed");
        msg->ShowAsync();
    }
    else if (connState == -2) {
        MessageDialog^ msg = ref new MessageDialog("Invalid IPv4 Address");
        msg->ShowAsync();
        textBox->Text = L"";
    }
    else if (connState == -3) {
        MessageDialog^ msg = ref new MessageDialog("Connection Falied");
        msg->ShowAsync();
    }
}