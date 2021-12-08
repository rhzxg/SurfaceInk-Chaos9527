//
// MainPage.xaml.h
// MainPage 类的声明。
//

#pragma once

#include "MainPage.g.h"

namespace SurfaceInk
{
	/// <summary>
	/// 可用于自身或导航至 Frame 内部的空白页。
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();
	private:
		void OnSizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);
        void InkPresenter_StrokesCollected(Windows::UI::Input::Inking::InkPresenter^ sender, Windows::UI::Input::Inking::InkStrokesCollectedEventArgs^ e);
        void OnReset(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void OnReplay(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
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

    };
}
