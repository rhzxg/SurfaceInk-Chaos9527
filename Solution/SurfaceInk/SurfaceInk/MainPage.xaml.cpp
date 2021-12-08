//
// MainPage.xaml.cpp
// MainPage 类的实现。
//

#include "pch.h"
#include "MainPage.xaml.h"
#include <WindowsNumerics.h>

using namespace SurfaceInk;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;

using namespace Platform::Collections;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI::Input::Inking;
using namespace Windows::UI::Input::Inking::Core;
using namespace Windows::UI::Xaml::Navigation;


MainPage::MainPage()
{
	InitializeComponent();
	inkCanvas->InkPresenter->InputDeviceTypes =
		CoreInputDeviceTypes::Mouse |
		CoreInputDeviceTypes::Pen;
	inkCanvas->InkPresenter->InputDeviceTypes = CoreInputDeviceTypes::Mouse | CoreInputDeviceTypes::Pen | CoreInputDeviceTypes::Touch;
	inkCanvas->InkPresenter->StrokesCollected += ref new TypedEventHandler<InkPresenter^, InkStrokesCollectedEventArgs^>(this, &MainPage::InkPresenter_StrokesCollected);
}

void MainPage::OnSizeChanged(Object^ sender, SizeChangedEventArgs^ e)
{
    UpdateFrameworkSize();
}

void MainPage::UpdateFrameworkSize() {
    outputGrid->Width = RootGrid->ActualWidth;
    outputGrid->Height = RootGrid->ActualHeight - inkToolBar->ActualHeight - ReplayProgress->ActualHeight;
    inkCanvas->Width = RootGrid->ActualWidth;
    inkCanvas->Height = RootGrid->ActualHeight - inkToolBar->ActualHeight - ReplayProgress->ActualHeight;

}

//void MainPage::OnNavigatingFrom(NavigatingCancelEventArgs^ e)
//{
//	StopReplay();
//}

void MainPage::InkPresenter_StrokesCollected(InkPresenter^ sender, InkStrokesCollectedEventArgs^ e)
{
    /*IVectorView<InkStroke^>^ strokes = e->Strokes;
    String^ log = L"Strokes collected: " + strokes->Size.ToString();

    auto timestampFormatter = ref new Windows::Globalization::DateTimeFormatting::DateTimeFormatter("day month year hour minute second");

    for (InkStroke^ stroke : strokes)
    {
        log += "\nId: " + stroke->Id.ToString() +
            ", StartedTime: " + timestampFormatter->Format(stroke->StrokeStartedTime->Value) +
            ", Duration: " + (stroke->StrokeDuration->Value.Duration / 10000L) + "ms";
    }
    rootPage->NotifyUser(log, NotifyType::StatusMessage);*/
}

void MainPage::OnReset(Object^ sender, RoutedEventArgs^ e)
{
    StopReplay();
    inkCanvas->InkPresenter->StrokeContainer->Clear();
    ClearCanvasStrokeCache();
    // rootPage->NotifyUser("Cleared Canvas", NotifyType::StatusMessage);
    inkToolBar->TargetInkCanvas = inkCanvas;
}

void MainPage::ClearCanvasStrokeCache()
{
    // Workaround for builds prior to 17650.
    // Throw away the old inkCanvas and create a new one to avoid accumulation of old strokes.
    outputGrid->Children->Clear();
    inkCanvas = ref new Windows::UI::Xaml::Controls::InkCanvas();
    outputGrid->Children->Append(inkCanvas);
    inkCanvas->InkPresenter->InputDeviceTypes = CoreInputDeviceTypes::Mouse | CoreInputDeviceTypes::Pen | CoreInputDeviceTypes::Touch;
    inkCanvas->InkPresenter->StrokesCollected += ref new TypedEventHandler<InkPresenter^, InkStrokesCollectedEventArgs^>(this, &MainPage::InkPresenter_StrokesCollected);
    UpdateFrameworkSize();
}

DateTime GetCurrentDateTime()
{
    FILETIME fileTime;
    GetSystemTimeAsFileTime(&fileTime);
    return DateTime{ (static_cast<uint64_t>(fileTime.dwHighDateTime) << 32) | fileTime.dwLowDateTime };
}

void MainPage::OnReplay(Object^ sender, RoutedEventArgs^ e)
{
    if (!strokeBuilder)
    {
        strokeBuilder = ref new InkStrokeBuilder();
        inkReplayTimer = ref new DispatcherTimer();
        inkReplayTimer->Interval = TimeSpan{ 10000000 / FPS };
        inkReplayTimer->Tick += ref new EventHandler<Object^>(this, &MainPage::InkReplayTimer_Tick);
    }

    strokesToReplay = inkCanvas->InkPresenter->StrokeContainer->GetStrokes();

    ReplayButton->IsEnabled = false;
    inkCanvas->InkPresenter->IsInputEnabled = false;
    ClearCanvasStrokeCache();

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

    ReplayButton->IsEnabled = true;
    inkCanvas->InkPresenter->IsInputEnabled = true;
    ReplayProgress->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    inkToolBar->TargetInkCanvas = inkCanvas;
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


