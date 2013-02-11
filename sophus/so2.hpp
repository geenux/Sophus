// This file is part of Sophus.
//
// Copyright 2012-2013 Hauke Strasdat
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights  to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#ifndef SOPHUS_SO2_HPP
#define SOPHUS_SO2_HPP

#include <complex>

#include "sophus.hpp"

////////////////////////////////////////////////////////////////////////////
// Forward Declarations / typedefs
////////////////////////////////////////////////////////////////////////////

namespace Sophus {
template<typename _Scalar, int _Options=0> class SO2Group;
typedef SOPHUS_DEPRECATED SO2Group<double> SO2;
typedef SO2Group<double> SO2d; /**< double precision SO2 */
typedef SO2Group<float> SO2f;  /**< single precision SO2 */
}

////////////////////////////////////////////////////////////////////////////
// Eigen Traits (For querying derived types in CRTP hierarchy)
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// Eigen Traits (For querying derived types in CRTP hierarchy)
////////////////////////////////////////////////////////////////////////////

namespace Eigen {
namespace internal {

template<typename _Scalar, int _Options>
struct traits<Sophus::SO2Group<_Scalar,_Options> > {
  typedef _Scalar Scalar;
  typedef Matrix<Scalar,2,1> ComplexType;
};

template<typename _Scalar, int _Options>
struct traits<Map<Sophus::SO2Group<_Scalar>, _Options> >
    : traits<Sophus::SO2Group<_Scalar, _Options> > {
  typedef _Scalar Scalar;
  typedef Map<Matrix<Scalar,2,1>,_Options> ComplexType;
};

template<typename _Scalar, int _Options>
struct traits<Map<const Sophus::SO2Group<_Scalar>, _Options> >
    : traits<const Sophus::SO2Group<_Scalar, _Options> > {
  typedef _Scalar Scalar;
  typedef Map<const Matrix<Scalar,2,1>,_Options> ComplexType;
};

}
}

namespace Sophus {
using namespace Eigen;

/**
 * \brief SO2 base type - implements SO2 class but is storage agnostic
 *
 * [add more detailed description/tutorial]
 */
template<typename Derived>
class SO2GroupBase {
public:
  typedef typename internal::traits<Derived>::Scalar Scalar;
  typedef typename internal::traits<Derived>::ComplexType ComplexType;
  /** \brief degree of freedom of group */
  static const int DoF = 1;
  /** \brief number of internal parameters used */
  static const int num_parameters = 2;

  /**
   * \brief Adjoint transformation
   *
   * This function return the adjoint transformation \f$ Ad \f$ of the
   * group instance \f$ A \f$  such that for all \f$ x \f$
   * it holds that \f$ \widehat{Ad_A\cdot x} = A\widehat{x}A^{-1} \f$
   * with \f$\ \widehat{\cdot} \f$ being the hat()-operator.
   *
   * For SO2, it simply returns 1.
   */
  inline
  const Scalar Adj() const {
    return 1;
  }

  /**
   * \returns copy of instance casted to NewScalarType
   */
  template<typename NewScalarType>
  inline SO2Group<NewScalarType> cast() const {
    return SO2Group<NewScalarType>(unit_complex()
                                   .template cast<NewScalarType>() );
  }

  /**
   * \returns pointer to internal data
   *
   * This provides unsafe read/write access to internal data. SO2 is represented
   * by a complex number with unit length (two parameters). When using direct
   * write access, the user needs to take care of that the complex number stays
   * normalized.
   *
   * \see normalize()
   */
  inline Scalar* data() {
    return unit_complex_nonconst().data();
  }

  /**
   * \returns const pointer to internal data
   *
   * Const version of data().
   */
  inline const Scalar* data() const {
    return reinterpret_cast<Scalar>(&unit_complex_nonconst());
  }

  /**
   * \brief Fast group multiplication
   *
   * This method is a fast version of operator*=(), since it does not perform
   * normalization. It is up to the user to call normalize() once in a while.
   *
   * \see operator*=()
   */
  inline
  void fastMultiply(const SO2Group<Scalar>& other) {
    Scalar lhs_real = unit_complex().x();
    Scalar lhs_imag = unit_complex().y();
    const Scalar & rhs_real = other.unit_complex().x();
    const Scalar & rhs_imag = other.unit_complex().y();
    // complex multiplication
    unit_complex_nonconst().x() = lhs_real*rhs_real - lhs_imag*rhs_imag;
    unit_complex_nonconst().y() = lhs_real*rhs_imag + lhs_imag*rhs_real;
  }

  /**
   * \returns group inverse of instance
   */
  inline
  const SO2Group<Scalar> inverse() const {
    return SO2Group<Scalar>(unit_complex().x(), -unit_complex().y());
  }

