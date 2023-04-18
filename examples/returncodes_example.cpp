/** @file       returncodes_example.cpp
 *  @brief      Demonstrates how to use dlp::ReturnCode objects
 *  @copyright  2016 Texas Instruments Incorporated - http://www.ti.com/ ALL RIGHTS RESERVED
 */

#include <dlp_sdk.hpp>
#include <string>
#include <vector>

// Use #define macros so that error and warning messages are consistent
#define EXAMPLE_ERROR_1     "EXAMPLE_ERROR_1"
#define EXAMPLE_ERROR_2     "EXAMPLE_ERROR_2"
#define EXAMPLE_ERROR_3     "EXAMPLE_ERROR_3"
#define EXAMPLE_WARNING_1   "EXAMPLE_WARNING_1"
#define EXAMPLE_WARNING_2   "EXAMPLE_WARNING_2"

int main(){
    // ReturnCode objects do NOT contain errors or warnings when initialized
    dlp::ReturnCode retval_a;
    dlp::ReturnCode retval_b;
    dlp::ReturnCode retval_c;


    // All errors and warnings within a ReturnCode can be output as a string
    // using the dlp::ReturnCode::ToString() method
    std::cout << "retval_a contains " << retval_a.ToString() << std::endl;
    std::cout << "retval_b contains " << retval_b.ToString() << std::endl;


    // Errors and warnings can be added as strings
    std::string error_msg   = "This is an error message!";
    std::string warning_msg = "This is just a warning...";
    retval_a.AddError(error_msg);
    retval_a.AddWarning(warning_msg);

    // #define macros help error and warning messages stay consistent
    retval_a.AddError(EXAMPLE_ERROR_2);
    retval_a.AddError(EXAMPLE_ERROR_3);
    retval_a.AddWarning(EXAMPLE_WARNING_2);

    // Entire ReturnCodes can be added as well
    retval_b.Add(retval_a);

    // Both ReturnCode objects have the same contents
    std::cout << std::endl;
    std::cout << "retval_a contains " << retval_a.ToString() << std::endl;
    std::cout << "retval_b contains " << retval_b.ToString() << std::endl;
    std::cout << "retval_c contains " << retval_c.ToString() << std::endl;

    // There are three methods to determine if a ReturnCode has errors
    std::cout << std::endl;
    if(retval_a)
        std::cout << "retval_a has errors!" << std::endl;

    if(retval_a.hasErrors())
        std::cout << "retval_a has errors!" << std::endl;

    if(retval_a.GetErrorCount() > 0)
        std::cout << "retval_a has "<< retval_a.GetErrorCount() << " errors!" << std::endl;

    // Warnings do NOT count as errors
    std::cout << std::endl;
    retval_c.AddWarning(EXAMPLE_WARNING_1);
    if(retval_c.hasErrors()){
        std::cout << "retval_c has errors!"    << std::endl;
    }
    else{
        std::cout << "retval_c has NO errors!" << std::endl;
    }

    // ReturnCodes can be checked for specific errors and warnings
    std::cout << std::endl;
    if(retval_a.ContainsError(EXAMPLE_ERROR_1))     std::cout << "retval_a contains " << EXAMPLE_ERROR_1   << std::endl;
    if(retval_a.ContainsError(EXAMPLE_ERROR_2))     std::cout << "retval_a contains " << EXAMPLE_ERROR_2   << std::endl;
    if(retval_a.ContainsError(EXAMPLE_ERROR_3))     std::cout << "retval_a contains " << EXAMPLE_ERROR_3   << std::endl;
    if(retval_a.ContainsWarning(EXAMPLE_WARNING_1)) std::cout << "retval_a contains " << EXAMPLE_WARNING_1 << std::endl;
    if(retval_a.ContainsWarning(EXAMPLE_WARNING_2)) std::cout << "retval_a contains " << EXAMPLE_WARNING_2 << std::endl;

    // ReturnCodes can be cleared
    retval_c.Clear();
    std::cout << std::endl;
    std::cout << "After clearing retval_c contains " << retval_c.ToString() << std::endl;

    // Error and warning messages can be returned as a vector of strings
    std::vector<std::string> error_msg_list;
    std::vector<std::string> warning_msg_list;
    error_msg_list   = retval_a.GetErrors();
    warning_msg_list = retval_a.GetWarnings();

    return 0;
}

