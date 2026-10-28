#pragma once
// Human-visible identity of this *specific* compiled source image.
// Update this string whenever test behavior changes, including a safety fix.
// Do not derive it from __DATE__ (two different builds may look identical).
namespace FirmwareBuild {
constexpr char kId[] = "HOT-261028-S10A25";
constexpr char kShortId[] = "H28-S10A";
constexpr char kMode[] = "S10A 25C NTC45K";
constexpr unsigned long kSplashExtraMs = 3000UL;
}  // namespace FirmwareBuild
