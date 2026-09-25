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
    bool IsCellData = false;
    std::vector<std::string> ComponentNames;
    std::vector<std::array<double, 2>> ComponentRanges;
    std::array<double, 2> MagnitudeRange = { std::numeric_limits<float>::max(),
      std::numeric_limits<float>::min() };
  };

  /**
   * Update internal coloring maps using provided dataset
   * useCellData control if point data or cell data should be updated
   */
  void UpdateColoringInfo(vtkDataSet* dataset);

  /**
   * Clear all internal coloring maps
   */
  void ClearColoringInfo();

  /**
   * Set the current coloring state
   * @param forceUseCellData: Only cell data should be used, point data will be ignored if true
   * @param arrayName: An optional arrayName to color with
   * @param quiet: If true, no log will be done by this method, even when failing to find an array
   * to color with
   * @return: current coloring info if any, unset optional otherwise
   */
  std::optional<ColoringInfo> SetCurrentColoring(
    bool forceUseCellData, const std::optional<std::string>& arrayName, bool quiet);

  /**
   * Get the current coloring state
   * Return current coloring info if any, unset optional otherwise
   */
  std::optional<ColoringInfo> GetCurrentColoringInfo() const;

  /**
   * Cycle the current coloring
   */
  void CycleColoringArray();

private:
  // Map of { arrayName, type } -> coloring info
  using ColoringMapKeyType = std::pair<std::string, bool>;
  using ColoringMapType = std::map<ColoringMapKeyType, ColoringInfo>;
  ColoringMapType ColoringInfoMap;

  // Current coloring state
  std::optional<ColoringMapType::const_iterator> CurrentColoringIter;
};

#endif
