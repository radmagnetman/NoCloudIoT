# NoCloudIoT

Code and designs for my "No Cloud" IoT project.

I disliked the idea of my home brew IoT devices communicating with a cloud service so I have started making a code base for a server(s) and IoT objects. 

Very much a work in progress..

## Design decisions

### Data flow

![Data flow](https://raw.githubusercontent.com/radmagnetman/NoCloudIoT/master/data_flow_diagram.jpg)


### Why MQTT?

My first attempt at making an IoT type T/RH sensor package, the device worked near flawlessly (still need to make a v2 of the hardware). In that iteration of the project, I had attempted (and succeeded!) in using UDP packets to transmit the data. The logger though... that was a bear. Receiving packets, parsing and logging the data reliably was something I struggled with. 

After some time off of the project, I ordered a simple Sonoff IoT switch just to tinker with. The device wants you to use a cloud based service and it just squigs me out freely transmitting data from my phone to the cloud and back to my house without any idea what was in between. Some searches led me to the maker forums where likeminded people talked about using MQTT in place of the cloud service.

Turns out, they were right, MQTT works great for this. It's light weight, libraries already exist, and once the paradigm was digested, the project became compartmentalized. Rather than keeping a list of devices in the network and what their IP address and capabilities are, I can just build a device, and the device encapsulates all the commands. 

### No Strings

Due to memory fragmentation, the Strings library is not recommended for Arduino use if the device will be running continuously. This decision forced the use of predeclaring char arrays so that they can be overwritten during operation with relavent data. 
