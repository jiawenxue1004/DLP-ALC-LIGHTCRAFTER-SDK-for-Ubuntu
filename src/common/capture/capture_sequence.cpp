/** @file   capture_sequence.cpp
 *  @brief  Contains methods to construct sequences of captured images from a camera
 *  @copyright 2014 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 */

#include <string>
#include <vector>
#include <iostream>

#include <common/debug.hpp>
#include <common/returncode.hpp>
#include <common/image/image.hpp>
#include <common/other.hpp>
#include <common/parameters.hpp>
#include <common/capture/capture.hpp>           // Adds dlp::Capture

namespace dlp{

/** @brief Default Constructor, calls the Clear() method*/
Capture::Sequence::Sequence(){
    this->Clear();
}

/** @brief Clears on Deconstruction */
Capture::Sequence::~Sequence(){
    this->Clear();
}

/** @brief Adds the first capture argument after construction */
Capture::Sequence::Sequence(const Capture &capture){
    this->Clear();
    this->Add(capture);
}

/** @brief Adds the entire \ref Sequence of captures after construction */
Capture::Sequence::Sequence(const Sequence &capture_seq){
    this->Clear();
    this->Add(capture_seq);
}

/** @brief Copies the supplied sequence */
Capture::Sequence& Capture::Sequence::operator=(const Sequence& capture_seq){
    this->Clear();
    this->Add(capture_seq);
    return *this;
}

/** @brief Return the number of captures
 *  @retval returns number of captures*/
unsigned int Capture::Sequence::GetCount() const{
    return this->captures_.size();
}

/** @brief Clears both captures and parameters */
void Capture::Sequence::Clear(){
    this->captures_.clear();
    this->parameters.Clear();
}

/** @brief Adds capture (if valid) to the internal vector of captures
 *  @retval CAPTURE_TYPE_INVALID the type of capture is invalid
 *  @retval FILE_DOES_NOT_EXIST that file can't be found
 *  @retval IMAGE_EMPTY no data in the image file
 */
ReturnCode Capture::Sequence::Add( const Capture &new_capture){
    ReturnCode ret;

    // Check that the capture type is valid
    if(new_capture.data_type == Capture::DataType::INVALID)
        return ret.AddError(CAPTURE_TYPE_INVALID);

    // Check the type to ensure the data, filename, or parameters are present
    switch(new_capture.data_type){
    case Capture::DataType::IMAGE_FILE:
        // Check that the file exists
        if(dlp::File::Exists(new_capture.image_file) == false)
            return ret.AddError(FILE_DOES_NOT_EXIST);
        break;
    case Capture::DataType::IMAGE_DATA:
        // Check that the image is not empty
        if(new_capture.image_data.isEmpty())
            return ret.AddError(IMAGE_EMPTY);
        break;
    case Capture::DataType::INVALID:
        return ret.AddError(CAPTURE_TYPE_INVALID);
    }

    // Add capture to the sequence
    this->captures_.push_back(new_capture);

    return ret;
}

/** @brief Add a sequence of captures to the internal vector of captures */
ReturnCode Capture::Sequence::Add( const Sequence &sequence){
    ReturnCode ret;
    for(unsigned int iCapture = 0; iCapture < sequence.GetCount(); iCapture++){
        Capture temp;

        // Get a capture
        ret = sequence.Get(iCapture,&temp);
        if(ret.hasErrors())
            return ret;

        // Save the capture
        ret = this->Add(temp);
        if(ret.hasErrors())
            return ret;
    }

    this->parameters = sequence.parameters;

    return ret;
}

/** @brief Assigns the capture of the specified index to the pointer ret_capture
 *  @param[in] index index of value you want returned from the sequence
 *  @retval ret_capture returns capture at index specified*/
ReturnCode Capture::Sequence::Get( const unsigned int &index, Capture* ret_capture) const{
    ReturnCode ret;

    // Check the index
    if( index >= this->captures_.size())
        return ret.AddError(CAPTURE_SEQUENCE_INDEX_OUT_OF_RANGE);

    // Return the capture
    (*ret_capture) = this->captures_.at(index);

    return ret;
}

/** @brief Assigns arg_capture to the specified index of the internal vector of captures
 *  @param[in] index index that you want to insert the capture into
 *  @param[in] arg_capture the capture to add to the sequence
 *  @retval CAPTURE_SEQUENCE_INDEX_OUT_OF_RANGE index out of range
 *  @retval CAPTURE_TYPE_INVALID image is not a supported type
 *  @retval FILE_DOES_NOT_EXIST file does not exist
 *  @retval IMAGE_EMPTY image contains no data
 *  @retval CAPTURE_TYPE_INVALID data type is not set initialized
 * */
ReturnCode Capture::Sequence::Set(const unsigned int &index, Capture &arg_capture){
    ReturnCode ret;

    // Check the index
    if( index >= this->captures_.size())
        return ret.AddError(CAPTURE_SEQUENCE_INDEX_OUT_OF_RANGE);

    // Check that the capture type is valid
    if(arg_capture.data_type == Capture::DataType::INVALID)
        return ret.AddError(CAPTURE_TYPE_INVALID);

    // Check the type to ensure the data, filename, or parameters are present
    switch(arg_capture.data_type){
    case Capture::DataType::IMAGE_FILE:
        // Check that the file exists
        if(dlp::File::Exists(arg_capture.image_file) == false)
            return ret.AddError(FILE_DOES_NOT_EXIST);
        break;
    case Capture::DataType::IMAGE_DATA:
        // Check that the image is not empty
        if(arg_capture.image_data.isEmpty())
            return ret.AddError(IMAGE_EMPTY);
        break;
    case Capture::DataType::INVALID:
        return ret.AddError(CAPTURE_TYPE_INVALID);
    }

    // Save the new capture settings to the sequence
    this->captures_.at(index) = arg_capture;

    return ret;
}

/** @brief Removes specified index from the internal vector of captures
 *  @param[in] index the index to remove a capture from
 *  @retval CAPTURE_SEQUENCE_INDEX_OUT_OF_RANGE index is not within range*/
ReturnCode Capture::Sequence::Remove(const unsigned int &index){
    ReturnCode ret;

    // Check the index
    if( index >= this->captures_.size())
        return ret.AddError(CAPTURE_SEQUENCE_INDEX_OUT_OF_RANGE);

    // Remove the capture
    this->captures_.erase(this->captures_.begin()+index);
    return ret;
}

/** @brief Returns true if all captures in sequence are set to the same data type
 *  @retval bool returns true of the data types are all equal in the sequence*/
bool Capture::Sequence::EqualDataTypes() const{
    bool ret = true;
    Capture::DataType prev_type;

    // Check that sequence has at least one capture
    if(this->captures_.size() > 0){
        // Get first value
        prev_type = this->captures_.at(0).data_type;

        for(unsigned int iPat = 0; (ret == true) && (iPat < this->captures_.size()); iPat++){
            if(this->captures_.at(iPat).data_type != prev_type)
                ret = false;
        }
    }
    return ret;
}
}