  /**
   * \brief Logarithmic map
   *
   * \returns tangent space representation (=rotation angle) of instance
   *
   * \see  log().
   */
  inline
  const Scalar log() const {
    return SO2Group<Scalar>::log(*this);
  }

  /**
   * \brief Normalize complex number
   *
   * It re-normalizes complex number to unit length. This method only needs to
   * be called in conjunction with fastMultiply() or data() write access.
   */
  inline
  void normalize() {
    Scalar length =
        sqrt(unit_complex().x()*unit_complex().x()
             + unit_complex().y()*unit_complex().y());
    assert(length > SophusConstants<Scalar>::epsilon());
    unit_complex_nonconst().x() /= length;
    unit_complex_nonconst().y() /= length;
  }

  /**
   * \returns 2x2 matrix representation of instance
   *
   * For SO2, the matrix representation is an orthogonal matrix R with det(R)=1,
   * thus the so-called rotation matrix.
   */
  inline
  const Matrix<Scalar,2,2> matrix() const {
    const Scalar & real = unit_complex().x();
    const Scalar & imag = unit_complex().y();
    Matrix<Scalar,2,2> R;
    R(0,0) = real; R(0,1) = -imag;
    R(1,0) = imag; R(1,1) =  real;
    return R;
  }

  /**
   * \brief Assignment operator
   */
  template<typename OtherDerived> inline
  SO2GroupBase<Derived>& operator=(const SO2GroupBase<OtherDerived> & other) {
    unit_complex_nonconst() = other.unit_complex();
    return *this;
  }

  /**
   * \brief Group multiplication
   * \see operator*=()
   */
  inline
  const SO2Group<Scalar> operator*(const SO2Group<Scalar>& other) const {
    SO2Group<Scalar> result(*this);
    result *= other;
    return result;
  }

  /**
   * \brief Group action on \f$ \mathbf{R}^2 \f$
   *
   * \param p point \f$p \in \mathbf{R}^2 \f$
   * \returns point \f$p' \in \mathbf{R}^2 \f$, rotated version of \f$p\f$
   *
   * This function rotates a point \f$ p \f$ in  \f$ \mathbf{R}^2 \f$ by the
   * SO2 transformation \f$R\f$ (=rotation matrix): \f$ p' = R\cdot p \f$.
   */
  inline
  const Matrix<Scalar,2,1> operator*(const Matrix<Scalar,2,1> & p) const {
    const Scalar & real = unit_complex().x();
    const Scalar & imag = unit_complex().y();
    return Matrix<Scalar,2,1>(real*p[0] - imag*p[1], imag*p[0] + real*p[1]);
  }

  /**
   * \brief In-place group multiplication
   *
   * \see fastMultiply()
   * \see operator*()
   */
  inline
  void operator*=(const SO2Group<Scalar>& other) {
    fastMultiply(other);
    normalize();
  }

  /**
   * \brief Setter of internal unit complex number representation
   *
   * \param complex
   * \pre   the complex number must not be zero
   *
   * The complex number is normalized to unit length.
   */
  inline
  void setComplex(const ComplexType& complex) {
    assert(abs(complex)!=static_cast<Scalar>(0));
    unit_complex() = complex;
    normalize();
  }

  /**
   * \brief Read access to unit complex number
   *
   * No direct write access is given to ensure the complex stays normalized.
   */
  EIGEN_STRONG_INLINE
  const ComplexType& unit_complex() const {
    return static_cast<const Derived*>(this)->unit_complex();
  }

  ////////////////////////////////////////////////////////////////////////////
  // public static functions
  ////////////////////////////////////////////////////////////////////////////

  /**
   * \brief Group exponential
   *
   * \param theta tangent space element (=rotation angle \f$ \theta \f$)
   * \returns     corresponding element of the group SO2
   *
   * To be more specific, this function computes \f$ \exp(\widehat{\theta}) \f$
   * with \f$ \exp(\cdot) \f$ being the matrix exponential
   * and \f$ \widehat{\cdot} \f$ the hat()-operator of SO2.
   *
   * \see hat()
   * \see log()
   */
  inline static
  const SO2Group<Scalar> exp(const Scalar & theta) {
    return SO2Group<Scalar>(ComplexType(cos(theta), sin(theta)));
  }

  /**
   * \brief Generator
   *
   * The infinitesimal generator of SO2
   * is \f$
   *        G_0 = \left( \begin{array}{ccc}
   *                          0& -1& \\
   *                          1&  0&
   *                     \end{array} \right).
   * \f$
   * \see hat()
   */
  inline static
  const Matrix<Scalar,2,2> generator() {
    return hat(1);
  }

