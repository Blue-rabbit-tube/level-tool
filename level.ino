#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SSD1306_LCDHEIGHT 64
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 
#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

long accelX, accelY, accelZ;      // 定义为全局变量，可直接在函数内部使用
float gForceX, gForceY, gForceZ;
 
long gyroX, gyroY, gyroZ;
float rotX, rotY, rotZ;

 
void setup() {
  Serial.begin(115200);
  Wire.begin();
  setupMPU();

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); 
  test_OPEN();     //调用开机函数
}

void loop() {
  recordAccelRegisters();
  recordGyroRegisters();
  printData();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); 
  test_SSD1306();     //调用测试函数
}


void test_OPEN(void)  
{
  display.clearDisplay(); 
  display.setTextSize(2); //选择字号
  display.setTextColor(WHITE);  //字体颜色
  display.setCursor(20,10);   //起点坐标
  display.println("WELCOME");
  display.display();
  delay(5);
  
  display.clearDisplay(); 
  display.setTextSize(2); //选择字号
  display.setTextColor(WHITE);  //字体颜色
  display.setCursor(30,10);   //起点坐标
  display.println("LEVEL");
  display.display();
  delay(5);

  display.clearDisplay(); 
  display.setTextSize(2); //选择字号
  display.setTextColor(WHITE);  //字体颜色
  display.setCursor(2,10);   //起点坐标
  display.println(" JZX (T_T)");
  display.display();
  delay(5);
}



void test_SSD1306(void)   //测试函数
{
  display.clearDisplay(); 
  display.setTextSize(1); //选择字号
  display.setTextColor(WHITE);  //字体颜色
  display.drawLine(0, 7,128,7, WHITE);   //分割线
/*  display.drawLine(40, 0,40,64, WHITE);   //分割线
  display.drawLine(75, 0,75,64, WHITE);   //分割线*/
  display.setCursor(3,0);   //起点坐标
  display.print("ANG: X     Y     Z");
  display.setTextSize(1); //选择字号
  display.setCursor(30,8);  
  display.print((int)(gForceX*100), DEC);
  display.setCursor(60,8);   //起点坐标
  display.print((int)(gForceY*100), DEC);
  display.setCursor(100,8);   //起点坐标
  display.print((int)(gForceZ*100), DEC);

  display.drawLine(0, 16,128,16, WHITE);   //分割线
  display.setCursor(3,17);   //起点坐标
  display.print("ANG: X     Y     Z");
  display.setTextSize(1); //选择字号
  display.drawLine(0, 24,128,24, WHITE);   //分割线
  display.setCursor(30,25);  
  display.print((int)(rotX*100), DEC);
  display.setCursor(60,25);   //起点坐标
  display.print((int)(rotY*100), DEC);
  display.setCursor(100,25);   //起点坐标
  display.print((int)(rotZ*100), DEC);
  display.display();
}






void setupMPU(){
  // REGISTER 0x6B/REGISTER 107:Power Management 1
  Wire.beginTransmission(0b1101000); //This is the I2C address of the MPU (b1101000/b1101001 for AC0 low/high datasheet Sec. 9.2)
  Wire.write(0x6B); //Accessing the register 6B/107 - Power Management (Sec. 4.30) 
  Wire.write(0b00000000); //Setting SLEEP register to 0, using the internal 8 Mhz oscillator
  Wire.endTransmission();
 
  // REGISTER 0x1b/REGISTER 27:Gyroscope Configuration
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x1B); //Accessing the register 1B - Gyroscope Configuration (Sec. 4.4) 
  Wire.write(0x00000000); //Setting the gyro to full scale +/- 250deg./s (转化为rpm:250/360 * 60 = 41.67rpm) 最高可以转化为2000deg./s 
  Wire.endTransmission();
  
  // REGISTER 0x1C/REGISTER 28:ACCELEROMETER CONFIGURATION
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x1C); //Accessing the register 1C - Acccelerometer Configuration (Sec. 4.5) 
  Wire.write(0b00000000); //Setting the accel to +/- 2g（if choose +/- 16g，the value would be 0b00011000）
  Wire.endTransmission(); 
}
 
void recordAccelRegisters() {
  // REGISTER 0x3B~0x40/REGISTER 59~64
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x3B); //Starting register for Accel Readings
  Wire.endTransmission();
  Wire.requestFrom(0b1101000,6); //Request Accel Registers (3B - 40)
 
  // 使用了左移<<和位运算|。Wire.read()一次读取1bytes，并在下一次调用时自动读取下一个地址的数据
  while(Wire.available() < 6);  // Waiting for all the 6 bytes data to be sent from the slave machine （必须等待所有数据存储到缓冲区后才能读取） 
  accelX = Wire.read()<<8|Wire.read(); //Store first two bytes into accelX （自动存储为定义的long型值）
  accelY = Wire.read()<<8|Wire.read(); //Store middle two bytes into accelY
  accelZ = Wire.read()<<8|Wire.read(); //Store last two bytes into accelZ
  processAccelData();
}
 
void processAccelData(){
  gForceX = accelX / 16384.0;     //float = long / float
  gForceY = accelY / 16384.0; 
  gForceZ = accelZ / 16384.0;
}
 
void recordGyroRegisters() {
  // REGISTER 0x43~0x48/REGISTER 67~72
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x43); //Starting register for Gyro Readings
  Wire.endTransmission();
  Wire.requestFrom(0b1101000,6); //Request Gyro Registers (43 ~ 48)
  while(Wire.available() < 6);
  gyroX = Wire.read()<<8|Wire.read(); //Store first two bytes into accelX
  gyroY = Wire.read()<<8|Wire.read(); //Store middle two bytes into accelY
  gyroZ = Wire.read()<<8|Wire.read(); //Store last two bytes into accelZ
  processGyroData();
}
 
void processGyroData() {
  rotX = gyroX / 131.0;
  rotY = gyroY / 131.0; 
  rotZ = gyroZ / 131.0;
}
 
void printData() {
/*  
  Serial.print("Gyro (deg)");
  Serial.print(" X=");
  Serial.print(rotX);
  Serial.print(" Y=");
  Serial.print(rotY);
  Serial.print(" Z=");
  Serial.print(rotZ);
  Serial.print(" Accel (g)");
  Serial.print(" X=");
  Serial.print(gForceX);
  Serial.print(" Y=");
  Serial.print(gForceY);
  Serial.print(" Z=");
  Serial.println(gForceZ);
  */
}
