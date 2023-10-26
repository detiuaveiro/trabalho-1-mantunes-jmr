/// image8bit - A simple image processing module.
///
/// This module is part of a programming project
/// for the course AED, DETI / UA.PT
///
/// You may freely use and modify this code, at your own risk,
/// as long as you give proper credit to the original and subsequent authors.
///
/// João Manuel Rodrigues <jmr@ua.pt>
/// 2013, 2023

// Student authors (fill in below):
// NMec:  Name:
// 
// 
// 
// Date:
//

#include "image8bit.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "instrumentation.h"

// The data structure
//
// An image is stored in a structure containing 3 fields:
// Two integers store the image width and height.
// The other field is a pointer to an array that stores the 8-bit gray
// level of each pixel in the image.  The pixel array is one-dimensional
// and corresponds to a "raster scan" of the image from left to right,
// top to bottom.
// For example, in a 100-pixel wide image (img->width == 100),
//   pixel position (x,y) = (33,0) is stored in img->pixel[33];
//   pixel position (x,y) = (22,1) is stored in img->pixel[122].
// 
// Clients should use images only through variables of type Image,
// which are pointers to the image structure, and should not access the
// structure fields directly.

// Maximum value you can store in a pixel (maximum maxval accepted)
const uint8 PixMax = 255;

// Internal structure for storing 8-bit graymap images
struct image {
  int width;
  int height;
  int maxval;   // maximum gray value (pixels with maxval are pure WHITE)
  uint8* pixel; // pixel data (a raster scan)
};


// This module follows "design-by-contract" principles.
// Read `Design-by-Contract.md` for more details.

/// Error handling functions

// In this module, only functions dealing with memory allocation or file
// (I/O) operations use defensive techniques.
// 
// When one of these functions fails, it signals this by returning an error
// value such as NULL or 0 (see function documentation), and sets an internal
// variable (errCause) to a string indicating the failure cause.
// The errno global variable thoroughly used in the standard library is
// carefully preserved and propagated, and clients can use it together with
// the ImageErrMsg() function to produce informative error messages.
// The use of the GNU standard library error() function is recommended for
// this purpose.
//
// Additional information:  man 3 errno;  man 3 error;

// Variable to preserve errno temporarily
static int errsave = 0;

// Error cause
static char* errCause;

/// Error cause.
/// After some other module function fails (and returns an error code),
/// calling this function retrieves an appropriate message describing the
/// failure cause.  This may be used together with global variable errno
/// to produce informative error messages (using error(), for instance).
///
/// After a successful operation, the result is not garanteed (it might be
/// the previous error cause).  It is not meant to be used in that situation!
char* ImageErrMsg() { ///
  return errCause;
}


// Defensive programming aids
//
// Proper defensive programming in C, which lacks an exception mechanism,
// generally leads to possibly long chains of function calls, error checking,
// cleanup code, and return statements:
//   if ( funA(x) == errorA ) { return errorX; }
//   if ( funB(x) == errorB ) { cleanupForA(); return errorY; }
//   if ( funC(x) == errorC ) { cleanupForB(); cleanupForA(); return errorZ; }
//
// Understanding such chains is difficult, and writing them is boring, messy
// and error-prone.  Programmers tend to overlook the intricate details,
// and end up producing unsafe and sometimes incorrect programs.
//
// In this module, we try to deal with these chains using a somewhat
// unorthodox technique.  It resorts to a very simple internal function
// (check) that is used to wrap the function calls and error tests, and chain
// them into a long Boolean expression that reflects the success of the entire
// operation:
//   success = 
//   check( funA(x) != error , "MsgFailA" ) &&
//   check( funB(x) != error , "MsgFailB" ) &&
//   check( funC(x) != error , "MsgFailC" ) ;
//   if (!success) {
//     conditionalCleanupCode();
//   }
//   return success;
// 
// When a function fails, the chain is interrupted, thanks to the
// short-circuit && operator, and execution jumps to the cleanup code.
// Meanwhile, check() set errCause to an appropriate message.
// 
// This technique has some legibility issues and is not always applicable,
// but it is quite concise, and concentrates cleanup code in a single place.
// 
// See example utilization in ImageLoad and ImageSave.
//
// (You are not required to use this in your code!)


