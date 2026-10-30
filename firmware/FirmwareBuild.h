#pragma once
// Human-visible identity of this *specific* compiled source image.
// Update this string whenever test behavior changes, including a safety fix.
// Do not derive it from __DATE__ (two different builds may look identical).
namespace FirmwareBuild {
constexpr char kId[] = "HOT-261030-SANWA";
constexpr char kShortId[] = "H30-SNW";
constexpr char kMode[] = "ATTENDED AIR55 SANWA CUE";
constexpr unsigned long kSplashExtraMs = 3000UL;
}  // namespace FirmwareBuild
