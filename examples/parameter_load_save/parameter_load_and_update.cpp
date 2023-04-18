
#include <common/debug.hpp>
#include <common/error.hpp>
#include <common/parameter.hpp>

#include <iostream>
#include <string>


int main()
{
    dlp::ErrorCode ret_error;

    std::string param_name;
    std::string param_value;

    std::string param_filename_module_0 = "parameters_module_0.txt";
    std::string param_filename_module_1 = "parameters_module_1.txt";
    std::string param_filename_module_2 = "parameters_module_2.txt";
    std::string param_filename_system   = "parameters_system.txt";


    // Create Parameters object
    dlp::Parameters module_0;
    dlp::Parameters module_1;
    dlp::Parameters module_2;
    dlp::Parameters system;
    dlp::Parameters required_system_parameters;

    // Setup required system_parameters with default values
    ret_error = required_system_parameters.Set("MODULE_0_SETTING_0","FALSE");
    ret_error = required_system_parameters.Set("MODULE_0_SETTING_1","FALSE");
    ret_error = required_system_parameters.Set("MODULE_1_SETTING_0","FALSE");
    ret_error = required_system_parameters.Set("MODULE_1_SETTING_0","FALSE");
    ret_error = required_system_parameters.Set("MODULE_2_SETTING_0","FALSE");
    ret_error = required_system_parameters.Set("MODULE_2_SETTING_0","FALSE");






//    // Add parameters entries
//    ret_error = parameters.Set("COLOR_0","RED");
//    ret_error = parameters.Set("COLOR_1","GREEN");
//    ret_error = parameters.Set("COLOR_2","BLUE");
//    ret_error = parameters.Set("COLOR_3","WHITE");
//    ret_error = parameters.Set("COLOR_4","BLACK");

//    // Print all parameters
//    DLP_DEBUG(0,"Printing parameters..." );
//    for( int i = 0; i < parameters.GetCount(); i++){
//        ret_error = parameters.GetName(i,&param_name);          // Get parameter i's name
//        ret_error = parameters.Get(param_name, "default value", &param_value);   // Get param_name value
//        DLP_DEBUG(0,param_name << " = " << param_value );
//    }
//    DLP_DEBUG(0," ");

//    // Save the parameters to a file
//    ret_error = parameters.Save(param_filename);
//    DLP_DEBUG(0,"Saved parameters to "<< param_filename << " " << ret_error.ToString());
//    DLP_DEBUG(0," ");

//    // Clear the parameter object
//    DLP_DEBUG(0,"Parameters object has " << parameters.GetCount() << " entries.");
//    DLP_DEBUG(0,"Clearing parameters object...");
//    parameters.Clear();
//    DLP_DEBUG(0,"Parameters object has " << parameters.GetCount() << " entries.");
//    DLP_DEBUG(0," ");



//    // Load the example_parameters_file.txt into parameters object
//    DLP_DEBUG(0,"Loading "<< param_filename << " into parameters object..." << ret_error.ToString());
//    ret_error = parameters.Load(param_filename);
//    DLP_DEBUG(0," ");

//    // Print all parameters how they were stored as strings
//    DLP_DEBUG(0,"Printing the values loaded into parameters object..." );
//    for( int i = 0; i < parameters.GetCount(); i++){
//        ret_error = parameters.GetName(i,&param_name);          // Get parameter i's name
//        ret_error = parameters.Get(param_name, "default value", &param_value);   // Get param_name value
//        DLP_DEBUG(0,param_name << " = " << param_value );
//    }
//    DLP_DEBUG(0," ");


    return(0);
}

