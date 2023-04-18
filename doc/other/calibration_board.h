/**
 * @page calibration_board What are Calibration Board Features?
 * @tableofcontents
 *
 * @section calibration_board_feature_points Feature Points
 *
 * The feature points on the chessboard calibration boards, are not the squares but
 * the internal corners of the squares. The following image shows a calibration board
 * with 10 column features and 7 row features:
 *
 * @image html calibration_board_features.png "Calibration Board Features" 
 *
 * @note It is critical that the corner squares diagonally across from each other are opposite colors! If they are identical the chessboard location algorithm could incorrectly rotate the board position. 
 * @section calibration_board_distances Feature Distances 
 *
 * The feature points have two distance parameters. One distance represents the physical
 * distance in 3D space of a printed board and the distance in pixels represents the distance
 * between points in a generated digital image of the board.  The following image shows the distance parameters:
 *
 * @image html calibration_board_feature_distances.png "Calibration Board Feature Distances"
 *
 * @section calibration_board_offsets Pixel Offsets 
 * The offset distances are used to provide a border around the chessboard. Projector calibration boards
 * to not need an offset. The following image shows the offset parameters:
 *
 * @image html calibration_board_offsets.png "Calibration Board Offsets"
 *
 */
