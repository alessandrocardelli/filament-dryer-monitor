#pragma once
// Human-visible identity of this *specific* compiled source image.
// Update this string whenever test behavior changes, including a safety fix.
// Do not derive it from __DATE__ (two different builds may look identical).
namespace FirmwareBuild {
constexpr char kId[] = "HOT-261027-S9B180";
constexpr char kShortId[] = "H27-B180";
constexpr char kMode[] = "S9B 180S AIR25";
constexpr unsigned long kSplashExtraMs = 3000UL;
}  // namespace FirmwareBuild
