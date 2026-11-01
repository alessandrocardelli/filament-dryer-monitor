#pragma once
#include <cstdint>

// Optional suggestions for a user-selected cycle, NOT heater safety limits.
// Values from the 31 Oct 2026 Work study: PLA (Prusament) 45C/6h,
// PLA+ (eSUN) 50C/8h, PVB (Prusament) 45C/8h.
// They are brand-specific examples, NOT universal material specifications.
// These values never enable heater output, and RH never ends a cycle.
enum class MaterialProfile : uint8_t { Manual=0, PLA=1, PLAPlus=2, PVB=3 };
namespace MaterialPresets {
struct Recommendation {int16_t airTargetC;uint16_t durationMinutes;};
constexpr bool valid(uint8_t x){return x<=3;}
constexpr MaterialProfile fromStored(uint8_t x){
  return valid(x)?static_cast<MaterialProfile>(x):MaterialProfile::Manual;
}
constexpr MaterialProfile next(MaterialProfile profile,int8_t step){
  int x=static_cast<int>(profile);
  if(x<0||x>3)x=0;
  return static_cast<MaterialProfile>((x+(step<0?3:1))%4);
}
constexpr const char *label(MaterialProfile p){
  switch(p){
    case MaterialProfile::PLA:return "PLA";
    case MaterialProfile::PLAPlus:return "PLA+";
    case MaterialProfile::PVB:return "PVB";
    default:return "MANUALE";
  }
}
constexpr Recommendation suggestion(MaterialProfile p){
  switch(p){
    case MaterialProfile::PLA:return {45,360};
    case MaterialProfile::PLAPlus:return {50,480};
    case MaterialProfile::PVB:return {45,480};
    default:return {0,0}; // manual preserves existing temperature and time
  }
}
} // namespace MaterialPresets
