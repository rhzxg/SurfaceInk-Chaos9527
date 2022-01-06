//
// MainPage.xaml.cpp
// MainPage 类的实现。
//

#include "pch.h"
#include "MainPage.xaml.h"
#include <WindowsNumerics.h>
#include "TCPServer.h"
#include <ppltasks.h>
#include <cstdlib>


using namespace ServerInk;
using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Numerics;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Input::Inking;
using namespace Windows::UI::Input::Inking::Core;
using namespace Windows::UI::Popups;

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::DataTransfer;

using namespace concurrency;

using namespace Windows::Storage;
using namespace Windows::Storage::Streams;

TCPServer tcpServer;
StorageFolder^ folder;


MainPage::MainPage()
{
    InitializeComponent();
    // inkCanvas->InkPresenter->InputDeviceTypes = CoreInputDeviceTypes::Mouse | CoreInputDeviceTypes::Pen | CoreInputDeviceTypes::Touch;
    
    // Use string to convert char * to Platform::String
    std::string s_str = "IP Address : " + tcpServer.ip;
    std::wstring wid_str = std::wstring(s_str.begin(), s_str.end());
    const wchar_t* w_char = wid_str.c_str();
    textBlock->Text = ref new Platform::String(w_char);

    // Get full path
    folder = ApplicationData::Current->LocalFolder;

    //// For debug only
    //MessageDialog^ msg = ref new MessageDialog(folder->Path);
    //msg->ShowAsync();

    // Cast Platform::String^ to std::string
    std::wstring wsstr(folder->Path->Data());
    std::string res(wsstr.begin(), wsstr.end());
    res += "\\strokes.gif";
    fullFileName = res;
}

void MainPage::RunDispatcher(int state)
{
    switch (state) {
    case 1: {
        Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this]()
            {
                textBlock2->Foreground = ref new SolidColorBrush(Windows::UI::Colors::Blue);
                textBlock3->Foreground = ref new SolidColorBrush(Windows::UI::Colors::Blue);
                textBlock3->Text = L"Accepting for connection";
                listenButton->IsEnabled = false;
            }));
        break;
    }
    case 2: {
        Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this]()
            {
                textBlock2->Foreground = ref new SolidColorBrush(Windows::UI::Colors::Green);
                textBlock3->Foreground = ref new SolidColorBrush(Windows::UI::Colors::Green);
                textBlock3->Text = L"Connected To Client";
            }));
        break;
    }
    }
}

void MainPage::ReceiveFrom()
{
    
    while (true) {
        int bytesRecv = tcpServer.recvFromClient();
        int a = 1;
        if (bytesRecv == -1) {
            Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this]()
                {
                    MessageDialog^ msg = ref new MessageDialog("Error occered during receiving from client");
                    msg->ShowAsync();
                }));
            break;
        }
        if (bytesRecv == 0) {
            Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this]()
                {
                    MessageDialog^ msg = ref new MessageDialog("Client disconnected");
                    msg->ShowAsync();
                }));
            isConnected = false;
            
            break;
        }

        Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([&]()
            {
                // Network stream to .gif file.
                StoreStreamToGif(bytesRecv);

                // .gif file to stroke stream:
                create_task(folder->GetFileAsync("strokes.gif")).then(
                    [this](StorageFile^ storageFile) {
                        create_task(storageFile->OpenSequentialReadAsync()).then(
                            [this](IInputStream^ stream) {
                                inkCanvasCopy = ref new InkCanvas;
                                create_task(inkCanvasCopy->InkPresenter->StrokeContainer->LoadAsync(stream)).then(
                                    [this, stream](task<void> loadTask) {
                                        delete stream;
                                        strokesToReplay = inkCanvasCopy->InkPresenter->StrokeContainer->GetStrokes();
                                        try
                                        {
                                            OnReplay();
                                        }
                                        catch (Platform::Exception^ ex)
                                        {
                                            // Report any I/O errors.
                                            MessageDialog^ msg = ref new MessageDialog("I/O error occurred");
                                            msg->ShowAsync();
                                        }
                                    }
                                );
                            }
                        );
                    }
                );

                //strokesToReplay = inkCanvasCopy->InkPresenter->StrokeContainer->GetStrokes();
                //OnReplay();
            }));

        //Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this]()
        //    {
        //        MessageDialog^ msg = ref new MessageDialog("Received");
        //        msg->ShowAsync();
        //    }));

    }
}

void MainPage::OnLoaded(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e)
{
    // Hand the UI change to main thread by using the message dispatcher.
    // .then() metheod will be called automatically after the tread returns.
    create_task([&]() {
        RunDispatcher(1);
        tcpServer.StartTcpConnection();
        }).then([&]() {
            RunDispatcher(2);
            ReceiveFrom();
            });
}

