// Printing and file handling
#include <stdio.h>
#include <string.h>
#include <stdlib.h>  // atoi
#include "gpio.h"


int _gpio_get_filename(gpio_pin* gpio,char* fname,GPIO_PROPERTIES prop)
{
    /**
     * @brief Generates a filename based on gpio number and property.
     * 
     * This helper function does not check if the file actually exists.
     *
     * @param gpio gpio_pin struct pointer.
     * @param fname filename buffer to be written to.
     * @param prop GPIO property enum.
     * @return 1 on success, -1 if property is not recognized.
     */
    if(prop == GPIO_PROP_DIRECTION)
        sprintf(fname,  "/sys/class/gpio/gpio%d/%s",gpio->number,"direction");
    else if(prop == GPIO_PROP_EDGE)
        sprintf(fname,  "/sys/class/gpio/gpio%d/%s",gpio->number,"edge");
    else if(prop == GPIO_PROP_LABEL)
        sprintf(fname,  "/sys/class/gpio/gpio%d/%s",gpio->number,"label");
    else if(prop == GPIO_PROP_VALUE)
        sprintf(fname,  "/sys/class/gpio/gpio%d/%s",gpio->number,"value");
    else if(prop == GPIO_PROP_ACTIVE_LOW)
        sprintf(fname,  "/sys/class/gpio/gpio%d/%s",gpio->number,"active_low");
    else
        return -1;
    return 1;
}

int _gpio_write(gpio_pin* gpio,GPIO_PROPERTIES prop,char* value)
{
    /**
     * @brief Writes a value to a GPIO pin property file.
     *
     * Uses the given gpio data and the desired property `prop`, builds the
     * filename, and attempts to write `value` to that file.
     *
     * @param gpio gpio_pin struct pointer.
     * @param prop GPIO property enum.
     * @param value C string with value to be written to file
     * @return 1 on success, -1 on failure.
     */
    // Generate filename.
    char filename[FILENAME_MAX];
    int stat = _gpio_get_filename(gpio,filename,prop);
    if(stat != 1)
    {
        printf("Unrecognized GPIO_PROPERTIES value %d\n",prop);
        return -1;
    }

    // Open file.
    FILE* fp = fopen(filename,"w");
    if(!fp){
	    perror("File opening failed.");
        printf("Check that the path is correct and that you have privileges for writing to it. The GPIO pin might not exist.\n");
	    return -1;
    }

    // Write to file.
    int write_status = fputs(value,fp);
    if(write_status == EOF)
    {
        printf("File writing failed with fputs(%s,%s)\n",value,filename);
        printf("Check that you have privileges for writing.\n");
        return -1;
    }
    // Close file.
    fclose(fp);
    // Return success.
    return 1;
}

int _gpio_read(gpio_pin* gpio,GPIO_PROPERTIES prop,char* value)
{
    /**
     * @brief Reads the GPIO property into a buffer.
     *
     * Uses the given gpio data and the desired property `prop`, builds the
     * filename, and attempts to read the contents into `value`.
     *
     * @param gpio gpio_pin struct pointer.
     * @param prop GPIO property enum.
     * @return Return 1 on success, -1 on failure.
     */
    char filename[FILENAME_MAX];
    int stat = _gpio_get_filename(gpio,filename,prop);
    if(stat != 1)
    {
        printf("Unrecognized GPIO_PROPERTIES value %d\n",prop);
        return -1;
    }

    // Open file.
    FILE* fp = fopen(filename,"r");
    if(!fp){
	    perror("File opening failed, check that the path is correct and that you have privileges for reading it. The GPIO pin might not exist. Error");
	    return -1;
    }

    // Read from file.
    fgets(value,100,fp);
    value[strcspn(value, "\n") ] = '\0'; // Remove line ending

    // Close file.
    fclose(fp);
    // Return success.
    return 1;
}

int gpio_set_direction(gpio_pin* gpio,GPIO_DIRECTION dir)
{
    /**
     * @brief Set GPIO pin direction.
     *
     * @param gpio gpio_pin struct pointer.
     * @param dir GPIO_DIRECTION enum value.
     * @return 1 on success, -1 on failure.
     */
    int write_status = -1;
    if(dir == GPIO_DIR_INPUT)
    {
        write_status = _gpio_write(gpio,GPIO_PROP_DIRECTION,"in");
    }
    else if(dir == GPIO_DIR_OUTPUT)
    {
        write_status = _gpio_write(gpio,GPIO_PROP_DIRECTION,"out");
    }
    else write_status = -1;
    return write_status;
}

