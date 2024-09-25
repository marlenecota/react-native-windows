// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"
#include "AppearanceModule.h"
#include <XamlUtils.h>
#include <winrt/Windows.UI.ViewManagement.h>
#include "IReactDispatcher.h"

using Application = xaml::Application;
using ApplicationTheme = xaml::ApplicationTheme;
using ElementTheme = xaml::ElementTheme;
using UISettings = winrt::Windows::UI::ViewManagement::UISettings;

namespace Microsoft::ReactNative {

static const React::ReactPropertyId<ApplicationTheme> &AppearanceCurrentThemePropertyId() noexcept {
  static const React::ReactPropertyId<ApplicationTheme> prop{L"ReactNative.Appearance", L"ApplicationTheme"};
  return prop;
}

void Appearance::Initialize(winrt::Microsoft::ReactNative::ReactContext const &reactContext) noexcept {
  m_context = reactContext;

  m_context.UIDispatcher().Post([wkThis = weak_from_this()]() {
    if (auto pThis = wkThis.lock()) {
      pThis->RequeryTheme();
    }
  });

  // UISettings will notify us on a background thread regardless of where we construct it or register for events.
  // Redirect callbacks to the UI thread where we can check app theme.
  m_revoker = m_uiSettings.ColorValuesChanged(
      winrt::auto_revoke,
      [wkThis = weak_from_this(), reactContext](const auto & /*sender*/, const auto & /*args*/) noexcept {
        if (auto pThis = wkThis.lock()) {
          reactContext.UIDispatcher().Post([wkThis]() noexcept {
            if (auto pThis = wkThis.lock()) {
              pThis->RequeryTheme();
            }
          });
        }
      });
}

ApplicationTheme Appearance::ToApplicationTheme(ElementTheme theme) noexcept {
  switch (theme) {
    case ElementTheme::Dark:
      return ApplicationTheme::Dark;
    case ElementTheme::Light:
    default:
      return ApplicationTheme::Light;
  }
}

ApplicationTheme Appearance::GetCurrentTheme() noexcept {
  assert(m_context.UIDispatcher().HasThreadAccess()); // xaml::Application is only accessible on the UI thread
  if (m_useColorScheme) {
    if (auto currentWindow = xaml::TryGetCurrentWindow()) {
      auto requestedThemeRoot{currentWindow.Content().as<xaml::FrameworkElement>()};
      return ToApplicationTheme(requestedThemeRoot.RequestedTheme());
    }
  }

  if (auto currentApp = xaml::TryGetCurrentApplication()) {
    return currentApp.RequestedTheme();
  }

  return ApplicationTheme::Light;
}

const char *Appearance::ToString(ApplicationTheme theme) noexcept {
  return theme == ApplicationTheme::Dark ? "dark" : "light";
}

void Appearance::RequeryTheme() noexcept {
  auto theme = GetCurrentTheme();
  auto oldThemeBoxed =
      m_context.Properties().Handle().Set(AppearanceCurrentThemePropertyId().Handle(), winrt::box_value(theme));
  auto oldTheme = winrt::unbox_value_or<ApplicationTheme>(oldThemeBoxed, ApplicationTheme::Light);

  if (oldTheme != theme) {
    appearanceChanged(ToString(theme));
  }
}

void Appearance::InitOnUIThread(const Mso::React::IReactContext &context) noexcept {
  xaml::ApplicationTheme theme = ApplicationTheme::Light;
  if (auto currentApp = xaml::TryGetCurrentApplication()) {
    theme = currentApp.RequestedTheme();
  }

  winrt::Microsoft::ReactNative::ReactPropertyBag pb{context.Properties()};
  pb.Set(AppearanceCurrentThemePropertyId(), theme);
}

void Appearance::setColorScheme(std::string style) noexcept {
  auto theme{ElementTheme::Default};
  if (style == "light") {
    theme = ElementTheme::Light;
  } else if (style == "dark") {
    theme = ElementTheme::Dark;
  }

  m_context.UIDispatcher().Post([theme, this]() {
    auto requestedThemeRoot{xaml::Window::Current().Content().as<xaml::FrameworkElement>()};
    if (requestedThemeRoot.RequestedTheme() != theme) {
      m_useColorScheme = theme != ElementTheme::Default;
      winrt::Microsoft::ReactNative::XamlHelper::UseColorScheme(m_useColorScheme);
      requestedThemeRoot.RequestedTheme(theme);
      RequeryTheme();
    }
  });
}

std::optional<std::string> Appearance::getColorScheme() noexcept {
  return ToString(*(m_context.Properties().Get(AppearanceCurrentThemePropertyId())));
}

void Appearance::addListener(std::string eventName) noexcept {
  // no-op
}

void Appearance::removeListeners(double count) noexcept {
  // no-op
}

} // namespace Microsoft::ReactNative
