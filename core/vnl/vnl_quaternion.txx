// This is core/vnl/vnl_quaternion.txx
#ifndef vnl_quaternion_txx_
#define vnl_quaternion_txx_
//:
// \file
//
// Copyright (C) 1992 General Electric Company.
//
// Permission is granted to any individual or institution to use, copy, modify,
// and distribute this software, provided that this complete copyright and
// permission notice is maintained, intact, in all copies and supporting
// documentation.
//
// General Electric Company,
// provides this software "as is" without express or implied warranty.
//
// Created: VDN 06/23/92  design and implementation
//
// Quaternion IS-A vector, and is a special case of general n-dimensional space.
// The IS-A relationship is enforced with public inheritance.
// All member functions on vectors are applicable to quaternions.
//
// Rep Invariant:
//   -  norm = 1, for a rotation.
//   -  position vector represented by imaginary quaternion.
// References:
// -  Horn, B.K.P. (1987) Closed-form solution of absolute orientation using
//       unit quaternions. J. Opt. Soc. Am. Vol 4, No 4, April.
// -  Horn, B.K.P. (1987) Robot Vision. MIT Press. pp. 437-551.
//


#include "vnl_quaternion.h"

#include <vcl_cmath.h>
#include <vcl_limits.h>
#include <vcl_iostream.h>

#include <vnl/vnl_cross.h>
#include <vnl/vnl_math.h>

//: Creates a quaternion from its ordered components.
// x, y, z denote the imaginary part, which are the  coordinates
// of the rotation axis multiplied by the sine of half the
// angle of rotation. r denotes  the  real  part,  or  the
// cosine  of  half  the  angle of rotation. Default is to
// create a null quaternion, corresponding to a null rotation
// or  an  identity  transform,  which has undefined
// rotation axis.

template <class T>
vnl_quaternion<T>::vnl_quaternion (T x, T y, T z, T r)
{
  this->operator[](0) = x;  // 3 first elements are
  this->operator[](1) = y;  // imaginary parts
  this->operator[](2) = z;
  this->operator[](3) = r;  // last element is real part
}

//: Creates a quaternion from the normalized axis direction and the angle of rotation in radians.

template <class T>
vnl_quaternion<T>::vnl_quaternion(vnl_vector_fixed<T,3> const& axis, double angle)
{
  double a = angle * 0.5;  // half angle
  T s = T(vcl_sin(a));
  for (int i = 0; i < 3; i++)            // imaginary vector is sine of
    this->operator[](i) = T(s * axis(i));// half angle multiplied with axis
  this->operator[](3) = T(vcl_cos(a));   // real part is cosine of half angle
}

//: Creates a quaternion from a vector.
// 3D vector is converted into an imaginary quaternion with same
// (x, y, z) components.

template <class T>
vnl_quaternion<T>::vnl_quaternion(vnl_vector_fixed<T,3> const& vec)
{
  for (unsigned int i = 0; i < 3; ++i)
    this->operator[](i) = vec(i);
  this->operator[](3) = T(0);
}

//: Creates a quaternion from a vector.
// 4D vector is assumed to be a 4-element quaternion, to
// provide casting between vector and quaternion

template <class T>
vnl_quaternion<T>::vnl_quaternion(vnl_vector_fixed<T,4> const& vec)
{
  for (unsigned int i = 0; i < 4; ++i) // 1-1 layout between vector & quaternion
    this->operator[](i) = vec[i];
}


