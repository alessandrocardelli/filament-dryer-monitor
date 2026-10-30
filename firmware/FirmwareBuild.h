#pragma once
// Human-visible identity of this *specific* compiled source image.
// Update this string whenever test behavior changes, including a safety fix.
// Do not derive it from __DATE__ (two different builds may look identical).
namespace FirmwareBuild {
constexpr char kId[] = "HOT-261030-AIR55";
constexpr char kShortId[] = "H30-A55";
constexpr char kMode[] = "ATTENDED AIR40-55 HOT";
constexpr unsigned long kSplashExtraMs = 3000UL;
}  // namespace FirmwareBuild
