
#include <common/debug.hpp>
#include <common/returncode.hpp>
#include <common/parameters.hpp>

#include <iostream>
#include <fstream>
#include <string>

enum class Colors{
    BLACK,
    RED,
    GREEN,
    BLUE,
    CYAN,
    YELLOW,
    MAGENTA,
    WHITE
};

DLP_NEW_PARAMETERS_ENTRY(Color1,"COLOR_1",Colors,Colors::BLACK);
DLP_NEW_PARAMETERS_ENTRY(Color2,"COLOR_2",Colors,Colors::BLACK);
DLP_NEW_PARAMETERS_ENTRY(Color3,"COLOR_3",Colors,Colors::BLACK);
DLP_NEW_PARAMETERS_ENTRY(Color4,"COLOR_4",Colors,Colors::BLACK);
DLP_NEW_PARAMETERS_ENTRY(Intensity1,"INTENSITY_1", int, 0);
DLP_NEW_PARAMETERS_ENTRY(Intensity2,"INTENSITY_2", int, 0);

int main()
{
    dlp::ReturnCode ret_error;

    std::string param_name;
    std::string param_value;
    std::string param_filename = "color_parameters.txt";

    // Create Parameters object
    dlp::Parameters parameters;

    // Add parameters entries
    ret_error = parameters.Set("COLOR_0","RED");
    ret_error = parameters.Set("COLOR_1","GREEN");
    ret_error = parameters.Set("COLOR_2","BLUE");
    ret_error = parameters.Set("COLOR_3","WHITE");
    ret_error = parameters.Set("COLOR_4","BLACK");

    // Print all parameters
    DLP_DEBUG(0,"Printing parameters..." );
    for( int i = 0; i < parameters.GetCount(); i++){
        ret_error = parameters.GetName(i,&param_name);          // Get parameter i's name
        ret_error = parameters.Get(param_name, "default value", &param_value);   // Get param_name value
        DLP_DEBUG(0,param_name << " = " << param_value );
    }
    DLP_DEBUG(0," ");

    // Save the parameters to a file
    ret_error = parameters.Save(param_filename);
    DLP_DEBUG(0,"Saved parameters to "<< param_filename << " " << ret_error.ToString());
    DLP_DEBUG(0," ");

    // Clear the parameter object
    DLP_DEBUG(0,"Parameters object has " << parameters.GetCount() << " entries.");
    DLP_DEBUG(0,"Clearing parameters object...");
    parameters.Clear();
    DLP_DEBUG(0,"Parameters object has " << parameters.GetCount() << " entries.");
    DLP_DEBUG(0," ");

    // Appending two parameter entries using std file io operations
    DLP_DEBUG(0,"Opening "<< param_filename << " with std file io methods");
    DLP_DEBUG(0,"to append two more parameter entries...");

    // Open file
    param_file.open(param_filename.c_str(), std::ios_base::app);

    // Add two new parameter entries
    param_file << "NEW_COLOR_0 = PURPLE" << std::endl;
    param_file << "NEW_COLOR_1 = ORANGE" << std::endl;

    // Close the file
    param_file.close();
    DLP_DEBUG(0," ");

    // Load the example_parameters_file.txt into parameters object
    DLP_DEBUG(0,"Loading "<< param_filename << " into parameters object..." << ret_error.ToString());
    ret_error = parameters.Load(param_filename);
    DLP_DEBUG(0," ");

    // Print all parameters how they were stored as strings
    DLP_DEBUG(0,"Printing the values loaded into parameters object..." );
    for( int i = 0; i < parameters.GetCount(); i++){
        ret_error = parameters.GetName(i,&param_name);          // Get parameter i's name
        ret_error = parameters.Get(param_name, "default value", &param_value);   // Get param_name value
        DLP_DEBUG(0,param_name << " = " << param_value );
    }
    DLP_DEBUG(0," ");


    return(0);
}

