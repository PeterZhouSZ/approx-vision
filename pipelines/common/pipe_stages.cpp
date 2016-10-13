
#include "pipe_stages.h"

using namespace Halide;
using namespace cv;

///////////////////////////////////////////////////////////////////////////////////////
// Conversion functions

Mat Image2Mat( Image<float> InImage ) {
  Mat OutMat(InImage.height(),InImage.width(),CV_32FC3);
  vector<Mat> three_channels;
  split(OutMat, three_channels);

  // Convert from planar RGB memory storage to interleaved BGR memory storage
  for (int y=0; y<InImage.height(); y++) {
    for (int x=0; x<InImage.width(); x++) {
      // Blue channel
      three_channels[0].at<float>(y,x) = InImage(x,y,2);
      // Green channel
      three_channels[1].at<float>(y,x) = InImage(x,y,1);
      // Red channel
      three_channels[2].at<float>(y,x) = InImage(x,y,0);
    }
  }

  merge(three_channels, OutMat);

  return OutMat;
}

Image<float> Mat2Image( Mat InMat ) {
  Image<float> OutImage(InMat.cols,InMat.rows,3);
  vector<Mat> three_channels;
  split(InMat, three_channels);

  // Convert from interleaved BGR memory storage to planar RGB memory storage
  for (int y=0; y<InMat.rows; y++) {
    for (int x=0; x<InMat.cols; x++) {
      // Blue channel
      OutImage(x,y,2) = three_channels[0].at<float>(y,x);
      // Green channel
      OutImage(x,y,1) = three_channels[1].at<float>(y,x);
      // Red channel
      OutImage(x,y,0) = three_channels[2].at<float>(y,x);
    }
  }

  return OutImage;
}

///////////////////////////////////////////////////////////////////////////////////////
// OpenCV Funcs for camera pipeline



///////////////////////////////////////////////////////////////////////////////////////
// Halide Funcs for camera pipeline

Func make_scale( Image<uint8_t> *in_img ) {
  Var x, y, c;
  // Cast input to float and scale from 8 bit 0-255 range to 0-1 range
  Func scale("scale");
    scale(x,y,c) = cast<float>( (*in_img)(x,y,c) ) / 256;
  return scale;
}

Func make_descale( Image<float> *in_func ) {
//Func make_descale( Func *in_func ) {
  Var x, y, c;
  // de-scale from 0-1 range to 0-255 range, and cast to 8 bit 
  Func descale("descale");
    descale(x,y,c) = cast<uint8_t>( (*in_func)(x,y,c) * 256 );
  return descale;
}

/*
// BACKWARD FUNCS /////////////////////////////////////////////////////////////////////

    // Backward tone mapping
    Func rev_tonemap("rev_tonemap");
      Expr rev_tone_idx = cast<uint8_t>(scale(x,y,c) * 256.0f);
      rev_tonemap(x,y,c) = rev_tone_h(c,rev_tone_idx) ;

    // Weighted radial basis function for gamut mapping
    Func rev_rbf_ctrl_pts("rev_rbf_ctrl_pts");
      // Initialization with all zero
      rev_rbf_ctrl_pts(x,y,c) = cast<float>(0);
      // Index to iterate with
      RDom revidx(0,num_ctrl_pts);
      // Loop code
      // Subtract the vectors 
      Expr revred_sub   = rev_tonemap(x,y,0) - ctrl_pts_h(0,revidx);
      Expr revgreen_sub = rev_tonemap(x,y,1) - ctrl_pts_h(1,revidx);
      Expr revblue_sub  = rev_tonemap(x,y,2) - ctrl_pts_h(2,revidx);
      // Take the L2 norm to get the distance
      Expr revdist      = sqrt( revred_sub*revred_sub + 
                                revgreen_sub*revgreen_sub + 
                                revblue_sub*revblue_sub );
      // Update persistant loop variables
      rev_rbf_ctrl_pts(x,y,c) = select( c == 0, rev_rbf_ctrl_pts(x,y,c) +
                                          (weights_h(0,revidx) * revdist),
                                        c == 1, rev_rbf_ctrl_pts(x,y,c) + 
                                          (weights_h(1,revidx) * revdist),
                                                rev_rbf_ctrl_pts(x,y,c) + 
                                          (weights_h(2,revidx) * revdist));

    // Add on the biases for the RBF
    Func rev_rbf_biases("rev_rbf_biases");
      rev_rbf_biases(x,y,c) = max( select( 
        c == 0, rev_rbf_ctrl_pts(x,y,0) + coefs[0][0] + coefs[1][0]*rev_tonemap(x,y,0) +
          coefs[2][0]*rev_tonemap(x,y,1) + coefs[3][0]*rev_tonemap(x,y,2),
        c == 1, rev_rbf_ctrl_pts(x,y,1) + coefs[0][1] + coefs[1][1]*rev_tonemap(x,y,0) +
          coefs[2][1]*rev_tonemap(x,y,1) + coefs[3][1]*rev_tonemap(x,y,2),
                rev_rbf_ctrl_pts(x,y,2) + coefs[0][2] + coefs[1][2]*rev_tonemap(x,y,0) +
          coefs[2][2]*rev_tonemap(x,y,1) + coefs[3][2]*rev_tonemap(x,y,2))
                              , 0);


    // Reverse color map and white balance transform
    Func rev_transform("rev_transform");
      rev_transform(x,y,c) = max( select(
        // Perform matrix multiplication, set min of 0
        c == 0, rev_rbf_biases(x,y,0)*TsTw_tran[0][0]
              + rev_rbf_biases(x,y,1)*TsTw_tran[1][0]
              + rev_rbf_biases(x,y,2)*TsTw_tran[2][0],
        c == 1, rev_rbf_biases(x,y,0)*TsTw_tran[0][1]
              + rev_rbf_biases(x,y,1)*TsTw_tran[1][1]
              + rev_rbf_biases(x,y,2)*TsTw_tran[2][1],
                rev_rbf_biases(x,y,0)*TsTw_tran[0][2]
              + rev_rbf_biases(x,y,1)*TsTw_tran[1][2]
              + rev_rbf_biases(x,y,2)*TsTw_tran[2][2])
                                                        , 0);
*/