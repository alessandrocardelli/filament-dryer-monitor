#pragma once
// Human-visible identity of this *specific* compiled source image.
// Update this string whenever test behavior changes, including a safety fix.
// Do not derive it from __DATE__ (two different builds may look identical).
namespace FirmwareBuild {
constexpr char kId[] = "THERM-241024-R1";
constexpr char kShortId[] = "T24-R1";
constexpr char kMode[] = "NTC COLD CHECK";
constexpr unsigned long kSplashExtraMs = 3000UL;
}  // namespace FirmwareBuild
