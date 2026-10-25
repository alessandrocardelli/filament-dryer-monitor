#pragma once
// Human-visible identity of this *specific* compiled source image.
// Update this string whenever test behavior changes, including a safety fix.
// Do not derive it from __DATE__ (two different builds may look identical).
namespace FirmwareBuild {
constexpr char kId[] = "HOT-251025-S6C";
constexpr char kShortId[] = "H25-S6C";
constexpr char kMode[] = "SUPERVISED 6x20s F100";
constexpr unsigned long kSplashExtraMs = 3000UL;
}  // namespace FirmwareBuild
