//
// MainPage.xaml.h
// MainPage 类的声明。
//

#pragma once

#include "MainPage.g.h"
#include <string>
#include <mutex>


namespace SurfaceInk
{
	[Windows::Foundation::Metadata::WebHostHidden]
	// In c++, adding this attribute to ref classes enables data binding.
	// For more info search for 'Bindable' on the page http://go.microsoft.com/fwlink/?LinkId=254639 
	[Windows::UI::Xaml::Data::Bindable]
	public ref class LocalHostItem sealed
	{
	public:
		LocalHostItem(Windows::Networking::HostName^ localHostName);
		property Platform::String^ DisplayString { Platform::String^ get() { return displayString; } }
		property Windows::Networking::HostName^ LocalHost { Windows::Networking::HostName^ get() { return localHost; } }

	private:
		Platform::String^ displayString;
		Windows::Networking::HostName^ localHost;
	};

	/// <summary>
	/// 可用于自身或导航至 Frame 内部的空白页。
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
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
		void InitComboBox();
		void RefreshUIState();
		int SendStrokes(std::string res);
        
		Windows::Foundation::Collections::IVectorView<Windows::UI::Input::Inking::InkStroke^>^ strokesToReplay;
		
		bool isConnected = false;

		std::string fullFileName;

		// Resource locks:
		std::mutex fileReadMutex;

		// Vector containing all available local HostName endpoints.
		Platform::Collections::Vector<LocalHostItem^>^ localHostItems;
    };
}
