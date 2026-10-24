#pragma once
// Human-visible identity of this *specific* compiled source image.
// Update this string whenever test behavior changes, including a safety fix.
// Do not derive it from __DATE__ (two different builds may look identical).
namespace FirmwareBuild {
constexpr char kId[] = "PULSE-241024-R1";
constexpr char kShortId[] = "P24-R1";
constexpr char kMode[] = "3 x 1.5s TEST";
constexpr unsigned long kSplashExtraMs = 3000UL;
}  // namespace FirmwareBuild
