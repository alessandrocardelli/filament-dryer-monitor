#pragma once
// Human-visible identity of this *specific* compiled source image.
// Update this string whenever test behavior changes, including a safety fix.
// Do not derive it from __DATE__ (two different builds may look identical).
namespace FirmwareBuild {
constexpr char kId[] = "HOT-261101-UI3";
constexpr char kShortId[] = "H01-UI3";
constexpr char kMode[] = "NTC90/85 UI";
constexpr unsigned long kSplashExtraMs = 3000UL;
}  // namespace FirmwareBuild
