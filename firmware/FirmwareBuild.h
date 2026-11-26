#pragma once
// Human-visible identity of this *specific* compiled source image.
// Update this string whenever test behavior changes, including a safety fix.
// Do not derive it from __DATE__ (two different builds may look identical).
namespace FirmwareBuild {
constexpr char kId[] = "NTC-261126-UI4";
constexpr char kShortId[] = "N26-UI4";
constexpr char kMode[] = "VISHAY HEAT OFF";
constexpr unsigned long kSplashExtraMs = 3000UL;
}  // namespace FirmwareBuild
