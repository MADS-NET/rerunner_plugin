#include "skeleton.hpp"
#include <iostream>
#include <cmath>

using namespace std;

int main(int argc, const char **argv) {
  vector<double> v = {1, 1, 1, 0, 0, 0};
  for (int i = 1; i < argc && i < v.size(); i++) {
    v[i-1] = atof(argv[i]);
  }
  // Eigen::Matrix3d D = Eigen::Vector3d(v[0], v[1], v[2]).asDiagonal();
  // Eigen::AngleAxisd rx(v[5] / 360.0 * M_PI, Eigen::Vector3d::UnitX());
  // Eigen::AngleAxisd ry(v[4] / 360.0 * M_PI, Eigen::Vector3d::UnitY());
  // Eigen::AngleAxisd rz(v[3] / 360.0 * M_PI, Eigen::Vector3d::UnitZ());
  // cout << "roll: " << rz.angle() << endl;
  // cout << "pitch: " << ry.angle() << endl;
  // cout << "yaw: " << rx.angle() << endl;
  // auto rotmat = (rz * ry * rx).toRotationMatrix();
  // cout << "Rotation matrix: " << endl << rotmat << endl;

  // Eigen::Matrix3d result = rotmat * D;
  // cout << "Rotated matrix:\n" << result << endl;


  Ellipsoid3D e = Ellipsoid3D(v);
  cout << "Covariance matrix: \n" << e.covariance << endl;
  cout << "-------------------------------------------" << endl;
  cout << "Rotation matrix: \n" << e.rotation << endl;
  cout << "-------------------------------------------" << endl;
  cout << "Semi-axes:  " << e.semi_axes.transpose() << endl;
  cout << "Quaternion: " << e.quaternion << endl;
  cout << "Angle: " << acos(e.quaternion.w()) * 2 / M_PI * 180 << "°" << endl;

  return 0;
}