  /**
   * \brief hat-operator
   *
   * \param theta scalar representation of Lie algebra element
   * \returns     2x2-matrix representatin of Lie algebra element
   *
   * Formally, the hat-operator of SO2 is defined
   * as \f$ \widehat{\cdot}: \mathbf{R}^2 \rightarrow \mathbf{R}^{2\times 2},
   * \quad \widehat{\theta} = G_0\cdot \theta \f$
   * with \f$ G_0 \f$ being the infinitesial generator().
   *
   * \see generator()
   * \see vee()
   */
  inline static
  const Matrix<Scalar,2,2> hat(const Scalar & theta) {
    Matrix<Scalar,2,2> Omega;
    Omega <<  static_cast<Scalar>(0), -theta
        ,  theta,     static_cast<Scalar>(0);
    return Omega;
  }

  /**
   * \brief Lie bracket
   *
   * \param theta1 scalar representation of Lie algebra element
   * \param theta2 scalar representation of Lie algebra element
   * \returns      zero
   *
   * It computes the bracket. For the Lie algebra so2, the Lie bracket is
   * simply \f$ [\theta_1, \theta_2]_{so2} = 0 \f$ since SO2 is a
   * commutative group.
   *
   * \see hat()
   * \see vee()
   */
  inline static
  const Scalar lieBracket(const Scalar & theta1,
                          const Scalar & theta2) {
    return static_cast<Scalar>(0);
  }

  /**
   * \brief Logarithmic map
   *
   * \param other element of the group SO2
   * \returns     corresponding tangent space element
   *              (=rotation angle \f$ \theta \f$)
   *
   * Computes the logarithmic, the inverse of the group exponential.
   * To be specific, this function computes \f$ \log({\cdot})^\vee \f$
   * with \f$ \vee(\cdot) \f$ being the matrix logarithm
   * and \f$ \vee{\cdot} \f$ the vee()-operator of SO2.
   *
   * \see exp()
   * \see vee()
   */
  inline static
  const Scalar log(const SO2Group<Scalar> & other) {
    // todo: general implementation for Scalar not being float or double.
    return atan2(other.unit_complex_.y(), other.unit_complex().x());
  }

  /**
   * \brief vee-operator
   *
   * \param Omega 2x2-matrix representation of Lie algebra element
   * \returns     scalar representatin of Lie algebra element
   *
   * This is the inverse of the hat()-operator.
   *
   * \see hat()
   */
  inline static
  const Scalar vee(const Matrix<Scalar,2,2> & Omega) {
    assert(fabs(Omega(1,0)+Omega(0,1)) < SophusConstants<Scalar>::epsilon());
    return Omega(1,0);
  }

private:
  // Nonconst accessor of complex number is private so users are hampered
  // from setting non-unit complex numbers.
  EIGEN_STRONG_INLINE
  ComplexType& unit_complex_nonconst() {
    return static_cast<Derived*>(this)->unit_complex_nonconst();
  }

};

/**
 * \brief SO2 default type - Constructors and default storage for SO2 Type
 */
template<typename _Scalar, int _Options>
class SO2Group : public SO2GroupBase<SO2Group<_Scalar,_Options> > {
public:
  typedef typename internal::traits<SO2Group<_Scalar,_Options> >
  ::Scalar Scalar;
  typedef typename internal::traits<SO2Group<_Scalar,_Options> >
  ::ComplexType ComplexType;

  // base is friend so unit_complex_nonconst can be accessed from base
  friend class SO2GroupBase<SO2Group<_Scalar,_Options> >;

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  /**
   * \brief Default constructor
   *
   * Initialize complex number to identity rotation.
   */
  inline SO2Group()
    : unit_complex_(static_cast<Scalar>(1), static_cast<Scalar>(0)) {
  }

  /**
   * \brief Copy constructor
   */
  template<typename OtherDerived> inline
  SO2Group(const SO2GroupBase<OtherDerived> & other)
    : unit_complex_(other.unit_complex()) {
  }

  /**
   * \brief Constructor from rotation matrix
   *
   * \pre rotation matrix need to be orthogonal with determinant of 1
   */
  inline SO2Group(const Matrix<Scalar,2,2> & R)
    : unit_complex_(static_cast<Scalar>(0.5)*(R(0,0)+R(1,1)),
                    static_cast<Scalar>(0.5)*(R(1,0)-R(0,1))) {
  }

  /**
   * \brief Constructor from pair of real and imaginary number
   *
   * \pre pair must not be zero
   */
  inline SO2Group(const Scalar & real, const Scalar & imag)
    : unit_complex_(real, imag) {
    assert(real*real + imag*imag > SophusConstants<Scalar>::epsilon());
    normalize();
  }

  /**
   * \brief Constructor from 2-vector
   *
   * \pre vector must not be zero
   */
  inline SO2Group(const ComplexType & complex)
    : unit_complex_(complex) {
    assert(complex.x()*complex.x() + complex.y()*complex.y()
           > SophusConstants<Scalar>::epsilon());
    normalize();
  }

