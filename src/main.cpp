// embedded challenge
// luigia than, shelby hou, john choi, shreya nalla

#include "mbed.h"

// variable declarations

//example 2 from recitation 5: 
//spi intialization
SPI spi(PF_9, PF_8, PF_7,PC_1,use_gpio_ssel); // mosi, miso, sclk, cs

//address of first register with gyro data
#define OUT_X_L 0x28

//register fields(bits): data_rate(2),Bandwidth(2),Power_down(1),Zen(1),Yen(1),Xen(1)
#define CTRL_REG1 0x20

//configuration: 200Hz ODR,50Hz cutoff, Power on, Z on, Y on, X on
#define CTRL_REG1_CONFIG 0b01'10'1'1'1'1

//register fields(bits): reserved(1), endian-ness(1),Full scale sel(2), reserved(1),self-test(2), SPI mode(1)
#define CTRL_REG4 0x23

//configuration: reserved,little endian,500 dps,reserved,disabled,4-wire mode
#define CTRL_REG4_CONFIG 0b0'0'01'0'00'0

#define SPI_FLAG 1

uint8_t write_buf[32]; 
uint8_t read_buf[32];

EventFlags flags;
//The spi.transfer() function requires that the callback
//provided to it takes an int parameter
void spi_cb(int event){
  flags.set(SPI_FLAG);
}

// LCD initialization
#include "drivers/LCD_DISCO_F429ZI.h"
LCD_DISCO_F429ZI lcd;

//LCD display buffers
char bufx[60];
char bufy[60];
char bufz[60];
char buft[60];
uint16_t result = 0;

//timer
Timer t;
volatile uint32_t elapsed_time;


int main(){

    //spi intialization for gyroscope
    spi.format(8,3);
    spi.frequency(1'000'000);

    write_buf[0]=CTRL_REG1;
    write_buf[1]=CTRL_REG1_CONFIG;
    spi.transfer(write_buf,2,read_buf,2,spi_cb,SPI_EVENT_COMPLETE );
    flags.wait_all(SPI_FLAG);

    write_buf[0]=CTRL_REG4;
    write_buf[1]=CTRL_REG4_CONFIG;
    spi.transfer(write_buf,2,read_buf,2,spi_cb,SPI_EVENT_COMPLETE );
    flags.wait_all(SPI_FLAG); 

    //LCD
    lcd.Clear(LCD_COLOR_LIGHTBLUE);
    lcd.SetTextColor(LCD_COLOR_BLACK);
    lcd.DisplayStringAtLine(0, (uint8_t *)"Hello World!");


    t.start();

    while (1){
    int16_t raw_gx,raw_gy,raw_gz;
    float gx, gy, gz;
      //prepare the write buffer to trigger a sequential read
      write_buf[0]=OUT_X_L|0x80|0x40;
      //start sequential sample reading
      spi.transfer(write_buf,7,read_buf,7,spi_cb,SPI_EVENT_COMPLETE );
      flags.wait_all(SPI_FLAG);
      //read_buf after transfer: garbage byte, gx_low,gx_high,gy_low,gy_high,gz_low,gz_high
      //Put the high and low bytes in the correct order lowB,HighB -> HighB,LowB
      raw_gx=( ( (uint16_t)read_buf[2] ) <<8 ) | ( (uint16_t)read_buf[1] );
      raw_gy=( ( (uint16_t)read_buf[4] ) <<8 ) | ( (uint16_t)read_buf[3] );
      raw_gz=( ( (uint16_t)read_buf[6] ) <<8 ) | ( (uint16_t)read_buf[5] );

      //printf("RAW|\tgx: %d \t gy: %d \t gz: %d\t",raw_gx,raw_gy,raw_gz);

      gx=((float)raw_gx)*(17.5f*0.017453292519943295769236907684886f / 1000.0f);
      gy=((float)raw_gy)*(17.5f*0.017453292519943295769236907684886f / 1000.0f);
      gz=((float)raw_gz)*(17.5f*0.017453292519943295769236907684886f / 1000.0f);
      

      //prep test for LCD
      snprintf(bufx, 60, "gx: %4.5f",gx);
      snprintf(bufy, 60, "gy: %4.5f",gy);
      snprintf(bufz, 60, "gz: %4.5f",gz);
      //display text
      lcd.DisplayStringAtLine(2, (uint8_t *)bufx);
      lcd.DisplayStringAtLine(3, (uint8_t *)bufy);
      lcd.DisplayStringAtLine(4, (uint8_t *)bufz);

      //checking if twenty seconds have passed
      elapsed_time=t.read_ms();
      if (elapsed_time >= 20000){
        snprintf(buft, 60, "twenty seconds: %d", result);
        lcd.DisplayStringAtLine(6, (uint8_t *)buft);
        t.reset();
        result = result + 1;
      }

        thread_sleep_for(100);

    }


}