void MainPage::OnSizeChanged(Object^ sender, SizeChangedEventArgs^ e)
{
    UpdateFrameworkSize();
}

void MainPage::UpdateFrameworkSize() {
    outputGrid->Width = RootGrid->ActualWidth;
    outputGrid->Height = RootGrid->ActualHeight - stackPanel->ActualHeight - ReplayProgress->ActualHeight;
    inkCanvas->Width = RootGrid->ActualWidth;
    inkCanvas->Height = RootGrid->ActualHeight - stackPanel->ActualHeight - ReplayProgress->ActualHeight;

}

//void MainPage::OnNavigatingFrom(NavigatingCancelEventArgs^ e)
//{
//	StopReplay();
//}

void MainPage::OnReset(Object^ sender, RoutedEventArgs^ e)
{
    StopReplay();
    inkCanvas->InkPresenter->StrokeContainer->Clear();
    ClearCanvasStrokeCache();
    // rootPage->NotifyUser("Cleared Canvas", NotifyType::StatusMessage);
    // inkToolBar->TargetInkCanvas = inkCanvas;
}

void MainPage::ClearCanvasStrokeCache()
{
    // Workaround for builds prior to 17650.
    // Throw away the old inkCanvas and create a new one to avoid accumulation of old strokes.
    outputGrid->Children->Clear();
    inkCanvas = ref new Windows::UI::Xaml::Controls::InkCanvas();
    outputGrid->Children->Append(inkCanvas);
    //inkCanvas->InkPresenter->InputDeviceTypes = CoreInputDeviceTypes::Mouse | CoreInputDeviceTypes::Pen | CoreInputDeviceTypes::Touch;
    // inkCanvas->InkPresenter->StrokesCollected += ref new TypedEventHandler<InkPresenter^, InkStrokesCollectedEventArgs^>(this, &MainPage::InkPresenter_StrokesCollected);
    UpdateFrameworkSize();
}

void ServerInk::MainPage::StoreStreamToGif(int size)
{
    std::lock_guard<std::mutex> lock(fileReadMutex);
    FILE* f = nullptr;
    f = fopen(fullFileName.c_str(), "wb");

    if (!f) {
        MessageDialog^ msg = ref new MessageDialog("Cannot write stroke file");
        msg->ShowAsync();
    }
    else {
        char c = tcpServer.buffer[10];
        fwrite(&tcpServer.buffer, 1, size, f);
    }
    fclose(f);
}

DateTime GetCurrentDateTime()
{
    FILETIME fileTime;
    GetSystemTimeAsFileTime(&fileTime);
    return DateTime{ (static_cast<uint64_t>(fileTime.dwHighDateTime) << 32) | fileTime.dwLowDateTime };
}

void MainPage::OnReplay()
{
    if (!strokeBuilder)
    {
        strokeBuilder = ref new InkStrokeBuilder();
        inkReplayTimer = ref new DispatcherTimer();
        inkReplayTimer->Interval = TimeSpan{ 10000000 / FPS };
        inkReplayTimer->Tick += ref new EventHandler<Object^>(this, &MainPage::InkReplayTimer_Tick);
    }

    // strokesToReplay = inkCanvas->InkPresenter->StrokeContainer->GetStrokes();

    // Calculate the beginning of the earliest stroke and the end of the latest stroke.
    // This establishes the time period during which the strokes were collected.
    beginTimeOfRecordedSession = DateTime{ MAXINT64 };
    endTimeOfRecordedSession = DateTime{ MININT64 };
    for (InkStroke^ stroke : strokesToReplay)
    {
        IBox<DateTime>^ startTime = stroke->StrokeStartedTime;
        IBox<TimeSpan>^ duration = stroke->StrokeDuration;
        if (startTime && duration)
        {
            // Check if the stroke is already drawn.
            if (strokesSet.count({ startTime->Value.UniversalTime , duration->Value.Duration }))
            {
                continue;
            }
            else
            {
                strokesSet.insert({ startTime->Value.UniversalTime , duration->Value.Duration });
            }

            if (beginTimeOfRecordedSession.UniversalTime > startTime->Value.UniversalTime)
            {
                beginTimeOfRecordedSession.UniversalTime = startTime->Value.UniversalTime;
            }
            if (endTimeOfRecordedSession.UniversalTime < startTime->Value.UniversalTime + duration->Value.Duration)

            {
                endTimeOfRecordedSession.UniversalTime = startTime->Value.UniversalTime + duration->Value.Duration;
            }
        }
    }

    // If we found at least one stroke with a timestamp, then we can replay.
    if (beginTimeOfRecordedSession.UniversalTime != MAXINT64)
    {
        durationOfRecordedSession = TimeSpan{ endTimeOfRecordedSession.UniversalTime - beginTimeOfRecordedSession.UniversalTime };

        ReplayProgress->Maximum = static_cast<double>(durationOfRecordedSession.Duration);
        ReplayProgress->Value = 0.0;
        ReplayProgress->Visibility = Windows::UI::Xaml::Visibility::Visible;

        beginTimeOfReplay = GetCurrentDateTime();

        // ReplayButton->IsEnabled = false;
        inkCanvas->InkPresenter->IsInputEnabled = false;
        //ClearCanvasStrokeCache();

        inkReplayTimer->Start();

        // rootPage->NotifyUser("Replay started.", NotifyType::StatusMessage);
    }
    else
    {
        // There was nothing to replay. Either there were no strokes at all,
        // or none of the strokes had timestamps.
        StopReplay();
    }
}

