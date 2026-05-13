#include "ClothSolver.h"

#include <algorithm>
#include <cmath>

ClothSolver::ClothSolver()
{
  constexpr int gridSize = 40;
  constexpr float edgeLen = 2.0f / static_cast<float>(gridSize);

  for (int i = 0; i <= gridSize; i++)
  {
    for (int j = 0; j <= gridSize; j++)
    {
      this->positions.push_back(-1.0f + i * edgeLen);
      this->positions.push_back(-1.0f + j * edgeLen);
      this->positions.push_back(5.0f);
      this->normals.push_back(0.0f);
      this->normals.push_back(0.0f);
      this->normals.push_back(1.0f);
      this->tcoords.push_back(static_cast<float>(i) / gridSize);
      this->tcoords.push_back(static_cast<float>(j) / gridSize);

      this->inversed_masses.push_back(1.0f);

      if ((i == gridSize) && (j == 0 || j == gridSize)) // pin the top left and top right corners
      {
        this->inversed_masses.back() = 0.0f;
      }
    }
  }

  this->next_positions = this->positions;
  this->velocities.resize(this->positions.size(), 0.0f);

  for (int i = 0; i < gridSize; i++)
  {
    for (int j = 0; j < gridSize; j++)
    {
      unsigned int topLeft = (i + 1) * (gridSize + 1) + j;
      unsigned int topRight = topLeft + 1;
      unsigned int bottomLeft = i * (gridSize + 1) + j;
      unsigned int bottomRight = bottomLeft + 1;
      this->face_indices.push_back(topRight);
      this->face_indices.push_back(bottomRight);
      this->face_indices.push_back(bottomLeft);
      this->face_indices.push_back(topLeft);

      this->distance_constraints.push_back({ .p1 = topLeft, .p2 = topRight, .rest_length = edgeLen });
      this->distance_constraints.push_back({ .p1 = topLeft, .p2 = bottomLeft, .rest_length = edgeLen });
      this->distance_constraints.push_back({ .p1 = topRight, .p2 = bottomRight, .rest_length = edgeLen });
      this->distance_constraints.push_back({ .p1 = bottomLeft, .p2 = bottomRight, .rest_length = edgeLen });
    }
  }

  this->face_offsets.resize(gridSize * gridSize + 1);
  std::generate(this->face_offsets.begin(), this->face_offsets.end(), [n = 0]() mutable {
      const unsigned int offset = n;
      n += 4;
      return offset;
    });
}

void ClothSolver::update(double newTime)
{
  double timeStep = newTime - this->currentTime;

  if (timeStep <= 0.0)
  {
    return; // no update needed
  }

  this->currentTime = newTime;

  // Apply gravity on Z axis (m.s^-2)
  constexpr float gravity = -9.81f;

  for (size_t i = 0; i < this->positions.size(); i++)
  {
    if (i % 3 == 2) // Z component
    {
      this->velocities[i] += gravity * this->inversed_masses[i / 3] * static_cast<float>(timeStep);
    }
    this->next_positions[i] = this->positions[i] + this->velocities[i] * static_cast<float>(timeStep);
  }

  // loop on constraints
  for (int iter = 0; iter < 200; iter++) // iterate a few times for better convergence
  {
    for (const DistanceConstraint& constraint : this->distance_constraints)
    {
      unsigned int i1 = constraint.p1 * 3;
      unsigned int i2 = constraint.p2 * 3;
      float dx = this->next_positions[i1] - this->next_positions[i2];
      float dy = this->next_positions[i1 + 1] - this->next_positions[i2 + 1];
      float dz = this->next_positions[i1 + 2] - this->next_positions[i2 + 2];
      float len = std::sqrt(dx * dx + dy * dy + dz * dz);
      float diff = (len - constraint.rest_length) / len;
      float invMass1 = this->inversed_masses[constraint.p1];
      float invMass2 = this->inversed_masses[constraint.p2];
      float sumInvMass = invMass1 + invMass2;
      if (sumInvMass > 0.0f)
      {
        float correction1 = (invMass1 / sumInvMass) * diff;
        float correction2 = (invMass2 / sumInvMass) * diff;
        this->next_positions[i1] -= correction1 * dx;
        this->next_positions[i1 + 1] -= correction1 * dy;
        this->next_positions[i1 + 2] -= correction1 * dz;
        this->next_positions[i2] += correction2 * dx;
        this->next_positions[i2 + 1] += correction2 * dy;
        this->next_positions[i2 + 2] += correction2 * dz;
      }
    }
  }

  // update velocities and positions
  for (size_t i = 0; i < this->positions.size(); i++)
  {
    this->velocities[i] = (this->next_positions[i] - this->positions[i]) / static_cast<float>(timeStep);
    this->positions[i] = this->next_positions[i];
  }
}
