
.. _program_listing_file_public_camera.h:

Program Listing for File camera.h
=================================

|exhale_lsh| :ref:`Return to documentation for file <file_public_camera.h>` (``public/camera.h``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   #ifndef f3d_camera_h
   #define f3d_camera_h
   
   #include "export.h"
   #include "types.h"
   
   #include <array>
   #include <string>
   
   namespace f3d
   {
   struct F3D_EXPORT camera_state_t
   {
     point3_t position = { 0., 0., 1. };
     point3_t focalPoint = { 0., 0., 0. };
     vector3_t viewUp = { 0., 1., 0. };
     angle_deg_t viewAngle = 30.;
   };
   
   class F3D_EXPORT camera
   {
   public:
   
     virtual camera& setPosition(const point3_t& pos) = 0;
     [[nodiscard]] virtual point3_t getPosition() = 0;
     virtual void getPosition(point3_t& pos) = 0;
     virtual camera& setFocalPoint(const point3_t& foc) = 0;
     [[nodiscard]] virtual point3_t getFocalPoint() = 0;
     virtual void getFocalPoint(point3_t& foc) = 0;
     virtual camera& setViewUp(const vector3_t& up) = 0;
     [[nodiscard]] virtual vector3_t getViewUp() = 0;
     virtual void getViewUp(vector3_t& up) = 0;
     virtual camera& setViewAngle(const angle_deg_t& angle) = 0;
     [[nodiscard]] virtual angle_deg_t getViewAngle() = 0;
     virtual void getViewAngle(angle_deg_t& angle) = 0;
     virtual camera& setState(const camera_state_t& state) = 0;
     [[nodiscard]] virtual camera_state_t getState() = 0;
     virtual void getState(camera_state_t& state) = 0;
   
   
   
     virtual camera& dolly(double val) = 0;
     virtual camera& pan(double right, double up, double forward = 0) = 0;
     virtual camera& zoom(double factor) = 0;
     virtual camera& roll(angle_deg_t angle) = 0;
     virtual camera& azimuth(angle_deg_t angle) = 0;
     virtual camera& yaw(angle_deg_t angle) = 0;
     virtual camera& elevation(angle_deg_t angle) = 0;
     virtual camera& pitch(angle_deg_t angle) = 0;
   
   
     virtual camera& setCurrentAsDefault() = 0;
   
     virtual camera& resetToDefault() = 0;
   
     virtual camera& resetToBounds(double zoomFactor = 0.9) = 0;
   
   protected:
     camera() = default;
     virtual ~camera() = default;
     camera(const camera&) = delete;
     camera(camera&&) = delete;
     camera& operator=(const camera&) = delete;
     camera& operator=(camera&&) = delete;
   };
   }
   
   #endif
