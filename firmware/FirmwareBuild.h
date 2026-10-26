#pragma once
// Human-visible identity of this *specific* compiled source image.
// Update this string whenever test behavior changes, including a safety fix.
// Do not derive it from __DATE__ (two different builds may look identical).
namespace FirmwareBuild {
constexpr char kId[] = "HOT-261026-S7U";
constexpr char kShortId[] = "H26-S7U";
constexpr char kMode[] = "S7G GUARD / CLEAR UI";
constexpr unsigned long kSplashExtraMs = 3000UL;
}  // namespace FirmwareBuild
