/**
 * @class F3DColoringInfoHelper
 * @brief A statefull helper to handle coloring info
 */
#ifndef F3DColoringInfoHelper_h
#define F3DColoringInfoHelper_h

#include <map>
#include <string>
#include <vector>
#include <array>
#include <limits>

class vtkDataSet;
class F3DColoringInfoHelper
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
    int Index = -1; // TODO remove
  };


  void UpdateColoringInfo(vtkDataSet* dataset, bool useCellData);
  void FinalizeColoringInfo(bool useCellData);
  void ClearColoringInfo();

  /**
   * Recover information about coloring by index
   * Should be called after actors have been imported
   */
  bool GetInfoForColoring(bool useCellData, int index, ColoringInfo& info);

  /**
   * Get the maximum index possible for coloring
   * Should be called after actors have been imported
   */
  int GetNumberOfIndexesForColoring(bool useCellData);

  /**
   * Find an index for coloring corresponding to provided arrayName if available
   * Should be called after actors have been imported
   */
  int FindIndexForColoring(bool useCellData, const std::string& arrayName);

private:
  // Map of arrayName -> coloring info
  std::map<std::string, ColoringInfo> PointDataColoringInfo;
  std::map<std::string, ColoringInfo> CellDataColoringInfo;
  std::vector<std::string> PointDataArrayNames;
  std::vector<std::string> CellDataArrayNames;
};

#endif