GPIO_DIRECTION gpio_get_direction(gpio_pin* gpio)
{
    /**
     * @brief Read GPIO pin direction and return it as GPIO_DIRECTION enum.
     *
     * @param gpio gpio_pin struct pointer.
     * @return GPIO_DIRECTION value.
     */
    char dir[10];
    _gpio_read(gpio,GPIO_PROP_DIRECTION,dir);
    if(strcmp(dir,"in") == 0)
    {
        return GPIO_DIR_INPUT;
    }
    else if(strcmp(dir,"out")==0)
    {
        return GPIO_DIR_OUTPUT;
    }
    return -1;
}

int gpio_set_value(gpio_pin* gpio,GPIO_VALUE val)
{
    /**
     * @brief Write GPIO pin value through sysfs.
     *
     * @param gpio gpio_pin struct pointer.
     * @return Return description.
     */
    int write_status = -1;
    
    char str[2];
	sprintf(str, "%d", val);

    write_status = _gpio_write(gpio,GPIO_PROP_VALUE,str);
    return write_status;
}

GPIO_VALUE gpio_get_value(gpio_pin* gpio)
{
    /**
     * @brief Read and return GPIO pin value.
     *
     * @param gpio gpio_pin struct pointer.
     * @return GPIO_VALUE value, or -1 on failure.
     */
    int read_status = -1;
    char val_str[2];
    read_status = _gpio_read(gpio,GPIO_PROP_VALUE,val_str);
    if(read_status == -1) return -1;
    GPIO_VALUE val_int = atoi(val_str);

    return val_int;
}

int gpio_toggle_output(gpio_pin* gpio)
{
    /**
     * @brief Toggle GPIO pin value.
     *
     * @param gpio gpio_pin struct pointer.
     * @return New value on success, -1 on failure.
     */
    GPIO_VALUE current_val = gpio_get_value(gpio);
    if(current_val == -1) return -1;
    GPIO_VALUE new_val = current_val ^ 1;  // XOR
    return new_val;
}

int gpio_set_active_low(gpio_pin* gpio)
{
    /**
     * @brief Summary.
     *
     * Long description.
     *
     * @param gpio gpio_pin struct pointer.
     * @return Return description.
     */
    int write_status = _gpio_write(gpio,GPIO_PROP_ACTIVE_LOW,"1");
    return write_status;
}

int gpio_set_active_high(gpio_pin* gpio)
{
    /**
     * @brief Summary.
     *
     * Long description.
     *
     * @param gpio gpio_pin struct pointer.
     * @return Return description.
     */
    int write_status = _gpio_write(gpio,GPIO_PROP_ACTIVE_LOW,"0");
    return write_status;
}




// int main()
// {
//     // TESTS
//     // For these tests, use GPIO67 (PIN37). Here is its info:
//     // GPIO67 (unallocated)
//     // pin 37 (PIN37): ocp:P8_08_pinmux (GPIO UNCLAIMED) function pinmux_P8_08_default_pin group pinmux_P8_08_default_pin
//     // pin 37 (PIN37) 3:gpio-64-95 44e10894 00000037 pinctrl-single
//     // Pad config: 0x37 = 0 1 1 0 111 = fast, input/output, pullup, pull enable, mode 7
//     // GPIO settings:
//     //   direction = in
//     //   active_low = 0
//     //   edge = none
//     //   label = P8_08
//     //   value = 1
//     gpio_pin gpio67;
//     gpio67.number = 67;
//     gpio67.dir = GPIO_DIR_INPUT;
    
//     // int _gpio_get_filename(gpio_pin* gpio,char* fname,GPIO_PROPERTIES prop);
//     /**
//      * @brief Generates a filename based on gpio number and property.
//      * 
//      * This helper function does not check if the file actually exists.
//      *
//      * @param gpio gpio_pin struct pointer.
//      * @param fname filename buffer to be written to.
//      * @param prop GPIO property enum.
//      * @return 1 on success, -1 if property is not recognized.
//      */
//     // Common test parameters //
//     char fname[FILENAME_MAX];
//     int stat = -999;

//     printf("1. Existing GPIO pin and property\n");
//     stat = _gpio_get_filename(&gpio67,fname,GPIO_PROP_DIRECTION);
//     printf("Filename for gpio67 direction: %s\n",fname);
//     printf("Status out: %d\n",stat);

//     printf("2. Existing GPIO pin but non-existent property\n");
//     stat = _gpio_get_filename(&gpio67,fname,999);
//     printf("Filename for gpio67 non-existent property: %s\n",fname);
//     printf("Status out: %d\n",stat);

