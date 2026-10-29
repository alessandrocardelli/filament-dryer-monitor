#pragma once
// Human-visible identity of this *specific* compiled source image.
// Update this string whenever test behavior changes, including a safety fix.
// Do not derive it from __DATE__ (two different builds may look identical).
namespace FirmwareBuild {
constexpr char kId[] = "V1-261029-CTRL55";
constexpr char kShortId[] = "V1-C55";
constexpr char kMode[] = "V1 40-55 COLD GATE";
constexpr unsigned long kSplashExtraMs = 3000UL;
}  // namespace FirmwareBuild
