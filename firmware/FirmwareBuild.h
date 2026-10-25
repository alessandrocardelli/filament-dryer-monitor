#pragma once
// Human-visible identity of this *specific* compiled source image.
// Update this string whenever test behavior changes, including a safety fix.
// Do not derive it from __DATE__ (two different builds may look identical).
namespace FirmwareBuild {
constexpr char kId[] = "COLD-251025-S6";
constexpr char kShortId[] = "C25-S6";
constexpr char kMode[] = "DRY RUN / WEB OTA";
constexpr unsigned long kSplashExtraMs = 3000UL;
}  // namespace FirmwareBuild
