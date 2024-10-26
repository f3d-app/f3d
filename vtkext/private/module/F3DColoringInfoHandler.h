/**
 * @class F3DColoringInfoHandler
 * @brief A stateful handler to handle coloring info
 */
#ifndef F3DColoringInfoHandler_h
#define F3DColoringInfoHandler_h

#include <array>
#include <limits>
#include <map>
#include <optional>
#include <string>
#include <vector>

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

  /**
   * Update internal coloring maps using provided dataset
   * useCellData control if point data or cell data should be updated
   */
  void UpdateColoringInfo(vtkDataSet* dataset, bool useCellData);

  /**
   * Clear all internal coloring maps
   */
  void ClearColoringInfo();

  /**
   * Set the current coloring state
   * @param enable: If coloring should be enabled or not
   * @param useCellData: If cell data or point data should be used
   * @param arrayName: An optional arrayName to color with
   * @param quiet: If true, no log will be done by this method, even when failing to find an array to color with
   * @param info: Output param containing current ColoringInfo
   * @return: true if the provided arguments resulted in coloring, false otherwise
   */
  bool SetCurrentColoring(bool enable, bool useCellData, std::optional<std::string> arrayName, bool quiet, ColoringInfo& info);

  /**
   * Get the current coloring state
   * Return true if coloring, false otherwise
   * When returning true, info contain the current ColoringInfo
   */
  bool GetCurrentColoring(ColoringInfo& info);

  /**
   * Cycle the current coloring
   * If not coloring, this will try to find an array to color with
   * This does not change the cell/point data status
   * @param cycleToNonColoring: Control wether to cycle to non coloring after reaching the last array or not
   */
  void CycleColoringArray(bool cycleToNonColoring);

private:
  // Map of arrayName -> coloring info
  using ColoringMap = std::map<std::string, ColoringInfo>;
  ColoringMap PointDataColoringInfo;
  ColoringMap CellDataColoringInfo;

  // Current coloring state
  bool CurrentUsingCellData = false;
  bool Coloring = false;
  ColoringMap::iterator CurrentColoringIter;
};

#endif
