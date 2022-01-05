//
// MainPage.xaml.h
// MainPage 类的声明。
//

#pragma once

#include "MainPage.g.h"

#include <string>

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
        void UpdateFrameworkSize();
        void InkPresenter_StrokesCollected(Windows::UI::Input::Inking::InkPresenter^ sender, Windows::UI::Input::Inking::InkStrokesCollectedEventArgs^ e);
		void OnConnect();
		void GetFullPath();
		int SendStrokes(std::string res);
        
		
		Windows::Foundation::Collections::IVectorView<Windows::UI::Input::Inking::InkStroke^>^ strokesToReplay;
		
		bool isConnected = false;

		std::string fullFileName;
    };
}
