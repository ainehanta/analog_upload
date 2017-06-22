/*
 * analog upload
 *
 * Author:   Makoto Uju (ainehanta), Hiromasa Ihara (taisyo)
 * Created:  2016-07-04
 */

#include <Ethernet.h>
#include <EthernetUdp.h>

#include <PFFIAPUploadAgent.h>
#include <TimeLib.h>
#include <LocalTimeLib.h>
#include <SerialCLI.h>
#include <NTP.h>

#define GREEN_LED 22
#define RED_LED   23
#define SEND_SEC  30

//cli
SerialCLI commandline(Serial);
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
//  debug
int debug = 1;

//ntp
NTPClient ntpclient;
TimeZone localtimezone = { 9*60*60, 0, "+09:00" };

//fiap
FIAPUploadAgent fiap_upload_agent;
char a0_str[6];
char a1_str[6];
char a2_str[6];
char a3_str[6];
char a4_str[6];
char a5_str[6];

uint16_t analog_values[6];

struct fiap_element fiap_elements [] = {
  { "A0", a0_str, 0, &localtimezone, },
  { "A1", a1_str, 0, &localtimezone, },
  { "A2", a2_str, 0, &localtimezone, },
  { "A3", a3_str, 0, &localtimezone, },
  { "A4", a4_str, 0, &localtimezone, },
  { "A5", a5_str, 0, &localtimezone, },
};

// buffers
char buf[32];

void enable_debug()
{
  debug = 1;
}

void disable_debug()
{
  debug = 0;
}

void setup()
{
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, HIGH);
  
  int ret;
  commandline.add_entry(&mac);

  commandline.add_entry(&dhcp);
  commandline.add_entry(&ip);
  commandline.add_entry(&gw);
  commandline.add_entry(&sm);
  commandline.add_entry(&dns_server);

  commandline.add_entry(&ntp);

  commandline.add_entry(&host);
  commandline.add_entry(&port);
  commandline.add_entry(&path);
  commandline.add_entry(&prefix);

  commandline.add_command("debug", enable_debug);
  commandline.add_command("nodebug", disable_debug);

  commandline.begin(9600, "analog upload gw");

  //ethernet & ip connection
  if(dhcp.get_val() == 1){
    ret = Ethernet.begin(mac.get_val());
    if(ret == 0) {
      restart("Failed to configure Ethernet using DHCP", 10);
    }
  }else{
    Ethernet.begin(mac.get_val(), ip.get_val(), dns_server.get_val(), gw.get_val(), sm.get_val()); 
  }

  // fetch time
  uint32_t unix_time = 0;
  ntpclient.begin();
  ret = ntpclient.getTime(ntp.get_val(), &unix_time);
  if(ret < 0){
    restart("Failed to configure time using NTP", 10);
  }
  setTime(unix_time);

  // fiap
  fiap_upload_agent.begin(host.get_val().c_str(), path.get_val().c_str(), port.get_val(), prefix.get_val().c_str());

  digitalWrite(RED_LED, LOW);
}

void loop()
{
  static unsigned long old_epoch = 0, epoch;
  
  commandline.process();
  epoch = now();
  if(dhcp.get_val() == 1){
    Ethernet.maintain();
  }

  if(epoch != old_epoch){
    analog_values[0] = analogRead(A0);
    analog_values[1] = analogRead(A1);
    analog_values[2] = analogRead(A2);
    analog_values[3] = analogRead(A3);
    analog_values[4] = analogRead(A4);
    analog_values[5] = analogRead(A5);

    sprintf(buf, "A0 = %d", analog_values[0]);
    debug_msg(buf);
    sprintf(buf, "A1 = %d", analog_values[1]);
    debug_msg(buf);
    sprintf(buf, "A2 = %d", analog_values[2]);
    debug_msg(buf);
    sprintf(buf, "A3 = %d", analog_values[3]);
    debug_msg(buf);
    sprintf(buf, "A4 = %d", analog_values[4]);
    debug_msg(buf);
    sprintf(buf, "A5 = %d", analog_values[5]);
    debug_msg(buf);

    if(epoch != old_epoch){

      if(epoch % SEND_SEC == 0){
        digitalWrite(RED_LED, HIGH);
        debug_msg("uploading...");

        sprintf(a0_str, "%d", analog_values[0]);
        sprintf(a1_str, "%d", analog_values[1]);
        sprintf(a2_str, "%d", analog_values[2]);
        sprintf(a3_str, "%d", analog_values[3]);
        sprintf(a4_str, "%d", analog_values[4]);
        sprintf(a5_str, "%d", analog_values[5]);

        for(int i = 0; i < sizeof(fiap_elements)/sizeof(fiap_elements[0]); i++){
          fiap_elements[i].time = epoch;
        }
        int ret = fiap_upload_agent.post(fiap_elements, sizeof(fiap_elements)/sizeof(fiap_elements[0]));
        if(ret == 0){
          debug_msg("done");
        }else{
          debug_msg("failed");
          debug_msg("code"+ret);
        }
      }
    }
    digitalWrite(RED_LED, LOW);
  }

  old_epoch = epoch;
}

void debug_msg(String msg)
{
  if(debug == 1){
    Serial.print("[");
    print_time();
    Serial.print("]");
    Serial.println(msg);
  }
}

void print_time()
{
  char print_time_buf[32];
  TimeElements* tm;
  tm = localtime();
  sprintf(print_time_buf, "%04d/%02d/%02d %02d:%02d:%02d",
      tm->Year + 1970, tm->Month, tm->Day, tm->Hour, tm->Minute, tm->Second);
  Serial.print(print_time_buf);
}

void restart(String msg, int restart_minutes)
{
  Serial.println(msg);
  Serial.print("This system will restart after ");
  Serial.print(restart_minutes);
  Serial.print("minutes.");

  unsigned int start_ms = millis();
  while(1){
    commandline.process();
    if(millis() - start_ms > restart_minutes*60UL*1000UL){
      commandline.reboot();
    }
  }
}
