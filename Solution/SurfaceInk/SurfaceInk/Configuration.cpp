#include "pch.h"
#include "Configuration.h"
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;

void HelperFunctions::UpdateCanvasSize(FrameworkElement^ root, FrameworkElement^ output, FrameworkElement^ inkCanvas)
{
    output->Width = root->ActualWidth;
    output->Height = root->ActualHeight * 0.95;
    inkCanvas->Width = root->ActualWidth;
    inkCanvas->Height = root->ActualHeight * 0.95;
}
