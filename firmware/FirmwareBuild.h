#pragma once
// Human-visible identity of this *specific* compiled source image.
// Update this string whenever test behavior changes, including a safety fix.
// Do not derive it from __DATE__ (two different builds may look identical).
namespace FirmwareBuild {
constexpr char kId[] = "HOT-261031-N8580";
constexpr char kShortId[] = "H31-8580";
constexpr char kMode[] = "ATTENDED NTC85/80 AIR THERMOSTAT";
constexpr unsigned long kSplashExtraMs = 3000UL;
}  // namespace FirmwareBuild