// Check a condition and set errCause to failmsg in case of failure.
// This may be used to chain a sequence of operations and verify its success.
// Propagates the condition.
// Preserves global errno!
static int check(int condition, const char* failmsg) {
  errCause = (char*)(condition ? "" : failmsg);
  return condition;
}


/// Init Image library.  (Call once!)
/// Currently, simply calibrate instrumentation and set names of counters.
void ImageInit(void) { ///
  InstrCalibrate();
  InstrName[0] = "pixmem";  // InstrCount[0] will count pixel array acesses
  // Name other counters here...
  //HIDE
  InstrName[1] = "pixops";  // InstrCount[1] will count pixel adds/subs/compares
  //SHOW
  
}

// Macros to simplify accessing instrumentation counters:
#define PIXMEM InstrCount[0]
// Add more macros here...
//HIDE
#define PIXOPS InstrCount[1]
//SHOW

// TIP: Search for PIXMEM or InstrCount to see where it is incremented!


/// Image management functions

/// Create a new black image.
///   width, height : the dimensions of the new image.
///   maxval: the maximum gray level (corresponding to white).
/// Requires: width and height must be non-negative, maxval > 0.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageCreate(int width, int height, uint8 maxval) { ///
  assert (width >= 0);
  assert (height >= 0);
  assert (0 < maxval && maxval <= PixMax);
  // Insert your code here!
  //HIDE
  Image img = NULL;
  int success =
  check( (img = (Image)calloc(1, sizeof(*img))) != NULL, "Alloc image failed" ) &&
  check( (img->pixel = (uint8*)calloc(width*height, sizeof(uint8))) != NULL, "Alloc pixels failed" );

  if (success) {
    img->width = width;
    img->height = height;
    img->maxval = maxval;
  } else {
    errsave = errno;
    free(img);
    img = NULL;
    errno = errsave;
  }
  return img;
  //SHOW
}

/// Destroy the image pointed to by (*imgp).
///   imgp : address of an Image variable.
/// If (*imgp)==NULL, no operation is performed.
/// Ensures: (*imgp)==NULL.
/// Should never fail, and should preserve global errno/errCause.
void ImageDestroy(Image* imgp) { ///
  assert (imgp != NULL);
  // Insert your code here!
  //HIDE
  Image img = *imgp;
  if (img != NULL) { free(img->pixel); }
  free(img);
  *imgp = NULL;
  //SHOW
}


/// PGM file operations

// See also:
// PGM format specification: http://netpbm.sourceforge.net/doc/pgm.html

// Match and skip 0 or more comment lines in file f.
// Comments start with a # and continue until the end-of-line, inclusive.
// Returns the number of comments skipped.
static int skipComments(FILE* f) {
  char c;
  int i = 0;
  while (fscanf(f, "#%*[^\n]%c", &c) == 1 && c == '\n') {
    i++;
  }
  return i;
}

/// Load a raw PGM file.
/// Only 8 bit PGM files are accepted.
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageLoad(const char* filename) { ///
  int w, h;
  int maxval;
  char c;
  FILE* f = NULL;
  Image img = NULL;

  int success = 
  check( (f = fopen(filename, "rb")) != NULL, "Open failed" ) &&
  // Parse PGM header
  check( fscanf(f, "P%c ", &c) == 1 && c == '5' , "Invalid file format" ) &&
  skipComments(f) >= 0 &&
  check( fscanf(f, "%d ", &w) == 1 && w >= 0 , "Invalid width" ) &&
  skipComments(f) >= 0 &&
  check( fscanf(f, "%d ", &h) == 1 && h >= 0 , "Invalid height" ) &&
  skipComments(f) >= 0 &&
  check( fscanf(f, "%d", &maxval) == 1 && 0 < maxval && maxval <= (int)PixMax , "Invalid maxval" ) &&
  check( fscanf(f, "%c", &c) == 1 && isspace(c) , "Whitespace expected" ) &&
  // Allocate image
  (img = ImageCreate(w, h, (uint8)maxval)) != NULL &&
  // Read pixels
  check( fread(img->pixel, sizeof(uint8), w*h, f) == w*h , "Reading pixels" );
  PIXMEM += (unsigned long)(w*h);  // count pixel memory accesses

  // Cleanup
  if (!success) {
    errsave = errno;
    ImageDestroy(&img);
    errno = errsave;
  }
  if (f != NULL) fclose(f);
  return img;
}

