/**
 * @class F3DColoringInfoHandler
 * @brief A statefull Handler to handle coloring info
 */
#ifndef F3DColoringInfoHandler_h
#define F3DColoringInfoHandler_h

#include <map>
#include <string>
#include <vector>
#include <array>
#include <limits>
#include <optional>

class vtkDataSet;
class F3DColoringInfoHandler
{
public:
  /**
   * A struct containing information about possible coloring
   */
  struct ColoringInfo
  {
    std::string Name;
    int MaximumNumberOfComponents = 0;
    std::vector<std::string> ComponentNames;
    std::vector<std::array<double, 2>> ComponentRanges;
    std::array<double, 2> MagnitudeRange = { std::numeric_limits<float>::max(),
      std::numeric_limits<float>::min() };
  };


  void UpdateColoringInfo(vtkDataSet* dataset, bool useCellData);
  void ClearColoringInfo();

  bool SetCurrentColoring(bool enable, bool useCellData, std::optional<std::string> arrayName, ColoringInfo& info, bool quiet);
  bool GetCurrentColoring(ColoringInfo& info);
  void CycleColoringArray(bool cycleToNonColoring);

private:
  // Map of arrayName -> coloring info
  using ColoringMap = std::map<std::string, ColoringInfo>;
  ColoringMap PointDataColoringInfo;
  ColoringMap CellDataColoringInfo;

  bool Coloring = false;
  ColoringMap::iterator CurrentColoringIter;
  bool CurrentUsingCellData = false;
};

#endif
