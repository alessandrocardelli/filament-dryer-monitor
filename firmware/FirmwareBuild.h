#pragma once
// Human-visible identity of this *specific* compiled source image.
// Update this string whenever test behavior changes, including a safety fix.
// Do not derive it from __DATE__ (two different builds may look identical).
namespace FirmwareBuild {
constexpr char kId[] = "HOT-261029-S12P25";
constexpr char kShortId[] = "H29-S12P";
constexpr char kMode[] = "S12P PLATE TRIAL";
constexpr unsigned long kSplashExtraMs = 3000UL;
}  // namespace FirmwareBuild