//: Creates a quaternion from a rotation matrix.
// Its orthonormal basis vectors are row-wise.
// WARNING: Takes the transpose of the rotation matrix...
template <class T>
vnl_quaternion<T>::vnl_quaternion(vnl_matrix_fixed<T,3,3> const& rot)
{
  double d0 = rot(0,0), d1 = rot(1,1), d2 = rot(2,2);
  double xx = 1.0 + d0 - d1 - d2;               // from the diagonal of rotation
  double yy = 1.0 - d0 + d1 - d2;               // matrix, find the terms in
  double zz = 1.0 - d0 - d1 + d2;               // each Quaternion component
  double rr = 1.0 + d0 + d1 + d2;

  double max = rr;                              // find the maximum of all
  if (xx > max) max = xx;                       // diagonal terms.
  if (yy > max) max = yy;
  if (zz > max) max = zz;

  if (rr == max) {
    T r4 = T(vcl_sqrt(rr * 4.0));
    this->x() = (rot(1,2) - rot(2,1)) / r4;     // find other components from
    this->y() = (rot(2,0) - rot(0,2)) / r4;     // off diagonal terms of
    this->z() = (rot(0,1) - rot(1,0)) / r4;     // rotation matrix.
    this->r() = r4 / 4;
  } else if (xx == max) {
    T x4 = T(vcl_sqrt(xx * 4.0));
    this->x() = x4 / 4;
    this->y() = (rot(0,1) + rot(1,0)) / x4;
    this->z() = (rot(0,2) + rot(2,0)) / x4;
    this->r() = (rot(1,2) - rot(2,1)) / x4;
  } else if (yy == max) {
    T y4 = T(vcl_sqrt(yy * 4.0));
    this->x() = (rot(0,1) + rot(1,0)) / y4;
    this->y() =  y4 / 4;
    this->z() = (rot(1,2) + rot(2,1)) / y4;
    this->r() = (rot(2,0) - rot(0,2)) / y4;
  } else {
    T z4 = T(vcl_sqrt(zz * 4.0));
    this->x() = (rot(0,2) + rot(2,0)) / z4;
    this->y() = (rot(1,2) + rot(2,1)) / z4;
    this->z() =  z4 / 4;
    this->r() = (rot(0,1) - rot(1,0)) / z4;
  }
}


//: Construct quaternion from Euler Angles
// That is a rotation about the X axis, followed by Y, followed by
// the Z axis, using a fixed reference frame.
template <class T>
vnl_quaternion<T>::vnl_quaternion(T theta_X, T theta_Y, T theta_Z)
{
  vnl_quaternion<T> Rx(T(vcl_sin(double(theta_X)*0.5)), 0, 0, T(vcl_cos(double(theta_X)*0.5)));
  vnl_quaternion<T> Ry(0, T(vcl_sin(double(theta_Y)*0.5)), 0, T(vcl_cos(double(theta_Y)*0.5)));
  vnl_quaternion<T> Rz(0, 0, T(vcl_sin(double(theta_Z)*0.5)), T(vcl_cos(double(theta_Z)*0.5)));
  *this = Rz * Ry * Rx;
}

//: Rotation representation in Euler angles.
// The angles raturned will be [theta_X,theta_Y,theta_Z]
// where the final rotation is found be first applying theta_X radians
// about the X axis, then theta_Y about the Y-axis, etc.
// The axes stay in a fixed reference frame.
template <class T>
vnl_vector_fixed<T,3> vnl_quaternion<T>::rotation_euler_angles() const
{
  vnl_vector_fixed<T,3> angles;

  vnl_matrix_fixed<T,4,4> rotM = rotation_matrix_transpose_4();
  T xy = T(vcl_sqrt(double(vnl_math_sqr(rotM(0,0)) + vnl_math_sqr(rotM(0,1)))));
  if (xy > vcl_numeric_limits<T>::epsilon() * T(8))
  {
    angles(0) = T(vcl_atan2(double(rotM(1,2)), double(rotM(2,2))));
    angles(1) = T(vcl_atan2(double(-rotM(0,2)), double(xy)));
    angles(2) = T(vcl_atan2(double(rotM(0,1)), double(rotM(0,0))));
  }
  else
  {
    angles(0) = T(vcl_atan2(double(-rotM(2,1)), double(rotM(1,1))));
    angles(1) = T(vcl_atan2(double(-rotM(0,2)), double(xy)));
    angles(2) = T(0);
  }
  return angles;
}


//: Queries the rotation angle of the quaternion.
//  Returned angle lies in [0, 2*pi]
template <class T>
double vnl_quaternion<T>::angle() const
{
  return 2 * vcl_atan2(double(this->imaginary().magnitude()),
                       double(this->real()));    // angle is always positive
}

//: Queries the direction of the rotation axis of the quaternion.
//  A null quaternion will return zero for angle and k direction for axis.
template <class T>
vnl_vector_fixed<T,3> vnl_quaternion<T>::axis() const
{
  vnl_vector_fixed<T,3> direc = this->imaginary(); // direc parallel to imag. part
  T mag = direc.magnitude();
  if (mag == T(0)) {
    vcl_cout << "Axis not well defined for zero Quaternion. Using (0,0,1) instead.\n";
    direc[2] = T(1);                    // or signal exception here.
  }
  else
    direc /= mag;                       // normalize direction vector
  return direc;
}


