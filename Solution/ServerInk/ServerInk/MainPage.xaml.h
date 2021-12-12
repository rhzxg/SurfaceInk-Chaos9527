//
// MainPage.xaml.h
// MainPage 类的声明。
//

#pragma once

#include "MainPage.g.h"
#include <string>

namespace ServerInk
{
    /// <summary>
    /// 可用于自身或导航至 Frame 内部的空白页。
    /// </summary>
    public ref class MainPage sealed
    {
    public:
        MainPage();
        void p();
    private:
        void OnLoaded(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);
        void StartServer();
        void StreamSocketListener_ConnectionReceived(Windows::Networking::Sockets::StreamSocketListener^ sender, Windows::Networking::Sockets::StreamSocketListenerConnectionReceivedEventArgs^ args);
        void OnSizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);
        void UpdateFrameworkSize();
        void InkPresenter_StrokesCollected(Windows::UI::Input::Inking::InkPresenter^ sender, Windows::UI::Input::Inking::InkStrokesCollectedEventArgs^ e);
        void OnReset(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void OnReplay();
        void InkReplayTimer_Tick(Platform::Object^ sender, Platform::Object^ e);

        void StopReplay();
        void ClearCanvasStrokeCache();
        Windows::UI::Input::Inking::InkStrokeContainer^ GetCurrentStrokesView(Windows::Foundation::DateTime time);
        Windows::UI::Input::Inking::InkStroke^ GetPartialStroke(Windows::UI::Input::Inking::InkStroke^ stroke, Windows::Foundation::DateTime time);

        // MainPage^ rootPage = MainPage::Current;

        static const int FPS = 60;

        Windows::Foundation::DateTime beginTimeOfRecordedSession;
        Windows::Foundation::DateTime endTimeOfRecordedSession;
        Windows::Foundation::TimeSpan durationOfRecordedSession;
        Windows::Foundation::DateTime beginTimeOfReplay;

        Windows::UI::Xaml::DispatcherTimer^ inkReplayTimer;

        Windows::UI::Input::Inking::InkStrokeBuilder^ strokeBuilder;
        Windows::Foundation::Collections::IVectorView<Windows::UI::Input::Inking::InkStroke^>^ strokesToReplay;
        Windows::Foundation::Collections::IVectorView<Windows::UI::Input::Inking::InkStroke^>^ strokesToReplayTemp;

        bool isConnected = false;
    };
}
