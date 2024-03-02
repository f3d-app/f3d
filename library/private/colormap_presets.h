#ifndef f3d_colormap_presets_h
#define f3d_colormap_presets_h

#include <string>
#include <unordered_map>
#include <vector>

namespace f3d::detail
{
extern const std::unordered_map<std::string, std::vector<double> > ColorMapPresets;
}
#endif
