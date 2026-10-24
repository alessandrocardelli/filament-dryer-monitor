#pragma once
// Human-visible identity of this *specific* compiled source image.
// Update this string whenever test behavior changes, including a safety fix.
// Do not derive it from __DATE__ (two different builds may look identical).
namespace FirmwareBuild {
constexpr char kId[] = "WIFI-241024-R2";
constexpr char kShortId[] = "W24-R2";
constexpr char kMode[] = "WEB OTA + PULSE";
constexpr unsigned long kSplashExtraMs = 3000UL;
}  // namespace FirmwareBuild
