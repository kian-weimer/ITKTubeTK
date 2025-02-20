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

#ifndef __tubeRigidSpatialObjectToImageRegistrationMethod_hxx
#define __tubeRigidSpatialObjectToImageRegistrationMethod_hxx


#include "vnl/vnl_inverse.h"

namespace itk
{

namespace tube
{

template <unsigned int ObjectDimension, class TImage>
RigidSpatialObjectToImageRegistrationMethod<ObjectDimension, TImage>
::RigidSpatialObjectToImageRegistrationMethod( void )
{
  if( ImageDimension == 2 )
    {
    typename Rigid2DTransformType::Pointer tmpTrans =
      Rigid2DTransformType::New();
    this->SetTransform( dynamic_cast<RigidTransformType *>(
                          tmpTrans.GetPointer() ) );
    tmpTrans->Register();
    }
  else if( ImageDimension == 3 )
    {
    typename Rigid3DTransformType::Pointer tmpTrans =
      Rigid3DTransformType::New();
    this->SetTransform( dynamic_cast<RigidTransformType *>(
                          tmpTrans.GetPointer() ) );
    tmpTrans->Register();
    }
  else
    {
    std::cerr << "ERROR: Rigid registration only supported for 2D & 3D images."
              << std::endl;
    }

  this->GetTypedTransform()->SetIdentity();

  this->SetInitialTransformParameters( this->GetTypedTransform()
                                       ->GetParameters() );
  this->SetInitialTransformFixedParameters( this->GetTypedTransform()
                                            ->GetFixedParameters() );
  this->SetLastTransformParameters( this->GetTypedTransform()
                                    ->GetParameters() );

  typename Superclass::TransformParametersScalesType scales;
  scales.set_size( this->GetTypedTransform()->GetNumberOfParameters() );
  if( ImageDimension == 2 )
    {
    scales[0] = 10;
    scales[1] = 0.1;
    scales[2] = 0.1;
    }
  else if( ImageDimension == 3 )
    {
    scales[0] = 10;
    scales[1] = 10;
    scales[2] = 10;
    scales[3] = 0.1;
    scales[4] = 0.1;
    scales[5] = 0.1;
    }

  this->SetTransformParametersScales( scales );

  this->SetTransformMethodEnum( Superclass::RIGID_TRANSFORM );
}

template <unsigned int ObjectDimension, class TImage>
RigidSpatialObjectToImageRegistrationMethod<ObjectDimension, TImage>
::~RigidSpatialObjectToImageRegistrationMethod( void )
{
  this->m_Transform->UnRegister();
}

template <unsigned int ObjectDimension, class TImage>
typename RigidSpatialObjectToImageRegistrationMethod<ObjectDimension, TImage>::TransformType
* RigidSpatialObjectToImageRegistrationMethod<ObjectDimension, TImage>
::GetTypedTransform( void )
  {
  return dynamic_cast<TransformType  *>( Superclass::GetTransform() );
  }

template <unsigned int ObjectDimension, class TImage>
const typename RigidSpatialObjectToImageRegistrationMethod<ObjectDimension, TImage>::TransformType
* RigidSpatialObjectToImageRegistrationMethod<ObjectDimension, TImage>
::GetTypedTransform( void ) const
  {
  return dynamic_cast<const TransformType  *>( Superclass::GetTransform() );
  }

template <unsigned int ObjectDimension, class TImage>
typename RigidSpatialObjectToImageRegistrationMethod<ObjectDimension, TImage>::AffineTransformPointer
RigidSpatialObjectToImageRegistrationMethod<ObjectDimension, TImage>
::GetAffineTransform( void ) const
{
  typename AffineTransformType::Pointer trans = AffineTransformType::New();

  trans->SetIdentity();
  trans->SetCenter( this->GetTypedTransform()->GetCenter() );
  trans->SetMatrix( this->GetTypedTransform()->GetMatrix() );
  trans->SetOffset( this->GetTypedTransform()->GetOffset() );

  return trans;
}

template <unsigned int ObjectDimension, class TImage>
void
RigidSpatialObjectToImageRegistrationMethod<ObjectDimension, TImage>
::SetInitialTransformParametersFromAffineTransform( const AffineTransformType * affine )
{
  RigidTransformType * rigidTransform =
    dynamic_cast<RigidTransformType *>( this->GetTransform() );

  if( !rigidTransform )
    {
    itkExceptionMacro("GetTransform() didn't return a Rigid Transform");
    }

  rigidTransform->SetCenter( affine->GetCenter() );
  rigidTransform->SetTranslation( affine->GetTranslation() );

  typedef vnl_matrix<double> VnlMatrixType;

  VnlMatrixType M = affine->GetMatrix().GetVnlMatrix();

  //
  // Polar decomposition algorithm proposed by [Higham 86]
  // SIAM J. Sci. Stat. Comput. Vol. 7, Num. 4, October 1986.
  // "Computing the Polar Decomposition - with Applications"
  // by Nicholas Higham.
  //
  // recommended by
  // Shoemake in the paper "Matrix Animation and Polar Decomposition".
  //
  VnlMatrixType PQ = M;
  VnlMatrixType NQ = M;
  VnlMatrixType PQNQDiff;

  const unsigned int maximumIterations = 100;
  for( unsigned int ni = 0; ni < maximumIterations; ni++ )
    {
    // Average current Qi with its inverse transpose
    NQ = ( PQ + vnl_inverse_transpose( PQ ) ) / 2.0;
    PQNQDiff = NQ - PQ;
    if( PQNQDiff.frobenius_norm() < 1e-7 )
      {
      break;
      }
    else
      {
      PQ = NQ;
      }
    }

  typename AffineTransformType::MatrixType QMatrix;

  QMatrix = NQ;

  rigidTransform->SetMatrix( QMatrix );

  this->SetInitialTransformFixedParameters( rigidTransform->GetFixedParameters() );
  this->SetInitialTransformParameters( rigidTransform->GetParameters() );
}

template <unsigned int ObjectDimension, class TImage>
void
RigidSpatialObjectToImageRegistrationMethod<ObjectDimension, TImage>
::GenerateData( void )
{
  // Set the center of rotation
  this->GetTransform()->SetFixedParameters( this->GetInitialTransformFixedParameters() );

  Superclass::GenerateData();
}

template <unsigned int ObjectDimension, class TImage>
void
RigidSpatialObjectToImageRegistrationMethod<ObjectDimension, TImage>
::PrintSelf( std::ostream & os, Indent indent ) const
{
  this->Superclass::PrintSelf(os, indent);
}

}; // tube

}; // itk

#endif
