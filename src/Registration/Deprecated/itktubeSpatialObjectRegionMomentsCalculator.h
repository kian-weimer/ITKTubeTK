/*=========================================================================

Library:   TubeTK

Copyright Kitware Inc.

All rights reserved.

Licensed under the Apache License, Version 2.0 ( the "License" );
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

=========================================================================*/

#ifndef __itktubeSpatialObjectMomentsCalculator_h
#define __itktubeSpatialObjectMomentsCalculator_h

#include "itkAffineTransform.h"
#include "itkMacro.h"
#include "itkSpatialObject.h"

#include "vnl/vnl_vector_fixed.h"
#include "vnl/vnl_matrix_fixed.h"
#include "vnl/vnl_diag_matrix.h"

namespace itk
{

namespace tube
{

/** \class SpatialObjectMomentsCalculator
 * \brief Compute moments of an n-dimensional image.
 *
 * This class provides methods for computing the moments and related
 * properties of a single-echo image.  Computing the ( non-central )
 * moments of a large image can easily take a million times longer
 * than computing the various other values derived from them, so we
 * compute the moments only on explicit request, and save their values
 * ( in an SpatialObjectMomentsCalculator object ) for later retrieval by
 * the user.
 *
 * The methods that return values return the values themselves rather
 * than references because the cost is small compared to the cost of
 * computing the moments and doing so simplifies memory management for
 * the caller.
 *
 * \ingroup Operators
 */
template <unsigned int ObjectDimension>
class SpatialObjectMomentsCalculator : public Object
{
public:
  /** Standard class typedefs. */
  typedef SpatialObjectMomentsCalculator<TImage> Self;
  typedef Object                               Superclass;
  typedef SmartPointer<Self>                   Pointer;
  typedef SmartPointer<const Self>             ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro( Self );

  /** Run-time type information ( and related methods ). */
  itkTypeMacro( SpatialObjectMomentsCalculator, Object );

  /** Standard scalar type within this class. */
  typedef double ScalarType;

  typedef typename TImage::PointType PointType;

  /** Standard vector type within this class. */
  typedef Vector<ScalarType, ObjectDimension> VectorType;

  /** Spatial Object type within this class. */
  typedef SpatialObject<ObjectDimension> SpatialObjectType;

  /** Spatial Object member types used within this class. */
  typedef typename SpatialObjectType::Pointer      SpatialObjectPointer;
  typedef typename SpatialObjectType::ConstPointer SpatialObjectConstPointer;

  /** Standard matrix type within this class. */
  typedef Matrix<ScalarType, ObjectDimension, ObjectDimension>      MatrixType;

  /** Affine transform for mapping to and from principal axis */
  typedef AffineTransform<double, ObjectDimension> AffineTransformType;
  typedef typename AffineTransformType::Pointer    AffineTransformPointer;

  /** Set the spatial object. */
  virtual void SetSpatialObject( const SpatialObject<ObjectDimension> * so )
    {
    if( m_SpatialObject != so )
      {
      m_SpatialObject = so;
      this->Modified();
      m_Valid = false;
      }
    }

  /** Set the spatial object mask - which defines the ROI over which
   *   moments are computed from the input spatial object. */
  virtual void SetSpatialObjectMask( const SpatialObject<ObjectDimension> * so )
    {
    if( m_SpatialObjectMask != so )
      {
      m_SpatialObjectMask = so;
      this->Modified();
      m_Valid = false;
      }
    }

  /** Compute moments of a new or modified image.
   * This method computes the moments of the image given as a
   * parameter and stores them in the object.  The values of these
   * moments and related parameters can then be retrieved by using
   * other methods of this object. */
  void Compute( void );

  /** Return the total mass ( or zeroth moment ) of an image.
   * This method returns the sum of pixel intensities ( also known as
   * the zeroth moment or the total mass ) of the image whose moments
   * were last computed by this object. */
  ScalarType GetTotalMass() const;

  /** Return first moments about origin, in index coordinates.
   * This method returns the first moments around the origin of the
   * image whose moments were last computed by this object.  For
   * simplicity, these moments are computed in index coordinates
   * rather than physical coordinates. */
  VectorType GetFirstMoments() const;

  /** Return second moments about origin, in index coordinates.
   * This method returns the second moments around the origin
   * of the image whose moments were last computed by this object.
   * For simplicity, these moments are computed in index coordinates
   * rather than physical coordinates. */
  MatrixType GetSecondMoments() const;

  /** Return center of gravity, in physical coordinates.
   * This method returns the center of gravity of the image whose
   * moments were last computed by this object.  The center of
   * gravity is computed in physical coordinates. */
  VectorType GetCenterOfGravity() const;

  /** Return second central moments, in physical coordinates.
   * This method returns the central second moments of the image
   * whose moments were last computed by this object.  The central
   * moments are computed in physical coordinates. */
  MatrixType GetCentralMoments() const;

  /** Return principal moments, in physical coordinates.
   * This method returns the principal moments of the image whose
   * moments were last computed by this object.  The moments are
   * returned as a vector, with the principal moments ordered from
   * smallest to largest.  The moments are computed in physical
   * coordinates.   */
  VectorType GetPrincipalMoments() const;

  /** Return principal axes, in physical coordinates.
   * This method returns the principal axes of the image whose
   * moments were last computed by this object.  The moments are
   * returned as an orthogonal matrix, each row of which corresponds
   * to one principal moment; for example, the principal axis
   * corresponding to the smallest principal moment is the vector
   * m[0], where m is the value returned by this method.  The matrix
   * of principal axes is guaranteed to be a proper rotation; that
   * is, to have determinant +1 and to preserve parity.  ( Unless you
   * have foolishly made one or more of the spacing values negative;
   * in that case, _you_ get to figure out the consequences. )  The
   * moments are computed in physical coordinates. */
  MatrixType GetPrincipalAxes() const;

  /** Get the affine transform from principal axes to physical axes
   * This method returns an affine transform which transforms from
   * the principal axes coordinate system to physical coordinates. */
  AffineTransformPointer GetPrincipalAxesToPhysicalAxesTransform(
    void ) const;

  /** Get the affine transform from physical axes to principal axes
   * This method returns an affine transform which transforms from
   * the physical coordinate system to the principal axes coordinate
   * system. */
  AffineTransformPointer GetPhysicalAxesToPrincipalAxesTransform(
    void ) const;

protected:
  SpatialObjectMomentsCalculator();
  virtual ~SpatialObjectMomentsCalculator();

  void PrintSelf( std::ostream& os, Indent indent ) const override;

private:
  SpatialObjectMomentsCalculator( const Self & ); // purposely not implemented
  void operator=( const Self & );               // purposely not implemented

  bool       m_Valid;                // Have moments been computed yet?
  ScalarType m_M0;                   // Zeroth moment
  VectorType m_M1;                   // First moments about origin
  MatrixType m_M2;                   // Second moments about origin
  VectorType m_Cg;                   // Center of gravity ( physical units )
  MatrixType m_Cm;                   // Second central moments ( physical )
  VectorType m_Pm;                   // Principal moments ( physical )
  MatrixType m_Pa;                   // Principal axes ( physical )

  SpatialObjectConstPointer m_SpatialObject;
  SpatialObjectConstPointer m_SpatialObjectMask;

};  // class SpatialObjectMomentsCalculator

} // end namespace tube

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itktubeSpatialObjectMomentsCalculator.hxx"
#endif

#endif /* __itkSpatialObjectMomentsCalculator_h */
