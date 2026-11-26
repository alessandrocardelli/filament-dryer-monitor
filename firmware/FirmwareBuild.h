#pragma once
// Human-visible identity of this *specific* compiled source image.
// Update this string whenever test behavior changes, including a safety fix.
// Do not derive it from __DATE__ (two different builds may look identical).
namespace FirmwareBuild {
constexpr char kId[] = "HOT-261126-UI5";
constexpr char kShortId[] = "H26-UI5";
constexpr char kMode[] = "VISHAY EXP";
constexpr unsigned long kSplashExtraMs = 3000UL;
}  // namespace FirmwareBuild
