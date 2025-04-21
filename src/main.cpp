#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <filesystem>

#include "DriverCore/ini/ini.h"
#include "main.h"

#define LOG_NAME "client"
#define VERSION "1.1"
#define REQUIRED_CMD_PARMS 2

namespace fs = std::filesystem;

/**
 * The entry point of the application 
 */
int main (int argc, char **argv) {
	const int ACTION_COUNT = 1000;

    char result[PATH_MAX]; // Buffer to store the path
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    
    if (count == -1) {
        std::cerr << "Error reading executable path." << std::endl;
        return 1;
    }
	// Null-terminate the string
    result[count] = '\0'; 
    std::string executablePath(result);

    // Extract the directory path using std::filesystem (C++17)
    fs::path executableDir = std::filesystem::path(executablePath).parent_path();

    std::cout << "Executable path: " << executablePath << std::endl;
    std::cout << "Executable directory: " << executableDir << std::endl;
	// Create the INI file path
	std::string iniFilePath = executableDir.c_str();
	iniFilePath.append("/upgrader.conf");
	// If the application has one parameter then use it as the configuration file, else use the default
	if (argc == REQUIRED_CMD_PARMS) {
		iniFilePath = argv[1];
	}
	// Create the configuration file reader
    INIReader iniFile{iniFilePath};
	fs::path filePath(iniFilePath);
	std::string dataDir = executableDir.c_str();
	dataDir.append("/");
	
	// Create a logger for the solution
	std::shared_ptr<spdlog::logger> logger = spdlog::logger::createLogger(iniFile.Get<int>("upgrader", "logLevel", 2), LOG_NAME, "upgrader.log");

    logger->critical("Upgrader Application {} start with: config file:{}", VERSION, iniFilePath);
    logger->critical("Data Dir:{}", dataDir);

	// Wait for the calling application to quit
	sleep(1);

	// Loop for up to ACTION_COUNT tasks	
	for (int i = 0; i < ACTION_COUNT; i++) {
		std::string task = "TASK." + std::to_string(i);
		std::string action = iniFile.Get<std::string>(task, "action", "");
		logger->info("Task.{} action: {}", i, action);
		if (action.empty()) {
		    logger->info("Upgrader Application end found at task {}", i);
			return 0;
		} else if (0 == action.compare("file_move")) {
			// If this is a file move task
			try {
				std::string fileName = iniFile.Get<std::string>(task, "file_name", "");
				fs::path sourceFile = dataDir + fileName;
				std::string fileDir = iniFile.Get<std::string>(task, "file_directory", "");

				logger->info("CMD move file: {}:{}", fileName, fileDir);
				fs::path moveDirPath;
				if (!fileDir.empty()) {
					moveDirPath = fileDir;
				} else {
					moveDirPath = "../../";
				}
				// Create new file path
				fs::path destinationFile = moveDirPath / fileName;
				// Copy the file
				fs::copy_file(sourceFile, destinationFile, fs::copy_options::overwrite_existing);
				logger->info("Copied file: {}", fileName);
			} catch (const fs::filesystem_error& e) {
				logger->critical("Error in file copy: {} ", e.what());
			}
 		} else if (0 == action.compare("file_move_exec")) {
			// If this is an executable file move task
			try {
				std::string fileName = iniFile.Get<std::string>(task, "file_name", "");
				fs::path sourceFile = dataDir + fileName;
				std::string fileDir = iniFile.Get<std::string>(task, "file_directory", "");
				logger->info("CMD move EXE file: {}:{}", fileName, fileDir);
				fs::path moveDirPath;
				if (!fileDir.empty()) {
					moveDirPath = fileDir;
				} else {
					moveDirPath = "../../";
				}
				// Create new file path
				fs::path destinationFile = moveDirPath / fileName;
				fs::copy_file(sourceFile, destinationFile, fs::copy_options::overwrite_existing);
				// Add execute permission for the owner
			    fs::permissions(destinationFile, fs::perms::owner_exec, fs::perm_options::add);

				logger->info("Copied EXE file: {}", fileName);
			} catch (const fs::filesystem_error& e) {
				logger->critical("Error in file copy: {} ", e.what());
			}
		} else if (0 == action.compare("file_delete")) {
			// If this is a file delete task
			try {
				std::string fileName = iniFile.Get<std::string>(task, "file_name", "");
				std::string deleteDir = iniFile.Get<std::string>(task, "file_directory", "");
				logger->info("CMD Delete file: {}:{}", fileName, deleteDir);
				fs::path sourceFile = fileName;
				fs::path deleteDirPath;
				if (!deleteDir.empty()) {
					deleteDirPath = deleteDir;
				} else {
					deleteDirPath = "../../";
				}
				fs::path destinationFile = deleteDirPath / sourceFile.filename();

		        // Delete the file in the parent directory
				if (fs::exists(destinationFile)) {
					fs::remove(destinationFile);
					logger->info("Deleted file: {}", destinationFile.c_str());
				} else {
					logger->warn("File {} does not exist for deletion", destinationFile.c_str());
				}
			} catch (const fs::filesystem_error& e) {
				logger->critical("Error in file deletion: {} ", e.what());
			}
		} else if (0 == action.compare("dir_create")) {
			// If this is a dir create task
			try {
				std::string dirName = iniFile.Get<std::string>(task, "dir_name", "");
				fs::path destinationDir = "../../" + dirName;

				// Create the destination directory
				if (!fs::exists(destinationDir)) {
					fs::create_directories(destinationDir);
					logger->info("Created directory: {}", destinationDir.c_str());
				} else {
					logger->warn("Directory {} exists", destinationDir.c_str());
				}
			} catch (const fs::filesystem_error& e) {
				logger->critical("Error in dir creation: {} ", e.what());
			}
		} else if (0 == action.compare("add_config")) {
			// If this is a config file add task
			try {
				std::string fileName = iniFile.Get<std::string>(task, "file_name", "");
				std::string targetSection = iniFile.Get<std::string>(task, "target_section", "");
				std::string targetName = iniFile.Get<std::string>(task, "target_name", "");
				std::string targetValue = iniFile.Get<std::string>(task, "target_value", "");
				fs::path sourceFile = fileName;
			    // Resolve the home directory
				const char* homeDir = std::getenv("HOME");
				if (!homeDir) {
					logger->critical("Could not resolve the home directory. HOME environment variable is not set.");
				} else {
					fs::path destinationDir = fs::path(homeDir) / ".config/CSS";
					fs::path destinationFile = destinationDir / sourceFile.filename();
					// Create the ini file editor with a link to the target file
					INIReader iniTargetFile{destinationFile.c_str()};
					// Add the entry to the config file
					iniTargetFile.InsertEntry(targetSection, targetName, targetValue);
					INIWriter configWriter;
					fs::remove(destinationFile);
					configWriter.write(destinationFile.c_str(), iniTargetFile);
					logger->info("Inserted to: {}, Section: {}, Name: {}, Value: {}", destinationFile.c_str(), targetSection, targetName, targetValue);
				}
			} catch (const std::runtime_error& e) {
				logger->critical("Error in config file add: {} ", e.what());
			}
		} else if (0 == action.compare("update_config")) {
			// If this is a config file update task
			try {
				std::string fileName = iniFile.Get<std::string>(task, "file_name", "");
				std::string targetSection = iniFile.Get<std::string>(task, "target_section", "");
				std::string targetName = iniFile.Get<std::string>(task, "target_name", "");
				std::string targetValue = iniFile.Get<std::string>(task, "target_value", "");
				fs::path sourceFile = fileName;
			    // Resolve the home directory
				const char* homeDir = std::getenv("HOME");
				if (!homeDir) {
					logger->critical("Could not resolve the home directory. HOME environment variable is not set.");
				} else {
					fs::path destinationDir = fs::path(homeDir) / ".config/CSS";
					fs::path destinationFile = destinationDir / sourceFile.filename();
					// Create the ini file editor with a link to the target file
					INIReader iniTargetFile{destinationFile.c_str()};
					// Add the entry to the config file
					iniTargetFile.UpdateEntry(targetSection, targetName, targetValue);
					INIWriter configWriter;
					fs::remove(destinationFile);
					configWriter.write(destinationFile.c_str(), iniTargetFile);
					logger->info("Updated to: {}, Section: {}, Name: {}, Value: {}", destinationFile.c_str(), targetSection, targetName, targetValue);
				}
			} catch (const std::runtime_error& e) {
				logger->critical("Error in config file update: {} ", e.what());
			}
		} else if (0 == action.compare("run_script")) {
			// If this is a run script task
			try {
				std::string fileName = iniFile.Get<std::string>(task, "file_name", "");
				fs::path sourceFile = fileName;
				fs::path destinationDir = "../../";
				fs::path destinationFile = destinationDir / sourceFile.filename();
				int result = std::system(destinationFile.c_str());
				if (result == 0) {
					logger->info("Script {} executed successfully.", destinationFile.c_str());
				} else {
					logger->info("Script {} Failed to execute.", destinationFile.c_str());
				}
			} catch (const std::runtime_error& e) {
				logger->critical("Error in config file update: {} ", e.what());
			}
		} else if (0 == action.compare("run_systemctl")) {
			// If this is a run script task
			try {
				std::string param1 = iniFile.Get<std::string>(task, "param_1", "");
				std::string param2 = iniFile.Get<std::string>(task, "param_2", "");
				std::string param3 = iniFile.Get<std::string>(task, "param_3", "");
				std::string command = "systemctl " + param1 + " " + param2 + " " + param3;

				logger->info("Executing command: {}", command);

				// Execute the system command
				int result = std::system(command.c_str());

				// Check the result
				if (result == 0) {
					logger->info("Successfully ran: {}", command);
				} else {
					logger->warn("Failed to run: {}", command);
				}
			} catch (const std::runtime_error& e) {
				logger->critical("Error in config file update: {} ", e.what());
			}
		} else if (0 == action.compare("sleep")) {
			// If this is a run script task
			try {
				int seconds = iniFile.Get<int>(task, "seconds", 5);

				logger->info("Sleeping sec: {}", seconds);

				// Execute the system command
				sleep(seconds);

			} catch (const std::runtime_error& e) {
				logger->critical("Error in config file update: {} ", e.what());
			}
		}
	}
	logger->warn("Warn: exit because hit maximum action count {}", ACTION_COUNT);
	return 0;
}