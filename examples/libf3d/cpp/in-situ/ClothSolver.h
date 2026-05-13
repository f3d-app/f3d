#include <cstdint>
#include <vector>

class ClothSolver
{
public:
  ClothSolver();

  void update(double timeStep);

  // accessors for visualization
  size_t getPointCount() const { return this->positions.size() / 3; }
  const float* getPositions() const { return this->positions.data(); }
  const float* getNormals() const { return this->normals.data(); }
  const float* getTextureCoordinates() const { return this->tcoords.data(); }
  const float* getInversedMasses() const { return this->inversed_masses.data(); }
  const float* getVelocities() const { return this->velocities.data(); }

  size_t getFaceOffsetCount() const { return this->face_offsets.size(); }
  size_t getFaceIndexCount() const { return this->face_indices.size(); }
  const uint32_t* getFaceOffsets() const { return this->face_offsets.data(); }
  const uint32_t* getFaceIndices() const { return this->face_indices.data(); }

private:
  double currentTime = 0.0;

  std::vector<float> positions;

  // used for rendering only
  std::vector<float> normals;
  std::vector<float> tcoords;

  // topology
  std::vector<uint32_t> face_offsets;
  std::vector<uint32_t> face_indices;

  // simulation data on particles
  std::vector<float> inversed_masses;
  std::vector<float> next_positions;
  std::vector<float> velocities;

  // constraints data
  struct DistanceConstraint
  {
    uint32_t p1;
    uint32_t p2;
    float rest_length;
  };

  std::vector<DistanceConstraint> distance_constraints;
};
