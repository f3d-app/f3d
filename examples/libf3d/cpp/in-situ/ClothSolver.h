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
    this->gridSize = gridSize;
  }

  /**
   * Number of PBD iterations during a solver step
   */
  void setIterations(uint32_t iterations)
  {
    this->iterations = iterations;
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
    return this->positions.size() / 3;
  }
  const float* getPositions() const
  {
    return this->positions.data();
  }
  const float* getInversedMasses() const
  {
    return this->inversed_masses.data();
  }
  const float* getVelocities() const
  {
    return this->velocities.data();
  }

  size_t getFaceOffsetCount() const
  {
    return this->face_offsets.size();
  }
  size_t getFaceIndexCount() const
  {
    return this->face_indices.size();
  }
  const uint32_t* getFaceOffsets() const
  {
    return this->face_offsets.data();
  }
  const uint32_t* getFaceIndices() const
  {
    return this->face_indices.data();
  }

  size_t getFixedVertexOffsetCount() const
  {
    return this->fixed_vertices_offsets.size();
  }
  size_t getFixedVertexIndexCount() const
  {
    return this->fixed_vertices_indices.size();
  }
  const uint32_t* getFixedVertexOffsets() const
  {
    return this->fixed_vertices_offsets.data();
  }
  const uint32_t* getFixedVertexIndices() const
  {
    return this->fixed_vertices_indices.data();
  }
  //@}

private:
  double currentTime = 0.0;
  uint32_t gridSize = 40;
  uint32_t iterations = 100;

  std::vector<float> positions;

  // topology
  std::vector<uint32_t> face_offsets;
  std::vector<uint32_t> face_indices;

  // simulation data on particles
  std::vector<float> inversed_masses;
  std::vector<float> next_positions;
  std::vector<float> velocities;

  // fixed vertices
  std::vector<uint32_t> fixed_vertices_indices;
  std::vector<uint32_t> fixed_vertices_offsets;

  // constraints data
  struct DistanceConstraint
  {
    uint32_t p1;
    uint32_t p2;
    float rest_length;
  };

  std::vector<DistanceConstraint> distance_constraints;
};
