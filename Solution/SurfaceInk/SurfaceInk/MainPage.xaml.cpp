//
// MainPage.xaml.cpp
// MainPage 类的实现。
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "Configuration.h"

using namespace SurfaceInk;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;

MainPage::MainPage()
{
	InitializeComponent();
	inkCanvas->InkPresenter->InputDeviceTypes =
		CoreInputDeviceTypes::Mouse |
		CoreInputDeviceTypes::Pen;
}

void MainPage::OnSizeChanged(Object^ sender, SizeChangedEventArgs^ e)
{
	HelperFunctions::UpdateCanvasSize(RootGrid, outputGrid, inkCanvas);
}