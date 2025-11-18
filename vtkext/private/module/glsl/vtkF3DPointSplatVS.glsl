//VTK::System::Dec

// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause

uniform float scaleFactor;
uniform int cameraParallel;

// low-pass filtering matrix
// stored as a vector since it's a 2x2 symmetric matrix
uniform vec3 lowpassMatrix;

in vec4 vertexMC;

//VTK::Covariance::Dec

//VTK::PositionVC::Dec

// optional normal declaration
//VTK::Normal::Dec

// Texture coordinates
//VTK::TCoord::Dec

// material property values
//VTK::Color::Dec

// clipping plane vars
//VTK::Clip::Dec

// camera and actor matrix values
//VTK::Camera::Dec

// picking support
//VTK::Picking::Dec

mat3 quaternionToMatrix(vec4 quat)
{
  // Normalize quaternion (required?)
  vec4 q = quat / length(quat);
  float r = q.x;
  float x = q.y;
  float y = q.z;
  float z = q.w;

  return mat3(1. - 2. * (y * y + z * z), 2. * (x * y + r * z), 2. * (x * z - r * y),
    2. * (x * y - r * z), 1. - 2. * (x * x + z * z), 2. * (y * z + r * x),
    2. * (x * z + r * y), 2. * (y * z - r * x), 1. - 2. * (x * x + y * y));
}

mat3 computeCov3D(vec3 scale, vec4 rotation)
{
  // Create scaling matrix
  mat3 S = mat3(
    scaleFactor * scale.x, 0., 0.,
    0., scaleFactor * scale.y, 0.,
    0., 0., scaleFactor * scale.z);

  // Compute rotation matrix from quaternion
  mat3 R = quaternionToMatrix(rotation);

  mat3 M = R * S;

  // Compute 3D world covariance matrix
  return M * transpose(M);
}

mat3 getProjectionMatrix(vec3 positionVC)
{
  // orthographic projection
  if (cameraParallel != 0)
  {
    return mat3(
      VCDCMatrix[0][0], 0., 0.,
      0., VCDCMatrix[1][1], 0.,
      0., 0., 0.);
  }

  // affine approximation of the perspective projection
  // Refer to eq 29 and 31 in "EWA Splatting" (Zwicker et al., 2002).
  return mat3(
    VCDCMatrix[0][0] / positionVC.z, 0., 0.,
    0., VCDCMatrix[1][1] / positionVC.z, 0.,
    -(VCDCMatrix[0][0] * positionVC.x) / (positionVC.z * positionVC.z), -(VCDCMatrix[1][1] * positionVC.y) / (positionVC.z * positionVC.z), 0.);
}

void main()
{
  //VTK::Color::Impl

  //VTK::Normal::Impl

  //VTK::TCoord::Impl

  //VTK::Clip::Impl

  vec4 posVC = MCVCMatrix * vertexMC;

  mat3 J = getProjectionMatrix(posVC.xyz);

  mat3 W = mat3(MCVCMatrix);

  mat3 T = J * W;

  //VTK::Covariance::Impl

  // discard third column/row and store the matrix in a vector
  mat2 cov2d = mat2(cov);

  // It is possible to apply a convolution here by adding a custom 2D covariance matrix
  // For example, it can be useful for making sure the point is at least 1 pixel wide
  cov2d += mat2(lowpassMatrix.x, lowpassMatrix.y, lowpassMatrix.y, lowpassMatrix.z);

  mat2 transform;

  // eigen vectors of the covariance matrix
  if (abs(cov2d[0][1]) > 1e-6)
  {
    // compute basis transformation based on eigen decomposition of the 2D covariance
    // half of the covariance matrix trace
    float halfTrace = 0.5 * (cov2d[0][0] + cov2d[1][1]);

    // eigen values of the covariance matrix
    float term = sqrt(halfTrace * halfTrace - determinant(cov2d));
    float eigenValue1 = halfTrace + term;
    float eigenValue2 = halfTrace - term;

    vec2 eigenVector1 = normalize(vec2(eigenValue1 - cov2d[1][1], cov2d[0][1]));
    vec2 eigenVector2 = vec2(eigenVector1.y, -eigenVector1.x);

    transform = mat2(sqrt(eigenValue1) * eigenVector1, sqrt(eigenValue2) * eigenVector2);
  }
  else
  {
    // the covariance is a diagonal matrix, so axis aligned
    // we just need to extract the scaling matrix (the square root of the covariance in this case)
    transform = mat2(sqrt(cov2d[0][0]), 0.0, 0.0, sqrt(cov2d[1][1]));
  }

  // filter out points outside of the frustum inflated by 30% because the approximation may stretch them too much
  // see https://github.com/graphdeco-inria/gaussian-splatting/issues/191
  // this is a hand-tuned parameter but it should work on most cases (if not, it may be useful to expose the value as uniform)
  vec4 posDC = VCDCMatrix * posVC;

  if (posDC.x < -1.3*posDC.w || posDC.x > 1.3*posDC.w || posDC.y < -1.3*posDC.w || posDC.y > 1.3*posDC.w)
  {
    transform = mat2(0);
  }

  //VTK::PositionVC::Impl

  //VTK::Picking::Impl
}
