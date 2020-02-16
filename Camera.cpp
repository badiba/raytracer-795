#include "Camera.h"
#include <string.h>

Camera::Camera(int id,                      // Id of the camera
               const char* imageName,       // Name of the output PPM file
               const Vector3f& pos,         // Camera position
               const Vector3f& gaze,        // Camera gaze direction
               const Vector3f& up,          // Camera up direction
               const ImagePlane& imgPlane)  // Image plane parameters
{
    /***********************************************
     *                                             *
     * TODO: Implement this function               *
     *                                             *
     ***********************************************
     */

    this -> id = id;
    this -> imgPlane = imgPlane;
    strcpy(this->imageName, imageName);
    this -> gaze = gaze;
    this -> pos = pos;
    this -> up = up;
    nxDivisionAvoided = (float) 1 / imgPlane.nx;
    nyDivisionAvoided = (float) 1 / imgPlane.ny;
    right = up.cross(gaze);
}

/* Takes coordinate of an image pixel as row and col, and
 * returns the ray going through that pixel.
 */
Ray Camera::getPrimaryRay(int col, int row) const
{
    /***********************************************
     *                                             *
     * TODO: Implement this function               *
     *                                             *
     ***********************************************
     */

    float u = imgPlane.left + (imgPlane.right - imgPlane.left) * (col + 0.5f) * nxDivisionAvoided;
    float v = imgPlane.top - (imgPlane.top - imgPlane.bottom) * (row + 0.5f) * nyDivisionAvoided;
    Vector3f m = pos + gaze*imgPlane.distance;
    m(0) += u;
    m(1) += v;
    Ray ray(pos, (m-pos)/((m-pos).norm()));
    return ray;

}
