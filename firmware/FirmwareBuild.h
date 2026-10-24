#pragma once
// Human-visible identity of this *specific* compiled source image.
// Update this string whenever test behavior changes, including a safety fix.
// Do not derive it from __DATE__ (two different builds may look identical).
namespace FirmwareBuild {
constexpr char kId[] = "WIFI-241024-R3";
constexpr char kShortId[] = "W24-R3";
constexpr char kMode[] = "IDE + WEB OTA";
constexpr unsigned long kSplashExtraMs = 3000UL;
}  // namespace FirmwareBuild
