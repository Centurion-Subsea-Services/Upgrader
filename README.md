## ROSCore Demo 
A demo project to make a simple service that uses the ROSCore code base. 

## What does this service do?
It will repeat publish values that have been configured in the an INI file. These values can be of Bool(0), Int32(1), Long(2), Float32(3), String(4), types supported by the ROSBoard. Also it will listen to a topic and if the there is an update to one of the values, or add a value then the publishing will change to that value. 

## Configuration 
[VALUE.0]
name = value_0
type = 0
value = 1
[VALUE.1]
name = value_1
type = 1
value = 123

These are repeating blocks until there are no more. Type is the data type Bool(0), Int32(1), Long(2), Float32(3), String(4), value is the initial value. The name parameter sets the ros topic name so if the base is /demo then 'value_0' would be published on /demo/value_0 and its subscriber is on /demo/value_0/set. 