  /**
   * \brief Constructor from std::complex
   *
   * \pre complex number must not be zero
   */
  inline SO2Group(const std::complex<Scalar> & complex)
    : unit_complex_(complex.real(), complex.imag()) {
    assert(complex.real()*complex.real() + complex.imag()*complex.imag()
           > SophusConstants<Scalar>::epsilon());
    normalize();
  }

  /**
   * \brief Constructor from an angle
   */
  inline SO2Group(Scalar theta) {
    unit_complex_nonconst() = SO2Group<Scalar>::exp(theta).unit_complex();
  }

  /**
   * \brief Normalize complex number
   *
   * It re-normalizes complex number to unit length. This method only needs to
   * be called in conjunction with fastMultiply() or data() write access.
   */
  inline
  void normalize() {
    Scalar length =
        sqrt(unit_complex_.x()*unit_complex_.x()
             + unit_complex_.y()*unit_complex_.y());
    assert(length > SophusConstants<Scalar>::epsilon());
    unit_complex_.x() /= length;
    unit_complex_.y() /= length;
  }

  /**
   * \brief Read access to unit complex number
   *
   * No direct write access is given to ensure the complex number stays
   * normalized.
   */
  EIGEN_STRONG_INLINE
  const ComplexType & unit_complex() const {
    return unit_complex_;
  }

protected:
  // Nonconst accessor of unit_complex is protected so users are hampered
  // from setting non-unit complex numbers.
  EIGEN_STRONG_INLINE
  ComplexType & unit_complex_nonconst() {
    return unit_complex_;
  }

  ComplexType unit_complex_;
};

} // end namespace


namespace Eigen {
/**
 * \brief Specialisation of Eigen::Map for SO3GroupBase
 *
 * Allows us to wrap SO2 Objects around POD array
 * (e.g. external c style complex number)
 */
template<typename _Scalar, int _Options>
class Map<Sophus::SO2Group<_Scalar>, _Options>
    : public Sophus::SO2GroupBase<Map<Sophus::SO2Group<_Scalar>, _Options> > {
  typedef Sophus::SO2GroupBase<Map<Sophus::SO2Group<_Scalar>, _Options> > Base;

public:
  typedef typename internal::traits<Map>::Scalar Scalar;
  typedef typename internal::traits<Map>::ComplexType ComplexType;

  // base is friend so unit_complex_nonconst can be accessed from base
  friend class Sophus::SO2GroupBase<Map<Sophus::SO2Group<_Scalar>, _Options> >;

  EIGEN_INHERIT_ASSIGNMENT_EQUAL_OPERATOR(Map)
  using Base::operator*=;
  using Base::operator*;

  EIGEN_STRONG_INLINE
  Map(Scalar* coeffs) : unit_complex_(coeffs) {
  }

  /**
   * \brief Read access to unit complex number
   *
   * No direct write access is given to ensure the complex number stays
   * normalized.
   */
  EIGEN_STRONG_INLINE
  const ComplexType & unit_complex() const {
    return unit_complex_;
  }

protected:
  // Nonconst accessor of complex number is protected so users are hampered
  // from setting non-unit complex number.
  EIGEN_STRONG_INLINE
  ComplexType & unit_complex_nonconst() {
    return unit_complex_;
  }

  ComplexType unit_complex_;
};

/**
 * \brief Specialisation of Eigen::Map for const SO3GroupBase
 *
 * Allows us to wrap SO2 Objects around POD array
 * (e.g. external c style complex number)
 */
template<typename _Scalar, int _Options>
class Map<const Sophus::SO2Group<_Scalar>, _Options>
    : public Sophus::SO2GroupBase<
    Map<const Sophus::SO2Group<_Scalar>, _Options> > {
  typedef Sophus::SO2GroupBase<Map<const Sophus::SO2Group<_Scalar>, _Options> >
  Base;

public:
  typedef typename internal::traits<Map>::Scalar Scalar;
  typedef typename internal::traits<Map>::ComplexType ComplexType;

  EIGEN_INHERIT_ASSIGNMENT_EQUAL_OPERATOR(Map)
  using Base::operator*=;
  using Base::operator*;

  EIGEN_STRONG_INLINE
  Map(const Scalar* coeffs) : unit_complex_(coeffs) {
  }

  /**
   * \brief Read access to unit complex number
   *
   * No direct write access is given to ensure the complex number stays
   * normalized.
   */
  EIGEN_STRONG_INLINE
  const ComplexType & unit_complex() const {
    return unit_complex_;
  }

protected:
  const ComplexType unit_complex_;
};

}


#endif // SOPHUS_SO2_HPP