/// Save image to PGM file.
/// On success, returns nonzero.
/// On failure, returns 0, errno/errCause are set appropriately, and
/// a partial and invalid file may be left in the system.
int ImageSave(Image img, const char* filename) { ///
  assert (img != NULL);
  int w = img->width;
  int h = img->height;
  uint8 maxval = img->maxval;
  FILE* f = NULL;

  int success =
  check( (f = fopen(filename, "wb")) != NULL, "Open failed" ) &&
  check( fprintf(f, "P5\n%d %d\n%u\n", w, h, maxval) > 0, "Writing header failed" ) &&
  check( fwrite(img->pixel, sizeof(uint8), w*h, f) == w*h, "Writing pixels failed" ); 
  PIXMEM += (unsigned long)(w*h);  // count pixel memory accesses

  // Cleanup
  if (f != NULL) fclose(f);
  return success;
}


/// Information queries

/// These functions do not modify the image and never fail.

/// Get image width
int ImageWidth(Image img) { ///
  assert (img != NULL);
  return img->width;
}

/// Get image height
int ImageHeight(Image img) { ///
  assert (img != NULL);
  return img->height;
}

/// Get image maximum gray level
int ImageMaxval(Image img) { ///
  assert (img != NULL);
  return img->maxval;
}

/// Pixel stats
/// Find the minimum and maximum gray levels in image.
/// On return,
/// *min is set to the minimum gray level in the image,
/// *max is set to the maximum.
void ImageStats(Image img, uint8* min, uint8* max) { ///
  assert (img != NULL);
  // Insert your code here!
  //HIDE
  *min = PixMax;  // maxval would mask overflows!
  *max = 0;
  uint8 p;
  for (int k = 0; k < img->width*img->height; k++) {
    p = img->pixel[k];
    if (p < *min) *min = p;
    if (p > *max) *max = p;
  }
  //SHOW
}

/// Check if pixel position (x,y) is inside img.
int ImageValidPos(Image img, int x, int y) { ///
  assert (img != NULL);
  return (0 <= x && x < img->width) && (0 <= y && y < img->height);
}

/// Check if rectangular area (x,y,w,h) is completely inside img.
int ImageValidRect(Image img, int x, int y, int w, int h) { ///
  assert (img != NULL);
  // Insert your code here!
  //HIDE
  return ImageValidPos(img, x, y) && ImageValidPos(img, x+w-1, y+h-1);
  //SHOW
}

/// Pixel get & set operations

/// These are the primitive operations to access and modify a single pixel
/// in the image.
/// These are very simple, but fundamental operations, which may be used to 
/// implement more complex operations.

// Transform (x, y) coords into linear pixel index.
// This internal function is used in ImageGetPixel / ImageSetPixel. 
// The returned index must satisfy (0 <= index < img->width*img->height)
static inline int G(Image img, int x, int y) {
  int index;
  // Insert your code here!
  //HIDE
  //x = mod(x, img->width);
  //y = mod(y, img->height);
  index = x + img->width*y;
  //SHOW
  assert (0 <= index && index < img->width*img->height);
  return index;
}

/// Get the pixel (level) at position (x,y).
uint8 ImageGetPixel(Image img, int x, int y) { ///
  assert (img != NULL);
  assert (ImageValidPos(img, x, y));
  PIXMEM += 1;  // count one pixel access (read)
  return img->pixel[G(img, x, y)];
} 

/// Set the pixel at position (x,y) to new level.
void ImageSetPixel(Image img, int x, int y, uint8 level) { ///
  assert (img != NULL);
  assert (ImageValidPos(img, x, y));
  PIXMEM += 1;  // count one pixel access (store)
  img->pixel[G(img, x, y)] = level;
} 


/// Pixel transformations

/// These functions modify the pixel levels in an image, but do not change
/// pixel positions or image geometry in any way.
/// All of these functions modify the image in-place: no allocation involved.
/// They never fail.

//HIDE
// These are internal functions to create and manipulate pixel maps.
// A pixel map is a uint[256] array that defines a pixel to pixel mapping,
// that may then be applied to an image.

// Init pixel map with identity function.
static void PixMapInit(uint8* map) {
  int p;
  for (p = 0; p <= PixMax; p++) {
    map[p] = (uint8)p;
  }
}

// Apply negative transform to map
static void PixMapNegative(uint8* map, uint8 maxval) {
  int p;
  for (p = 0; p <= PixMax; p++) {
    int v = maxval - map[p];
    map[p] = (uint8)(v >= 0 ? v : 0);  // avoid underflows for pixels > maxval!
  }
}

