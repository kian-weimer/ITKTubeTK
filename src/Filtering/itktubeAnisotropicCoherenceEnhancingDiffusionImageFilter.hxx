/*=========================================================================

Library:   TubeTKLib

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

#ifndef __itktubeAnisotropicCoherenceEnhancingDiffusionImageFilter_hxx
#define __itktubeAnisotropicCoherenceEnhancingDiffusionImageFilter_hxx


#include <itkFixedArray.h>
#include <itkImageFileWriter.h>
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIterator.h>
#include <itkNeighborhoodAlgorithm.h>
#include <itkNumericTraits.h>
#include <itkVector.h>

#include <list>

namespace itk
{

namespace tube
{

/**
 * Constructor
 */
template< class TInputImage, class TOutputImage >
AnisotropicCoherenceEnhancingDiffusionImageFilter<TInputImage, TOutputImage>
::AnisotropicCoherenceEnhancingDiffusionImageFilter( void )
{
  m_ContrastParameterLambdaC = 15.0;
  m_Alpha = 0.001;
  m_Sigma = 1.0;
  m_SigmaOuter = 1.0;
}

template< class TInputImage, class TOutputImage >
void
AnisotropicCoherenceEnhancingDiffusionImageFilter<TInputImage, TOutputImage>
::UpdateDiffusionTensorImage( void )
{
  itkDebugMacro( << "UpdateDiffusionTensorImage() called." );

  std::cerr << "UpdateDiffusionTensorImage()" << std::endl;

  /* IN THIS METHOD, the following items will be implemented
   - Compute the structure tensor
   - Compute its eigenvectors
   - Compute eigenvalues corresponding to the diffusion matrix tensor
  */

  //Step 1: Compute the structure tensor and identify the eigenvectors
  //Step 1.1: Compute the structure tensor
  // Instantiate the structure tensor filter
  typename StructureTensorFilterType::Pointer
          StructureTensorFilter  = StructureTensorFilterType::New();

  StructureTensorFilter->SetInput( this->GetOutput() );
  StructureTensorFilter->SetSigma ( m_Sigma );
  StructureTensorFilter->SetSigmaOuter ( m_SigmaOuter );
  StructureTensorFilter->Update();

  // Step 1.2: Identify the eigenvectors of the structure tensor
  typedef  Matrix< double, 3, 3>
    EigenVectorMatrixType;
  typedef  Image< EigenVectorMatrixType, 3>
    EigenVectorImageType;
  typedef  typename itk::Image< EigenValueArrayType, 3>
    EigenValueImageType;

  typedef  typename StructureTensorFilterType::OutputImageType
    SymmetricSecondRankTensorImageType;
  typedef  SymmetricEigenVectorAnalysisImageFilter<
    SymmetricSecondRankTensorImageType, EigenValueImageType,
    EigenVectorImageType>
    EigenVectorAnalysisFilterType;

  typename EigenVectorAnalysisFilterType::Pointer eigenVectorAnalysisFilter
    = EigenVectorAnalysisFilterType::New();
  eigenVectorAnalysisFilter->SetDimension( 3 );
  eigenVectorAnalysisFilter->OrderEigenValuesBy(
    EigenValueOrderEnum::OrderByValue );

  eigenVectorAnalysisFilter->SetInput( StructureTensorFilter->GetOutput() );
  eigenVectorAnalysisFilter->Modified();
  eigenVectorAnalysisFilter->Update();

  //Step 1.3: Compute the eigenvalues
  typedef itk::SymmetricEigenAnalysisImageFilter<
    SymmetricSecondRankTensorImageType, EigenValueImageType>
    EigenAnalysisFilterType;

  typename EigenAnalysisFilterType::Pointer eigenAnalysisFilter
    = EigenAnalysisFilterType::New();
  eigenAnalysisFilter->SetDimension( 3 );
  eigenAnalysisFilter->OrderEigenValuesBy(
    EigenValueOrderEnum::OrderByValue );

  eigenAnalysisFilter->SetInput( StructureTensorFilter->GetOutput() );
  eigenAnalysisFilter->Update();

  /* Step 2: Generate the diffusion tensor matrix
      D = [v1 v2 v3] [DiagonalMatrixContainingLambdas] [v1 v2 v3]^t
  */

  //Setup the iterators
  //
  //Iterator for the eigenvector matrix image
  EigenVectorImageType::ConstPointer eigenVectorImage
    = eigenVectorAnalysisFilter->GetOutput();
  itk::ImageRegionConstIterator<EigenVectorImageType>
    eigenVectorImageIterator;
  eigenVectorImageIterator
    = itk::ImageRegionConstIterator<EigenVectorImageType>( eigenVectorImage,
    eigenVectorImage->GetRequestedRegion() );
  eigenVectorImageIterator.GoToBegin();

  //Iterator for the diffusion tensor image
  typedef itk::ImageRegionIterator< DiffusionTensorImageType >
    DiffusionTensorIteratorType;
  DiffusionTensorIteratorType it( this->GetDiffusionTensorImage(),
    this->GetDiffusionTensorImage()->GetLargestPossibleRegion() );

  //Iterator for the eigenvalue image
  typename EigenValueImageType::ConstPointer eigenImage
    = eigenAnalysisFilter->GetOutput();
  itk::ImageRegionConstIterator<EigenValueImageType>
    eigenValueImageIterator;
  eigenValueImageIterator = itk::ImageRegionConstIterator<
    EigenValueImageType>( eigenImage, eigenImage->GetRequestedRegion() );
  eigenValueImageIterator.GoToBegin();

  it.GoToBegin();
  eigenVectorImageIterator.GoToBegin();
  eigenValueImageIterator.GoToBegin();

  MatrixType  eigenValueMatrix;
  while( !it.IsAtEnd() )
    {
    // Generate the diagonal matrix with the eigenvalues
    eigenValueMatrix.SetIdentity();

    //Set the lambda's appropriately. For now, set them to be equal to the
    //eigenvalues
    double Lambda1;
    double Lambda2;
    double Lambda3;

    // Get the eigenvalue
    // Make sure they are sorted
    EigenValueArrayType eigenValue;
    eigenValue = eigenValueImageIterator.Get();

    /* Assumption is that eigenvalue1 > eigenvalue2 > eigenvalue3 */

    // Find the smallest eigenvalue
    double smallest = std::fabs( eigenValue[0] );
    unsigned int smallestEigenValueIndex=0;
    for( unsigned int i=1; i <=2; i++ )
      {
      if( std::fabs( eigenValue[i] ) < smallest )
        {
        Lambda1 = eigenValue[i];
        smallest = std::fabs( eigenValue[i] );
        smallestEigenValueIndex = i;
        }
      }

    // Find the largest eigenvalue
    double largest = std::fabs( eigenValue[0] );
    unsigned int largestEigenValueIndex=0;
    for( unsigned int i=1; i <=2; i++ )
      {
      if( std::fabs( eigenValue[i] > largest ) )
        {
        largest = std::fabs( eigenValue[i] );
        largestEigenValueIndex = i;
        }
      }

    unsigned int middleEigenValueIndex=0;
    for( unsigned int i=0; i <=2; i++ )
      {
      if( eigenValue[i] != smallest && eigenValue[i] != largest )
        {
        middleEigenValueIndex = i;
        break;
        }
      }

    Lambda1 = m_Alpha;
    Lambda2 = m_Alpha;

    double zeroValueTolerance = 1.0e-20;

    /* largest > middle > smallest */

    if( ( std::fabs( eigenValue[middleEigenValueIndex] ) <
        zeroValueTolerance )  ||
       ( std::fabs( eigenValue[smallestEigenValueIndex] ) <
         zeroValueTolerance ) )
      {
      Lambda3 = 1.0;
      }
    else
      {
      double kappa = std::pow( (float)( eigenValue[middleEigenValueIndex] )
        / ( m_Alpha + eigenValue[smallestEigenValueIndex] ), 4.0 );

      double contrastParameterLambdaCSquare = m_ContrastParameterLambdaC
        * m_ContrastParameterLambdaC;

      double expVal = std::exp( ( -1.0 * ( std::log( 2.0 )
        * contrastParameterLambdaCSquare )/kappa ) );
      Lambda3 = m_Alpha + ( 1.0 - m_Alpha )*expVal;

      }

    eigenValueMatrix( 0, 0 ) = Lambda1;
    eigenValueMatrix( 1, 1 ) = Lambda2;
    eigenValueMatrix( 2, 2 ) = Lambda3;

    //Get the eigenVector matrix
    EigenVectorMatrixType eigenVectorMatrix;
    unsigned int vectorLength = 3; // Eigenvector length

    itk::VariableLengthVector<double> firstEigenVector( vectorLength );
    itk::VariableLengthVector<double> secondEigenVector( vectorLength );
    itk::VariableLengthVector<double> thirdEigenVector( vectorLength );

    for( unsigned int i=0; i < vectorLength; i++ ) {
    // Get eigenvectors belonging to eigenvalue order
      firstEigenVector[i] = eigenVectorMatrix[largestEigenValueIndex][i];
      secondEigenVector[i] = eigenVectorMatrix[middleEigenValueIndex][i];
      thirdEigenVector[i] = eigenVectorMatrix[smallestEigenValueIndex][i];

      // Set eigenVectorMatrix in correct order
      eigenVectorMatrix[0][i] = firstEigenVector[i];
      eigenVectorMatrix[1][i] = secondEigenVector[i];
      eigenVectorMatrix[2][i] = thirdEigenVector[i];
      }

    EigenVectorMatrixType  eigenVectorMatrixTranspose;
    eigenVectorMatrixTranspose = eigenVectorMatrix.GetTranspose();

    // Generate the tensor matrix
    EigenVectorMatrixType  productMatrix;
    productMatrix = eigenVectorMatrix * eigenValueMatrix
      * eigenVectorMatrixTranspose;

    //Copy the ITK::Matrix to the tensor...there should be a better way of
    //doing this TODO
    typename DiffusionTensorImageType::PixelType        tensor;

    tensor( 0, 0 ) = productMatrix( 0, 0 );
    tensor( 0, 1 ) = productMatrix( 0, 1 );
    tensor( 0, 2 ) = productMatrix( 0, 2 );

    tensor( 1, 0 ) = productMatrix( 1, 0 );
    tensor( 1, 1 ) = productMatrix( 1, 1 );
    tensor( 1, 2 ) = productMatrix( 1, 2 );

    tensor( 2, 0 ) = productMatrix( 2, 0 );
    tensor( 2, 1 ) = productMatrix( 2, 1 );
    tensor( 2, 2 ) = productMatrix( 2, 2 );
    it.Set( tensor );

    ++it;
    ++eigenValueImageIterator;
    ++eigenVectorImageIterator;
    }
}

template< class TInputImage, class TOutputImage >
void
AnisotropicCoherenceEnhancingDiffusionImageFilter<TInputImage, TOutputImage>
::PrintSelf( std::ostream& os, Indent indent ) const
{
  Superclass::PrintSelf( os, indent );

  os << indent << "Contrast parameter LambdaC: "
    << m_ContrastParameterLambdaC << std::endl;
  os << indent << "Sigma: " << m_Sigma << std::endl;
  os << indent << "SigmaOuter : " << m_SigmaOuter << std::endl;
  os << indent << "Alpha: " << m_Alpha << std::endl;
}

} // End namespace tube

} // End namespace itk

#endif
// End !defined(
//   __itktubeAnisotropicCoherenceEnhancingDiffusionImageFilter_hxx )
