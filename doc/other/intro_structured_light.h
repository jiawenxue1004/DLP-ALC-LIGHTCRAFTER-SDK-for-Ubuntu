/**
 * @page intro_structured_light Introduction to 3D machine vision
 * @tableofcontents
 * @section intro_structured_light_introduction Introduction
 *
 * Structured light is a method to achieve 3-dimensional photography of objects by manipulating lighting
 * conditions of a scene under study. Typical structured light systems project a series of time-multiplexed
 * patterns to extract spatial data from a scene. DLP Technology is well suited for structured light primarily
 * because of the highly-programmable DLP digital micromirror device which can rapidly display patterns to reduce
 * data acquisition times. DLP Technology is also light source agnostic, making it well suited for structured light
 * applications using solid-state – near infrared, visible, or ultraviolet light – or laser-based illumination.
 *
 * @section intro_structured_light_triangulation Using Triangulation to Calculate Depth
 *
 * Imagine two windows in space each with a known viewpoint looking out through their windows. If an identified feature
 * in space is visible by both viewpoints and the locations of the viewpoints are known, it is possible to
 * calculate where that feature is in space using trigonometry.
 *
 * @image html triangulation.png "Determining depth with known distances and angles"
 *
 * @section intro_structured_light_cameras Using Stereo Vision to Calculate Depth
 *
 * Taking a step beyond two windows, its simple to imagine that each viewpoint is instead a camera. The challenge
 * for stereo vision 3D systems is finding common features from both viewpoints. Feature identification is processor
 * intensive and prone to error especially is surfaces are nondescript.
 *
 * @image html triangulation_camera.png "Stereo vision 3D measurement"
 *
 * @section intro_structured_light_projector Using Structured Light to Calculate Depth
 *
 * Structured light techniques for 3D machine vision calculate depth with the exact same principles of stereo vision
 * systems, but instead substituting one of the cameras with a projector. This allows the system to project easily
 * identifiable features, rather than actively searching for them. The following image demonstrates this point by
 * showing the projector outputting a rainbow. The projected pattern appears on the surface and is easily identified
 * by the camera.
 *
 * @image html triangulation_projector.png "Structured light 3D measurement"
 *
 * @section intro_structured_light_subsystems Subsystems Needed for 3D Machine Vision with Structured Light
 *
 * For a complete 3D Machine Vision scanner, several subsystems are needed.
 *
 * - Calibration (\ref dlp::Calibration)
 *   - Determine physical locations and directions
 *   - Determine optical parameters of camera and projector
 *     - Focal length
 *     - Focal point
 *     - Radial distortion coefficients
 * - Geometry (\ref dlp::Geometry)
 *   - Uses calibration data and feature identification to reconstruct (x,y,z) point
 * - Structured Light (\ref dlp::StructuredLight)
 *   - Generate structured light patterns
 *   - Decode captured images and generate disparity map which details which projector pixels are viewed by camera
 * - DLP Platform (\ref dlp::DLP_Platform)
 *   - Project high-speed patterns
 * - Camera (\ref dlp::Camera)
 *   - Capture high-speed patterns for analysis
 *
 * 
 * @section intro_structured_light_links Helpful Links
 *
 * The following links may be helpful to better understand 3D machine vision and structured light:
 * - http://mesh.brown.edu/byo3d/
 * - https://sites.google.com/site/structuredlight/
 * - http://www.instructables.com/id/Structured-Light-3D-Scanning/
 * - https://www.cs.purdue.edu/homes/aliaga/cs635-10/lec-structured-light.pdf
 * - http://www.embedded.com/design/real-world-applications/4411991/2/Using-3D-sensors-to-bring-depth-discernment-to-embedded-vision-apps
 * - http://www.sci.utah.edu/~gerig/CS6320-S2012/Materials/CS6320-CV-S2012-StructuredLight.pdf
 * - http://www.opticsinfobase.org/aop/abstract.cfm?uri=aop-3-2-128
 *
 * @warning The links above are not made available from TI, representative of TI, nor are they guaranteed by TI.
 * 
 */