// Apply step function (0 if p<thr else tread)
static void PixMapThreshold(uint8* map, uint8 thr, uint8 tread) {
  int p;
  for (p = 0; p <= PixMax; p++) {
    map[p] = (uint8)(map[p] < thr ? 0 : tread);
  }
}

// Clamp (and round) pixel value to range [0, maxval].
static uint8 clamp(double p, uint8 maxval) {
  return (p < 0.0 ? (uint8)0 : (p < maxval+1.0 ? (uint8)(p+0.5) : maxval));
}

// Apply affine transform
static void PixMapAffine(uint8* map, double m, double b, uint8 maxval) {
  int p;
  for (p = 0; p <= PixMax; p++) {
    map[p] = clamp(m * map[p] + b, maxval);
  }
}

// In-place apply mapping to image pixels.
static void ImageMap(Image img, uint8* map) {
  assert (img != NULL);
  for (int k = 0; k < img->width*img->height; k++) {
    PIXMEM += 2;
    img->pixel[k] = map[img->pixel[k]];
  }
}
//SHOW

/// Transform image to negative image.
/// This transforms dark pixels to light pixels and vice-versa,
/// resulting in a "photographic negative" effect.
void ImageNegative(Image img) { ///
  assert (img != NULL);
  // Insert your code here!
  //HIDE
  uint8 map[1+PixMax];
  PixMapInit(map);
  PixMapNegative(map, img->maxval);
  ImageMap(img, map);
  //SHOW
}

/// Apply threshold to image.
/// Transform all pixels with level<thr to black (0) and
/// all pixels with level>=thr to white (maxval).
void ImageThreshold(Image img, uint8 thr) { ///
  assert (img != NULL);
  // Insert your code here!
  //HIDE
  uint8 map[1+PixMax];
  PixMapInit(map);
  PixMapThreshold(map, thr, img->maxval);
  ImageMap(img, map);
  //SHOW
}

/// Brighten image by a factor.
/// Multiply each pixel level by a factor, but saturate at maxval.
/// This will brighten the image if factor>1.0 and
/// darken the image if factor<1.0.
void ImageBrighten(Image img, double factor) { ///
  assert (img != NULL);
  // ? assert (factor >= 0.0);
  // Insert your code here!
  //HIDE
  uint8 map[1+PixMax];
  PixMapInit(map);
  PixMapAffine(map, factor, 0.0, img->maxval);
  ImageMap(img, map);
  //SHOW
}


/// Geometric transformations

/// These functions apply geometric transformations to an image,
/// returning a new image as a result.
/// 
/// Success and failure are treated as in ImageCreate:
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.

// Implementation hint: 
// Call ImageCreate whenever you need a new image!

/// Rotate an image.
/// Returns a rotated version of the image.
/// The rotation is 90 degrees clockwise.
/// Ensures: The original img is not modified.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageRotate(Image img) { ///
  assert (img != NULL);
  // Insert your code here!
  //HIDE
  int w = img->height;
  int h = img->width;
  Image img2 = ImageCreate(w, h, img->maxval);
  if (img2 == NULL) return NULL;
  int i, j;
  for (i = 0; i < w; i++) {
    for (j = 0; j < h; j++) {
      // apply transform:
      // x = r[0][0]*i + r[0][1]*j + t[0];
      // y = r[1][0]*i + r[1][1]*j + t[1];
      ImageSetPixel(img2, i, j, ImageGetPixel(img, h-1-j, i));
    }
  }
  return img2;
  //SHOW
}

/// Mirror an image = flip left-right.
/// Returns a mirrored version of the image.
/// Ensures: The original img is not modified.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageMirror(Image img) { ///
  assert (img != NULL);
  // Insert your code here!
  //HIDE
  // This could be done in-place too!
  int w = img->width;
  int h = img->height;
  Image img2 = ImageCreate(w, h, img->maxval);
  if (img2 == NULL) return NULL;
  int i, j;
  for (i = 0; i < w; i++) {
    for (j = 0; j < h; j++) {
      // apply transform:
      // x = r[0][0]*i + r[0][1]*j + t[0];
      // y = r[1][0]*i + r[1][1]*j + t[1];
      ImageSetPixel(img2, i, j, ImageGetPixel(img, w-1-i, j));
    }
  }
  return img2;
  //SHOW
}

