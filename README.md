## Upgrader
A project to make a simple service that upgrades ROS-UI based applications. 

## What does this application do?
It is intendend to upgrade an application. To do this it reads a configuration file upgrader.conf, which is stored alongside the executable file. 
The configuration file contains a series of commands that execute the upgrade.

## Configuration 
In this simple example:
-task 0 runs the close application script. 
-task 1 runs the close launcher appliction script. (these need to run to unlock the ros-ui application file) 
-task 2 waits 2 seconds to allow the scripts to complete
-task 3 copies the new application file over the old application file
-task 4 restarts the launcher

[TASK.0]
action = run_systemctl
param_1 = --user
param_2 = stop
param_3 = ros-ui.service

[TASK.1]
action = run_systemctl
param_1 = --user
param_2 = stop 
param_3 = ros-ui-launcher.service

[TASK.2]
action = sleep
seconds = 2

[TASK.3]
action = file_move_exec
file_name = ros-ui

[TASK.4]
action = run_systemctl
param_1 = --user
param_2 = start 
param_3 = ros-ui-launcher.service

## Availble Actions
- file_move (Copy file to application directory)
- file_move_exec (Copy file to application directory, and make it executable)
- file_delete (Delete a file in the applciation directory)
- dir_create (Create a new directory)
- add_config (Add a new application configuration file change)
- update_config (Update a application configuration file)
- run_script (Run a shell script) 
- run_systemctl (Run and systemctl function) 
- sleep (Wait for a number of seconds)

# Making an installer 
This is a fairly simple task, of placing the upgrader application and the configured upgrader.conf file in a ZIP file with all the supporting files all at the root of the ZIP file. 

Rename the ZIP file to replace the extension .zip to .upg, so the application will know that this is a upgrade file.

For example with a simple executable replacement the files would look like:
- ros-ui (the replacement file)
- upgrader (the upgrader executable)
- upgrader.conf (the upgrade configuration file)


