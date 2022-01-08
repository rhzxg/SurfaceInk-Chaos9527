//
// MainPage.xaml.cpp
// MainPage 类的实现。
//

#pragma warning(disable:4996)

#include "pch.h"
#include "MainPage.xaml.h"
#include <string>
#include <cstdlib>
#include "TCPClient.h"


using namespace concurrency;
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
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;


TCPClient tcpClient;
StorageFolder^ folder;

MainPage::MainPage()
{
	InitializeComponent();
	inkCanvas->InkPresenter->InputDeviceTypes = CoreInputDeviceTypes::Mouse | CoreInputDeviceTypes::Pen | CoreInputDeviceTypes::Touch;
	inkCanvas->InkPresenter->StrokesCollected += ref new TypedEventHandler<InkPresenter^, InkStrokesCollectedEventArgs^>(this, &MainPage::InkPresenter_StrokesCollected);
    GetFullPath();
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
    // It's almost impossible to exchange stroke(s) between devices.
    // Send a .gif file is recommanded by Microsoft official.

    create_task(folder->CreateFileAsync("strokes.gif", Windows::Storage::CreationCollisionOption::ReplaceExisting)).then(
        [this, folder = folder, res = fullFileName](StorageFile^ q) {
            create_task(folder->GetFileAsync("strokes.gif")).then(
                [this, res](StorageFile^ storageFile) {
                    create_task(storageFile->OpenAsync(FileAccessMode::ReadWrite)).then(
                        [this, res](IRandomAccessStream^ stream) {
                            create_task(inkCanvas->InkPresenter->StrokeContainer->SaveAsync(stream)).then(
                                [this, res, stream](unsigned int a) {
                                    SendStrokes(res);
                                }
                            );
                        }
                    );
                }
            );
        }
    );
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
        connectButton->IsEnabled = false;
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

void SurfaceInk::MainPage::GetFullPath()
{
    //auto sp = ref new FolderPicker();
    //create_task(sp->PickSingleFolderAsync()).then(
    //    [this](StorageFolder^ f) {
    //        folder = f;
    //        // Cast Platform::String^ to std::string
    //        std::wstring wsstr(folder->Path->Data());
    //        std::string res(wsstr.begin(), wsstr.end());
    //        res += "\\strokes.gif";
    //        fullFileName = res;
    //    }
    //);
    
    //folder = ApplicationData::Current->LocalFolder;
    //MessageDialog^ msg = ref new MessageDialog(folder->Path);
    //msg->ShowAsync();

    // Cast Platform::String^ to std::string
    std::wstring wsstr(folder->Path->Data());
    std::string res(wsstr.begin(), wsstr.end());
    res += "\\strokes.gif";
    fullFileName = res;
}

void SurfaceInk::MainPage::RefreshUIState()
{
    Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this]()
        {
            textBlock->Foreground = ref new SolidColorBrush(Windows::UI::Colors::Orange);
            textBlock2->Foreground = ref new SolidColorBrush(Windows::UI::Colors::Orange);
            textBlock2->Text = L"Waitting For Connection";
            connectButton->IsEnabled = true;

            closesocket(tcpClient.connfd);
            isConnected = false;

            MessageDialog^ msg = ref new MessageDialog("Server disconnected");
            msg->ShowAsync();

            outputGrid->Children->Clear();
            inkCanvas = ref new Windows::UI::Xaml::Controls::InkCanvas();
            inkCanvas->InkPresenter->InputDeviceTypes = CoreInputDeviceTypes::Mouse | CoreInputDeviceTypes::Pen | CoreInputDeviceTypes::Touch;
            inkCanvas->InkPresenter->StrokesCollected += ref new TypedEventHandler<InkPresenter^, InkStrokesCollectedEventArgs^>(this, &MainPage::InkPresenter_StrokesCollected);
            inkToolBar->TargetInkCanvas = inkCanvas;
            outputGrid->Children->Append(inkCanvas);
            UpdateFrameworkSize();
        }));
}

int SurfaceInk::MainPage::SendStrokes(std::string res)
{
    std::lock_guard<std::mutex> lock(fileReadMutex);
    // UWP applications can only access files in its local folder.
    FILE* f = nullptr;
    f = fopen(res.c_str(), "rb");

    if (!f) {
        MessageDialog^ msg = ref new MessageDialog("Stroke file doesn't exist");
        msg->ShowAsync();

        res += " ";
        res += std::to_string(errno);
        std::wstring w_str = std::wstring(res.begin(), res.end());
        const wchar_t* w_chars = w_str.c_str();
        MessageDialog^ msg2 = ref new MessageDialog(ref new Platform::String(w_chars));
        msg2->ShowAsync();
    }
    else {
        fseek(f, 0, SEEK_END);
        int n = ftell(f);
        fseek(f, 0, SEEK_SET);
        int len;
        memset(tcpClient.buffer, '\0', sizeof(tcpClient.buffer));
        if ((len = fread(&tcpClient.buffer, 1, n, f)) && len > 0)
        {
            if (isConnected) {
                int socketState = send(tcpClient.sockfd, tcpClient.buffer, n, 0);
                if (socketState == SOCKET_ERROR)
                {
                    RefreshUIState();
                }
            }
        }
        fclose(f);
    }
    return 0;
}