/// Crop a rectangular subimage from img.
/// The rectangle is specified by the top left corner coords (x, y) and
/// width w and height h.
/// Requires:
///   The rectangle must be inside the original image.
/// Ensures:
///   The original img is not modified.
///   The returned image has width w and height h.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageCrop(Image img, int x, int y, int w, int h) { ///
  assert (img != NULL);
  assert (ImageValidRect(img, x, y, w, h));
  // Insert your code here!
  //HIDE
  Image img2 = ImageCreate(w, h, img->maxval);
  if (img2 == NULL) return NULL;
  int i, j;
  for (i = 0; i < w; i++) {
    for (j = 0; j < h; j++) {
      ImageSetPixel(img2, i, j, ImageGetPixel(img, x+i, y+j));
    }
  }
  return img2;
  //SHOW
}


/// Operations on two images

/// Paste an image into a larger image.
/// Paste img2 into position (x, y) of img1.
/// This modifies img1 in-place: no allocation involved.
/// Requires: img2 must fit inside img1 at position (x, y).
void ImagePaste(Image img1, int x, int y, Image img2) { ///
  assert (img1 != NULL);
  assert (img2 != NULL);
  assert (ImageValidRect(img1, x, y, img2->width, img2->height));
  // Insert your code here!
  //HIDE
  int w = img2->width;
  int h = img2->height;
  int i, j;
  for (i = 0; i < w; i++) {
    for (j = 0; j < h; j++) {
      ImageSetPixel(img1, x+i, y+j, ImageGetPixel(img2, i, j));
    }
  }
  //SHOW
}

/// Blend an image into a larger image.
/// Blend img2 into position (x, y) of img1.
/// This modifies img1 in-place: no allocation involved.
/// Requires: img2 must fit inside img1 at position (x, y).
/// alpha usually is in [0.0, 1.0], but values outside that interval
/// may provide interesting effects.  Over/underflows should saturate.
void ImageBlend(Image img1, int x, int y, Image img2, double alpha) { ///
  assert (img1 != NULL);
  assert (img2 != NULL);
  assert (ImageValidRect(img1, x, y, img2->width, img2->height));
  // Insert your code here!
  //HIDE
  int w = img2->width;
  int h = img2->height;
  int i, j;
  // scale factor to map img2 maxval to img1 maxval
  double scale = (double)img1->maxval / (double)img2->maxval;
  double a = alpha * scale;
  double b = (1 - alpha);
  for (i = 0; i < w; i++) {
    for (j = 0; j < h; j++) {
      uint8 p1 = ImageGetPixel(img1, x+i, y+j);
      uint8 p2 = ImageGetPixel(img2, i, j);
      // using 2 LUTs would be faster!
      // PIXMEM already counted by Get and Set!
      PIXOPS += 3;  // 2 mults + 1 add
      uint8 p0 = clamp(b * (double)p1 + a * (double)p2, img1->maxval);
      ImageSetPixel(img1, x+i, y+j, p0);
    }
  }
  //SHOW
}

/// Compare an image to a subimage of a larger image.
/// Returns 1 (true) if img2 matches subimage of img1 at pos (x, y).
/// Returns 0, otherwise.
int ImageMatchSubImage(Image img1, int x, int y, Image img2) { ///
  assert (img1 != NULL);
  assert (img2 != NULL);
  assert (ImageValidPos(img1, x, y));
  // Insert your code here!
  //HIDE
  if (!ImageValidPos(img1, x+img2->width-1, y+img2->height-1)) {
    return 0;
  }
  int i, j;
  for (i = 0; i < img2->width; i++) {
    for (j = 0; j < img2->height; j++) {
      PIXOPS += 1;  // 1 comparison
      if (ImageGetPixel(img1, x+i, y+j) != ImageGetPixel(img2, i, j)) {
        return 0;
      }
    }
  }
  return 1;
  //SHOW
}

/// Locate a subimage inside another image.
/// Searches for img2 inside img1.
/// If a match is found, returns 1 and matching position is set in vars (*px, *py).
/// If no match is found, returns 0 and (*px, *py) are left untouched.
int ImageLocateSubImage(Image img1, int* px, int* py, Image img2) { ///
  assert (img1 != NULL);
  assert (img2 != NULL);
  // Insert your code here!
  //HIDE
  int x, y;
  for (x = 0; x <= img1->width-img2->width; x++) {
    for (y = 0; y <= img1->height-img2->height; y++) {
      if (ImageMatchSubImage(img1, x, y, img2)) {
        *px = x;
        *py = y;
        return 1;
      }
    }
  }
  return 0;
  //SHOW
}


