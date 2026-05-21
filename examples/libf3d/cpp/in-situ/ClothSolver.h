#include <cstdint>
#include <vector>

/**
 * Simple cloth solver based on the PBD method.
 */
class ClothSolver
{
public:
  ClothSolver();

  /**
   * Set the grid size
   */
  void setGridSize(uint32_t gridSize)
  {
    this->GridSize = gridSize;
  }

  /**
   * Number of PBD iterations during a solver step
   */
  void setIterations(uint32_t iterations)
  {
    this->Iterations = iterations;
  }

  /**
   * Initialize and allocate cloth state
   */
  void initialize();

  /**
   * Advance simulation by `timeStep` seconds
   */
  void update(double timeStep);

  //@{
  /**
   * Memory accessors
   */
  size_t getPointCount() const
  {
    return this->Positions.size() / 3;
  }
  const float* getPositions() const
  {
    return this->Positions.data();
  }
  const float* getInversedMasses() const
  {
    return this->InversedMasses.data();
  }
  const float* getVelocities() const
  {
    return this->Velocities.data();
  }

  size_t getFaceOffsetCount() const
  {
    return this->FaceOffsets.size();
  }
  size_t getFaceIndexCount() const
  {
    return this->FaceIndices.size();
  }
  const uint32_t* getFaceOffsets() const
  {
    return this->FaceOffsets.data();
  }
  const uint32_t* getFaceIndices() const
  {
    return this->FaceIndices.data();
  }

  size_t getFixedVertexOffsetCount() const
  {
    return this->FixedVerticesOffsets.size();
  }
  size_t getFixedVertexIndexCount() const
  {
    return this->FixedVerticesIndices.size();
  }
  const uint32_t* getFixedVertexOffsets() const
  {
    return this->FixedVerticesOffsets.data();
  }
  const uint32_t* getFixedVertexIndices() const
  {
    return this->FixedVerticesIndices.data();
  }
  //@}

private:
  double CurrentTime = 0.0;
  uint32_t GridSize = 40;
  uint32_t Iterations = 100;

  std::vector<float> Positions;

  // topology
  std::vector<uint32_t> FaceOffsets;
  std::vector<uint32_t> FaceIndices;

  // simulation data on particles
  std::vector<float> InversedMasses;
  std::vector<float> NextPositions;
  std::vector<float> Velocities;

  // fixed vertices
  std::vector<uint32_t> FixedVerticesIndices;
  std::vector<uint32_t> FixedVerticesOffsets;

  // constraints data
  struct DistanceConstraint
  {
    uint32_t Indices[2];
    float RestLength;
  };

  std::vector<DistanceConstraint> DistanceConstraints;
};
