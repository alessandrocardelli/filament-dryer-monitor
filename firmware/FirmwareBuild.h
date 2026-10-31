#pragma once
// Human-visible identity of this *specific* compiled source image.
// Update this string whenever test behavior changes, including a safety fix.
// Do not derive it from __DATE__ (two different builds may look identical).
namespace FirmwareBuild {
constexpr char kId[] = "HOT-261031-N9085";
constexpr char kShortId[] = "H31-9085";
constexpr char kMode[] = "ATTENDED NTC90/85 + LIVE WEB LOG";
constexpr unsigned long kSplashExtraMs = 3000UL;
}  // namespace FirmwareBuild
