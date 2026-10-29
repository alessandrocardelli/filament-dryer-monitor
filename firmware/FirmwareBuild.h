#pragma once
// Human-visible identity of this *specific* compiled source image.
// Update this string whenever test behavior changes, including a safety fix.
// Do not derive it from __DATE__ (two different builds may look identical).
namespace FirmwareBuild {
constexpr char kId[] = "HOT-261029-S11D25";
constexpr char kShortId[] = "H29-S11D";
constexpr char kMode[] = "S11D PLATE DIAG";
constexpr unsigned long kSplashExtraMs = 3000UL;
}  // namespace FirmwareBuild