/// Filtering

//HIDE
static inline int max(int a, int b) { return a >= b ? a : b; }
static inline int min(int a, int b) { return a <= b ? a : b; }
//SHOW
/// Blur an image by a applying a (2dx+1)x(2dy+1) mean filter.
/// Each pixel is substituted by the mean of the pixels in the rectangle
/// [x-dx, x+dx]x[y-dy, y+dy].
/// The image is changed in-place.
void ImageBlur(Image img, int dx, int dy) { ///
  // Insert your code here!
  //HIDE
  // Allocate array for cummulative sums
  int w = img->width;
  int h = img->height;
  uint32_t* cumsum = (uint32_t*)calloc(w*h, sizeof(*cumsum));
  // check(cumsum != NULL, "Out of memory");
  
  // Compute cumsums
  int k = 0;
  //printf("----\n");
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      assert( k == G(img, x, y) );
      PIXMEM += 4; PIXOPS += 3;
      cumsum[k] =
          (uint32_t)img->pixel[k]
          + (y > 0 ? cumsum[k-w] : 0u)
          + (x > 0 ? cumsum[k-1] : 0u)
          - (x > 0 && y >0 ? cumsum[k-w-1] : 0u);
      //printf("%7ud\t", cumsum[k]);
      k++;
    }
    //printf("\n");
  }
  //printf("----\n");

  // Compute diffs
  k = 0;
  for (int y = 0; y < h; y++) {
    // BUG FIX:
    //   y1 = -1 is allowed, with cumsum[x,-1] = 0
    //   x1 = -1 is allowed, with cumsum[-1,y] = 0.
    // TODO: redefine cumsum to be (w+1)x(h+1) and avoid conditionals.
    int y1 = max(y-dy-1, -1);
    int y2 = min(y+dy, h-1);
    for (int x = 0; x < w; x++) {
      assert( k == G(img, x, y) );
      int x1 = max(x-dx-1, -1);
      int x2 = min(x+dx, w-1);
      PIXMEM += 4; PIXOPS += 3;
      uint32_t diff =
          cumsum[G(img, x2, y2)] 
          - (x1 >= 0 ? cumsum[G(img, x1, y2)] : 0u)
          - (y1 >= 0 ? cumsum[G(img, x2, y1)] : 0u)
          + (x1 >= 0 && y1 >= 0 ? cumsum[G(img, x1, y1)] : 0u);
      uint32_t area = (x2-x1)*(y2-y1);
      PIXMEM += 1; PIXOPS += 3;
      img->pixel[k] = (uint8)((2*diff + area) / (2*area));  // round
      //printf("pixel(%d,%d) = %u / %u ~ %hhu\n", x, y, diff, area, img->pixel[k]);
      k++;
    }
  }

  free(cumsum);
  //SHOW
}

//HIDE
/* GARBAGE

// The 4 rotation matrices:
static int R[4][2][2] = {
  { { 1,  0}, { 0,  1} },
  { { 0, -1}, { 1,  0} },
  { {-1,  0}, { 0, -1} },
  { { 0,  1}, {-1,  0} },
};

// Compute x mod w (positive remainder). Assumes w>0;
static int mod(int x, int w) {
  return x % w + ((x<0) ? w : 0);
}

/// Scroll an image.
/// Displace the origin of an image to (x,y).
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageScroll(Image img, int x, int y) { ///
  assert (img != NULL);
  assert (ImageValidPos(img, x, y));
  // Insert your code here!
  // #HIDE
  int w = img->width;
  int h = img->height;
  Image img2 = ImageCreate(w, h, img->maxval);
  if (img2 == NULL) return NULL;
  for (int i = 0; i < w; i++) {
    int i0 = mod(x+i, w);
    for (int j = 0; j < h; j++) {
      ImageSetPixel(img2, i, j, ImageGetPixel(img, i0, mod(y+j, h)));
    }
  }
  return img2;
  // #SHOW
}

Image ImageClone(Image img);

FlipUD();

Compose(a, b, x,y)

StitchLR()
*/
//SHOW
