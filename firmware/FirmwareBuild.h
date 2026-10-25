#pragma once
// Human-visible identity of this *specific* compiled source image.
// Update this string whenever test behavior changes, including a safety fix.
// Do not derive it from __DATE__ (two different builds may look identical).
namespace FirmwareBuild {
constexpr char kId[] = "HOT-251025-S7G";
constexpr char kShortId[] = "H25-S7G";
constexpr char kMode[] = "NTC GUARD 40K F100";
constexpr unsigned long kSplashExtraMs = 3000UL;
}  // namespace FirmwareBuild