//     printf("3. Non-existent GPIO pin and existing property\n");
//     gpio67.number = 999;
//     stat = _gpio_get_filename(&gpio67,fname,GPIO_PROP_DIRECTION);
//     printf("Filename for gpio67 (999) direction: %s\n",fname);
//     printf("Status out: %d\n",stat);
//     gpio67.number = 67;

//     printf("4. Non-existent GPIO pin and property\n");
//     gpio67.number = 999;
//     stat = _gpio_get_filename(&gpio67,fname,GPIO_PROP_DIRECTION);
//     printf("Filename for gpio67 (999) non-existent property: %s\n",fname);
//     printf("Status out: %d\n",stat);
//     gpio67.number = 67;

//     printf("\n");

//     // int _gpio_read(gpio_pin* gpio,GPIO_PROPERTIES prop,char* value)
//     /**
//      * @brief Reads the GPIO property into a buffer.
//      *
//      * Uses the given gpio data and the desired property `prop`, builds the
//      * filename, and attempts to read the contents into `value`.
//      *
//      * @param gpio gpio_pin struct pointer.
//      * @param prop GPIO property enum.
//      * @return Return 1 on success, -1 on failure.
//      */
//     // Common test parameters //
//     char pindir[10];
//     stat = -999;

//     printf("1. Existing pin and property\n");
//     sprintf(pindir,"\0");
//     stat = _gpio_read(&gpio67,GPIO_PROP_DIRECTION,pindir);
//     printf("gpio67 direction: %s\n",pindir);
//     printf("Status out: %d\n",stat);

//     printf("2. Existing pin and non-existent property\n");
//     sprintf(pindir,"\0");
//     stat = _gpio_read(&gpio67,99,pindir);
//     printf("gpio67 bad property: %s\n",pindir);
//     printf("Status out: %d\n",stat);

//     printf("3. Non-existent pin and existing property\n");
//     sprintf(pindir,"\0");
//     gpio67.number = 999;
//     stat = _gpio_read(&gpio67,GPIO_PROP_DIRECTION,pindir);
//     printf("gpio67 (999) direction: %s\n",pindir);
//     printf("Status out: %d\n",stat);
//     gpio67.number = 67;

//     printf("4. Non-existent pin and property\n");
//     sprintf(pindir,"\0");
//     gpio67.number = 999;
//     stat = _gpio_read(&gpio67,999,pindir);
//     printf("gpio67 (999) bad property: %s\n",pindir);
//     printf("Status out: %d\n",stat);
//     gpio67.number = 67;


//     // int _gpio_write(gpio_pin* gpio,GPIO_PROPERTIES prop,char* value)
//     /**
//      * @brief Writes a value to a GPIO pin property file.
//      *
//      * Uses the given gpio data and the desired property `prop`, builds the
//      * filename, and attempts to write `value` to that file.
//      *
//      * @param gpio gpio_pin struct pointer.
//      * @param prop GPIO property enum.
//      * @param value C string with value to be written to file
//      * @return 1 on success, -1 on failure.
//      */

//     // Common test parameters //
//     char pindir_set[] = "in";
//     stat = -999;
//     sprintf(pindir,"\0");

//     printf("1. Existing pin and property, attempting to write \"%s\"\n",pindir_set);
//     stat = _gpio_write(&gpio67,GPIO_PROP_DIRECTION,pindir_set);
//     printf("Attempting to read pin direction...\n");
//     _gpio_read(&gpio67,GPIO_PROP_DIRECTION,pindir);
//     printf("Read: %s\n",pindir);
//     printf("String comparison: %d\n",strcmp(pindir_set,pindir));
//     if(strcmp(pindir_set,pindir)==0)
//     {
//         printf("Success.\n");
//     }
//     else printf("Failure.\n");
//     printf("\n");

//     printf("2. Non-existent pin and existing property, attempting to write \"%s\"\n",pindir_set);
//     gpio67.number = 999;
//     sprintf(pindir,"\0");
//     stat = _gpio_write(&gpio67,GPIO_PROP_DIRECTION,pindir_set);
//     printf("Attempting to read pin direction...\n");
//     _gpio_read(&gpio67,GPIO_PROP_DIRECTION,pindir);
//     printf("Read: %s\n",pindir);
//     if(strcmp(pindir_set,pindir)==0)
//     {
//         printf("Success.\n");
//     }
//     else printf("Failure.\n");
//     gpio67.number = 67;

//     GPIO_DIRECTION dir = gpio_get_direction(&gpio67);
//     printf("Reading pin 67 direction: %s\n",GPIO_DIRECTION_MAP[dir]);
//     GPIO_VALUE val = gpio_get_value(&gpio67);
//     printf("Reading pin 67 value: %s\n",GPIO_VALUE_MAP[val]);


//     return 0;
// }





