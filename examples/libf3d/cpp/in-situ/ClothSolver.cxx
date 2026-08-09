#include "ClothSolver.h"

#include <algorithm>
#include <cmath>

ClothSolver::ClothSolver()
{
  this->initialize();
}

void ClothSolver::initialize()
{
  this->CurrentTime = 0.f;

  const float edgeLen = 2.0f / static_cast<float>(this->GridSize);

  this->Positions.clear();
  this->InversedMasses.clear();
  this->NextPositions.clear();
  this->Velocities.clear();
  this->FaceIndices.clear();
  this->FaceOffsets.clear();
  this->DistanceConstraints.clear();

  // define the grid
  for (uint32_t i = 0; i <= this->GridSize; i++)
  {
    for (uint32_t j = 0; j <= this->GridSize; j++)
    {
      this->Positions.push_back(-1.0f + i * edgeLen);
      this->Positions.push_back(-1.0f + j * edgeLen);
      this->Positions.push_back(3.0f);

      this->InversedMasses.push_back(1.0f);
    }
  }

  this->NextPositions = this->Positions;
  this->Velocities.resize(this->Positions.size(), 0.0f);

  // fixed vertices
  this->FixedVerticesIndices = { 0, this->GridSize };
  this->FixedVerticesOffsets = { 0, 1, 2 };

  for (uint32_t index : this->FixedVerticesIndices)
  {
    this->InversedMasses[index] = 0.0f;
  }

  // add cells and distance constraints
  for (uint32_t i = 0; i < this->GridSize; i++)
  {
    for (uint32_t j = 0; j < this->GridSize; j++)
    {
      uint32_t topLeft = (i + 1) * (this->GridSize + 1) + j;
      uint32_t topRight = topLeft + 1;
      uint32_t bottomLeft = i * (this->GridSize + 1) + j;
      uint32_t bottomRight = bottomLeft + 1;
      this->FaceIndices.push_back(topRight);
      this->FaceIndices.push_back(bottomRight);
      this->FaceIndices.push_back(bottomLeft);
      this->FaceIndices.push_back(topLeft);

      // avoid duplicated constraints by only creating them for the top and left edges of each quad
      this->DistanceConstraints.push_back({ topLeft, topRight, edgeLen });
      this->DistanceConstraints.push_back({ topLeft, bottomLeft, edgeLen });

      // but make sure to create constraints for the right and bottom edges of the last quads
      if (j == this->GridSize - 1)
      {
        this->DistanceConstraints.push_back({ topRight, bottomRight, edgeLen });
      }
      if (i == this->GridSize - 1)
      {
        this->DistanceConstraints.push_back({ bottomLeft, bottomRight, edgeLen });
      }

      // add diagonal constraints for better stability
      this->DistanceConstraints.push_back({ topLeft, bottomRight, std::sqrt(2.0f) * edgeLen });
      this->DistanceConstraints.push_back({ topRight, bottomLeft, std::sqrt(2.0f) * edgeLen });
    }
  }

  this->FaceOffsets.resize(this->GridSize * this->GridSize + 1);
  std::generate(this->FaceOffsets.begin(), this->FaceOffsets.end(),
    [n = 0]() mutable
    {
      const unsigned int offset = n;
      n += 4;
      return offset;
    });
}

void ClothSolver::update(double newTime)
{
  if (newTime == 0.0)
  {
    if (this->CurrentTime != 0.0)
    {
      this->initialize();
    }
    return;
  }

  double timeStep = newTime - this->CurrentTime;

  if (timeStep == 0.0)
  {
    return;
  }

  if (timeStep < 0.0)
  {
    this->initialize();
    return;
  }

  this->CurrentTime = newTime;

  // Apply gravity on Z axis (m.s^-2) and predict next Positions
  constexpr float gravity = -9.81f;

  for (size_t i = 0; i < this->Positions.size(); i++)
  {
    if (i % 3 == 2) // Z component
    {
      this->Velocities[i] += gravity * this->InversedMasses[i / 3] * static_cast<float>(timeStep);
    }
    this->NextPositions[i] =
      this->Positions[i] + this->Velocities[i] * static_cast<float>(timeStep);
  }

  // loop on constraints and project
  for (uint32_t iter = 0; iter < this->Iterations; iter++)
  {
    for (const DistanceConstraint& constraint : this->DistanceConstraints)
    {
      uint32_t i1 = constraint.Indices[0] * 3;
      uint32_t i2 = constraint.Indices[1] * 3;
      float dx = this->NextPositions[i2] - this->NextPositions[i1];
      float dy = this->NextPositions[i2 + 1] - this->NextPositions[i1 + 1];
      float dz = this->NextPositions[i2 + 2] - this->NextPositions[i1 + 2];
      float len = std::sqrt(dx * dx + dy * dy + dz * dz);
      float diff = (len - constraint.RestLength) / len;
      float invMass1 = this->InversedMasses[constraint.Indices[0]];
      float invMass2 = this->InversedMasses[constraint.Indices[1]];
      float sumInvMass = invMass1 + invMass2;
      if (sumInvMass > 0.0f)
      {
        float correction1 = (invMass1 / sumInvMass) * diff;
        float correction2 = (invMass2 / sumInvMass) * diff;
        this->NextPositions[i1] += correction1 * dx;
        this->NextPositions[i1 + 1] += correction1 * dy;
        this->NextPositions[i1 + 2] += correction1 * dz;
        this->NextPositions[i2] -= correction2 * dx;
        this->NextPositions[i2 + 1] -= correction2 * dy;
        this->NextPositions[i2 + 2] -= correction2 * dz;
      }
    }
  }

  // update Velocities and Positions
  for (size_t i = 0; i < this->Positions.size(); i++)
  {
    this->Velocities[i] =
      (this->NextPositions[i] - this->Positions[i]) / static_cast<float>(timeStep);
    this->Positions[i] = this->NextPositions[i];
  }
}
