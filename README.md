# analog upload gw

## Library Dependencies

Please download and install following arduino libraries.

* [PFFIAPUploadAgent](https://github.com/PlantFactory/PFFIAPUploadAgent)
* [Time](https://github.com/PaulStoffregen/Time)
* [LocalTime](https://github.com/PlantFactory/LocalTimeLib)
* [SerialCLI](https://github.com/PlantFactory/SerialCLI)
* [NTP](https://github.com/PlantFactory/NTP)

## Configuration

Configurations exist in L23 to L37.
Please change to your parameters.


```
//  ethernet
MacEntry mac("MAC", "B0:12:66:01:02:D4", "mac address");
//  ip
BoolEntry dhcp("DHCP", "true", "DHCP enable/disable");
IPAddressEntry ip("IP", "192.168.13.127", "IP address");
IPAddressEntry gw("GW", "192.168.13.1", "default gateway IP address");
IPAddressEntry sm("SM", "255.255.255.0", "subnet mask");
IPAddressEntry dns_server("DNS", "8.8.8.8", "dns server");
//  ntp
StringEntry ntp("NTP", "ntp.nict.jp", "ntp server");
//  fiap
StringEntry host("HOST", "202.15.110.21", "host of ieee1888 server end point");
IntegerEntry port("PORT", "80", "port of ieee1888 server end point");
StringEntry path("PATH", "/axis2/services/FIAPStorage", "path of ieee1888 server end point");
StringEntry prefix("PREFIX", "http://j.kisarazu.ac.jp/EcPhMeter/", "prefix of point id");
```