void MainPage::StopReplay()
{
    if (inkReplayTimer)
    {
        inkReplayTimer->Stop();
    }

    // ReplayButton->IsEnabled = true;
    // inkCanvas->InkPresenter->IsInputEnabled = true;
    ReplayProgress->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    // inkToolBar->TargetInkCanvas = inkCanvas;
}

void MainPage::InkReplayTimer_Tick(Object^ sender, Object^ e)
{
    DateTime currentTimeOfReplay = GetCurrentDateTime();
    TimeSpan timeElapsedInReplay = TimeSpan{ currentTimeOfReplay.UniversalTime - beginTimeOfReplay.UniversalTime };

    ReplayProgress->Value = static_cast<double>(timeElapsedInReplay.Duration);

    DateTime timeEquivalentInRecordedSession = DateTime{ beginTimeOfRecordedSession.UniversalTime + timeElapsedInReplay.Duration };
    inkCanvas->InkPresenter->StrokeContainer = GetCurrentStrokesView(timeEquivalentInRecordedSession);
    inkCanvas->InkPresenter->IsInputEnabled = false;
    if (timeElapsedInReplay.Duration > durationOfRecordedSession.Duration)
    {
        StopReplay();
        // rootPage->NotifyUser("Replay finished.", NotifyType::StatusMessage);
    }
}

InkStrokeContainer^ MainPage::GetCurrentStrokesView(DateTime time)
{
    auto inkStrokeContainer = ref new InkStrokeContainer();

    // The purpose of this sample is to demonstrate the timestamp usage,
    // not the algorithm. (The time complexity of the code is O(N^2).)
    for (auto stroke : strokesToReplay)
    {
        InkStroke^ s = GetPartialStroke(stroke, time);
        if (s)
        {
            inkStrokeContainer->AddStroke(s);
        }
    }

    return inkStrokeContainer;
}

InkStroke^ MainPage::GetPartialStroke(InkStroke^ stroke, DateTime time)
{
    IBox<DateTime>^ startTime = stroke->StrokeStartedTime;
    IBox<TimeSpan>^ duration = stroke->StrokeDuration;
    if (!startTime || !duration)
    {
        // If a stroke does not have valid timestamp, then treat it as
        // having been drawn before the playback started.
        // We must return a clone of the stroke, because a single stroke cannot
        // exist in more than one container.
        return stroke->Clone();
    }

    if (time.UniversalTime < startTime->Value.UniversalTime)
    {
        // Stroke has not started 
        return nullptr;
    }

    if (time.UniversalTime >= startTime->Value.UniversalTime + duration->Value.Duration)
    {
        // Stroke has already ended.
        // We must return a clone of the stroke, because a single stroke cannot exist in more than one container.
        return stroke->Clone();
    }

    // Stroke has started but not yet ended.
    // Create a partial stroke on the assumption that the ink points are evenly distributed in time.
    IVectorView<InkPoint^>^ points = stroke->GetInkPoints();
    double portion = static_cast<double>(time.UniversalTime - startTime->Value.UniversalTime) / duration->Value.Duration;
    auto count = (int)((points->Size - 1) * portion) + 1;
    auto initialPoints = ref new VectorView<InkPoint^>(begin(points), begin(points) + count);
    // Pass the attributes of the stroke to stroke builder.
    auto inkDrawingAttributes = stroke->DrawingAttributes;
    strokeBuilder->SetDefaultDrawingAttributes(inkDrawingAttributes);
    return strokeBuilder->CreateStrokeFromInkPoints(initialPoints, float3x2::identity());
}