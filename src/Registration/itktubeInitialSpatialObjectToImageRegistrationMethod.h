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

#ifndef __itktubeInitialSpatialObjectToImageRegistrationMethod_h
#define __itktubeInitialSpatialObjectToImageRegistrationMethod_h

#include "itkImage.h"
#include "itkCommand.h"

#include "itktubeSpatialObjectToImageRegistrationMethod.h"

#include "itkAffineTransform.h"

namespace itk
{

namespace tube
{

template <unsigned int ObjectDimension, class TImage>
class InitialSpatialObjectToImageRegistrationMethod
  : public SpatialObjectToImageRegistrationMethod<ObjectDimension, TImage>
{

public:

  typedef InitialSpatialObjectToImageRegistrationMethod  Self;
  typedef SpatialObjectToImageRegistrationMethod<ObjectDimension, TImage> Superclass;
  typedef SmartPointer<Self>                     Pointer;
  typedef SmartPointer<const Self>               ConstPointer;

  itkTypeMacro( InitialSpatialObjectToImageRegistrationMethod,
                SpatialObjectToImageRegistrationMethod );

  itkNewMacro( Self );

  //
  // Typedefs from Superclass
  //
  typedef typename Superclass::SpatialObjectType SpatialObjectType;

  itkStaticConstMacro( ImageDimension, unsigned int,
                       TImage::ImageDimension );

  typedef AffineTransform<double, itkGetStaticConstMacro( ImageDimension )>
  TransformType;

  typedef typename TransformType::Pointer TransformPointer;

  //
  // Local Typedefs
  //
  typedef Point<double, itkGetStaticConstMacro( ImageDimension )>
  LandmarkPointType;
  typedef std::vector<LandmarkPointType> LandmarkPointContainer;

  //
  // Custom Methods
  //

  /**
   * The function performs the casting.  This function should only appear
   *   once in the class hierarchy.  It is provided so that member
   *   functions that exist only in specific transforms ( e.g., SetIdentity )
   *   can be called without the caller having to do the casting. */
  TransformType * GetTypedTransform( void );

  const TransformType * GetTypedTransform( void ) const;

  /** This method creates, initializes and returns an Affine transform.  The
   * transform is initialized with the current results available in the
   * GetTypedTransform() method. The returned transform is not a member
   * variable, and therefore, must be received into a SmartPointer to prevent
   * it from being destroyed by depletion of its reference counting. */
  TransformPointer GetAffineTransform( void ) const;

  itkSetMacro( NumberOfMoments, unsigned int );
  itkGetConstMacro( NumberOfMoments, unsigned int );

  itkSetMacro( ComputeCenterOfRotationOnly, bool );
  itkGetConstMacro( ComputeCenterOfRotationOnly, bool );

  itkSetMacro( UseLandmarks, bool );
  itkGetConstMacro( UseLandmarks, bool );

  void SetFixedLandmarks( const LandmarkPointContainer& fixedLandmarks );

  void SetMovingLandmarks( const LandmarkPointContainer& movingLandmarks );

protected:

  InitialSpatialObjectToImageRegistrationMethod( void );
  virtual ~InitialSpatialObjectToImageRegistrationMethod( void );

  void PrintSelf( std::ostream & os, Indent indent ) const override;

  //
  //  Methods from Superclass. Only the GenerateData() method should be
  //  overloaded. The Update() method must not be overloaded.
  //
  void    GenerateData() override;

private:

  // Purposely not implemented
  InitialSpatialObjectToImageRegistrationMethod( const Self & );
  // Purposely not implemented
  void operator =( const Self & );

  unsigned int           m_NumberOfMoments;
  bool                   m_ComputeCenterOfRotationOnly;
  bool                   m_UseLandmarks;
  LandmarkPointContainer m_FixedLandmarks;
  LandmarkPointContainer m_MovingLandmarks;
};

} // end namespace tube

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itktubeInitialSpatialObjectToImageRegistrationMethod.hxx"
#endif

#endif // __InitialSpatialObjectToImageRegistrationMethod_h
