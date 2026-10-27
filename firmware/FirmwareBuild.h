#pragma once
// Human-visible identity of this *specific* compiled source image.
// Update this string whenever test behavior changes, including a safety fix.
// Do not derive it from __DATE__ (two different builds may look identical).
namespace FirmwareBuild {
constexpr char kId[] = "HOT-261027-S9B";
constexpr char kShortId[] = "H27-S9B";
constexpr char kMode[] = "S09B AIR FEEDBACK";
constexpr unsigned long kSplashExtraMs = 3000UL;
}  // namespace FirmwareBuild