//: Converts a normalized quaternion into a square rotation matrix with dimension dim.
//  This is the reverse counterpart of constructing a quaternion from a transformation matrix.
// WARNING this is inconsistent with the quaternion docs and q.rotate()

template <class T>
vnl_matrix_fixed<T,3,3> vnl_quaternion<T>::rotation_matrix_transpose() const
{
  T x2 = x() * x(),  xy = x() * y(),  rx = r() * x(),
    y2 = y() * y(),  yz = y() * z(),  ry = r() * y(),
    z2 = z() * z(),  zx = z() * x(),  rz = r() * z(),
    r2 = r() * r();
  vnl_matrix_fixed<T,3,3> rot;
  rot(0,0) = r2 + x2 - y2 - z2;         // fill diagonal terms
  rot(1,1) = r2 - x2 + y2 - z2;
  rot(2,2) = r2 - x2 - y2 + z2;
  rot(0,1) = 2 * (xy + rz);             // fill off diagonal terms
  rot(0,2) = 2 * (zx - ry);
  rot(1,2) = 2 * (yz + rx);
  rot(1,0) = 2 * (xy - rz);
  rot(2,0) = 2 * (zx + ry);
  rot(2,1) = 2 * (yz - rx);

  return rot;
}


template <class T>
vnl_matrix_fixed<T,4,4> vnl_quaternion<T>::rotation_matrix_transpose_4() const
{
  vnl_matrix_fixed<T,4,4> rot;
  rot.set_identity();
  rot.update(this->rotation_matrix_transpose().as_ref());
  return rot;
}

//: Returns the conjugate of given quaternion, having same real and opposite imaginary parts.

template <class T>
vnl_quaternion<T> vnl_quaternion<T>::conjugate() const
{
  return vnl_quaternion<T> (-x(), -y(), -z(), r());
}

//: Returns the inverse of given quaternion.
//  For unit quaternion representing rotation, the inverse is the
// same as the conjugate.

template <class T>
vnl_quaternion<T> vnl_quaternion<T>::inverse() const
{
  vnl_quaternion<T> inv = this->conjugate();
  inv /= vnl_c_vector<T>::dot_product(this->data_, this->data_, 4);
  return inv;
}

//: Returns  the product of two quaternions.
// Multiplication of two quaternions is not symmetric and has
// fewer  operations  than  multiplication  of orthonormal
// matrices. If object is rotated by r1, then by r2,  then
// the  composed  rotation (r2 o r1) is represented by the
// quaternion (q2 * q1), or by the matrix (m1 * m2).  Note
// that  matrix  composition  is reversed because matrices
// and vectors are represented row-wise.

template <class T>
vnl_quaternion<T> vnl_quaternion<T>::operator* (vnl_quaternion<T> const& rhs) const
{
  T r1 = this->real();                  // real and img parts of args
  T r2 = rhs.real();
  vnl_vector_fixed<T,3> i1 = this->imaginary();
  vnl_vector_fixed<T,3> i2 = rhs.imaginary();
  T real_v = (r1 * r2) - ::dot_product(i1, i2); // real&img of product q1*q2
  vnl_vector_fixed<T,3> img = vnl_cross_3d(i1, i2);
  img += (i2 * r1) + (i1 * r2);
  return vnl_quaternion<T>(img[0], img[1], img[2], real_v);
}

//: Rotates 3D vector v with source quaternion and stores the rotated vector back into v.
// For speed and greater accuracy, first convert quaternion into an orthonormal
// matrix,  then  use matrix multiplication to rotate many vectors.

template <class T>
vnl_vector_fixed<T,3> vnl_quaternion<T>::rotate(vnl_vector_fixed<T,3> const& v) const
{
  T r = this->real();
  vnl_vector_fixed<T,3> i = this->imaginary();
  vnl_vector_fixed<T,3> i_x_v(vnl_cross_3d(i, v));
  return v + i_x_v * T(2*r) - vnl_cross_3d(i_x_v, i) * T(2);
}

#undef VNL_QUATERNION_INSTANTIATE
#define VNL_QUATERNION_INSTANTIATE(T) \
template class vnl_quaternion<T >;\
VCL_INSTANTIATE_INLINE(vcl_ostream& operator<< (vcl_ostream&, vnl_quaternion<T > const&))

#endif // vnl_quaternion_txx_
