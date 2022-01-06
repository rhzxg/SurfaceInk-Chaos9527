//
// MainPage.xaml.h
// MainPage 类的声明。
//

#pragma once

#include "MainPage.g.h"
#include <string>
#include <set>
#include <utility>
#include <mutex>

namespace ServerInk
{
    /// <summary>
    /// 可用于自身或导航至 Frame 内部的空白页。
    /// </summary>
    public ref class MainPage sealed
    {
    public:
        MainPage();
    private:
        void OnLoaded(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);
        void OnSizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);
        void UpdateFrameworkSize();
        void OnReset(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void OnReplay();
        void RunDispatcher(int state);
        void ReceiveFrom();
        void InkReplayTimer_Tick(Platform::Object^ sender, Platform::Object^ e);

        void StopReplay();
        void ClearCanvasStrokeCache();

        void StoreStreamToGif(int size);

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
        Windows::UI::Xaml::Controls::InkCanvas^ inkCanvasCopy;
        bool isConnected = false;

        MainPage^ rootPage;

        std::string fullFileName;
        std::set<std::pair<long long, long long>> strokesSet;

        // Resource locks:
        std::mutex fileReadMutex;
    };
}
