#include <stdio.h>
#include <string>
#include "serialib.h"
#include "PiezoController.h"

PiezoController::PiezoController(const char* device, unsigned baudrate) {
    int ret = ser.openDevice(device, baudrate);
	printf("port %s", device);
	printf("baudrate %u\n", baudrate);
    // Open serial link at 115200 bauds
    if (ret != 1) {
        printf ("Error while opening port. Permission problem ?\n");
        exit(1);
    }
    
    printf ("Serial port opened successfully !\n");
    
    if (ret != 1) {   
        printf ("Error while opening port. Permission problem ?\n");
        exit(1);
    }
	// printf ("Serial port opened successfully !\n");
	//ser.writeString("C33H1024c;Y13=15;Y3=-100000;Y4=100000;Y7=1000;Y33=0;"); Original
	ser.writeString("C31H50c;Y13=15;Y3=-100000;Y4=100000");
	char response[1024];
	int status = ser.readString(response, '>', 1024, 30);
	status = ser.readString(response, '>', 1024, 30);
	status = ser.readString(response, '>', 1024, 30);
	status = ser.readString(response, '>', 1024, 30);
	status = ser.readString(response, '>', 1024, 30);
	status = ser.readString(response, '>', 1024, 30);
	status = ser.readString(response, '>', 1024, 30);
}

void PiezoController::Step(int steps) {
    std::string stepCommand = std::to_string(steps);
    if(steps > 0) {
        //stepCommand = "+" + stepCommand; Original
		stepCommand = "J" + stepCommand;
    }
    stepCommand += ";";
    ser.writeString(stepCommand.c_str());
}

void PiezoController::MoveTo(int position) {
	std::string targetCommand = "T"+std::to_string(position) + ";";
	ser.writeString(targetCommand.c_str());
}

int PiezoController::GetPosition() {
	ser.writeString("E;");
	char response[1024];
	int status = ser.readString(response, '\r', 1024, 30);
	if (status < 1) {
		printf("There was an issue when reading from serial: %d\n", status);
		exit(1);
	}
	//std::cout << response << std::endl;
	//std::cout << status << std::endl;
	char* pos = response+1;
	return std::atoi(pos);
}

bool PiezoController::Running() {
	ser.writeString("*;");
	char response[128];
	int status = ser.readString(response, 'R', 128, 30);
	if (status < 0) {
		printf("There was an issue when reading from serial: %d\n", status);
		exit(1);
	}
	if (response[0] == '0') {
		return true;
	} else {
		return false;
	}
}