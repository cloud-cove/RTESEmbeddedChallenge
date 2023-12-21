// embedded challenge
// luigia than, shelby hou, john choi, shreya nalla

#include "mbed.h"

// variable declarations
//*************Data Storage*****************

//arrays to store 20 seconds of x y z data sampled every 250 ms
float ang_x[80];
float ang_y[80];
float ang_z[80];
//array to calc the integration with
float lin_velocity;
float integrate[80];
//track index
volatile uint16_t ang_ind = 0;

//length of the users legs for our approximation
uint16_t userheight = 30; //in inches

//*************SPI intialization*****************
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

//*************LCD screen intialization*****************
// LCD initialization
#include "drivers/LCD_DISCO_F429ZI.h"
#include "drivers/TS_DISCO_F429ZI.h"
LCD_DISCO_F429ZI lcd;
TS_DISCO_F429ZI ts;

//LCD display buffers
char bufx[60];
char bufy[60];
char bufz[60];
char buft[60];
char bufl[60];
float result = 0;

bool start_flag = false;

//timer for 20s result
Timer t;
volatile uint32_t elapsed_time;
//timer for between steps
Timer stept;
volatile uint32_t step_time;


//*************main*****************
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

    //LCD initialization
    lcd.Clear(LCD_COLOR_LIGHTBLUE);
    lcd.SetTextColor(LCD_COLOR_BLACK);
    lcd.DisplayStringAtLine(0, (uint8_t *)"Step Counter 3000");


    TS_StateTypeDef TS_State;
    uint8_t status;
    status = ts.Init(lcd.GetXSize(), lcd.GetYSize());
    char x;
    char y;
    char text[60];
    char userheighttext[60];


    //**************start screen****************
    //we could also change this to just height and divide in half roughly
    lcd.DisplayStringAtLine(1, (uint8_t *)"Enter height of hips");
    lcd.DisplayStringAtLine(4, (uint8_t *)"  /\\  ");
    lcd.DisplayStringAtLine(5, (uint8_t *)" /  \\ ");
    lcd.DisplayStringAtLine(6, (uint8_t *)"  ||  ");
    sprintf((char*)userheighttext, "%d inches", userheight);
    lcd.DisplayStringAt(1, LINE(8), (uint8_t *)&userheighttext, CENTER_MODE);
    lcd.DisplayStringAtLine(10, (uint8_t *)"  ||  ");
    lcd.DisplayStringAtLine(11, (uint8_t *)" \\  / ");
    lcd.DisplayStringAtLine(12, (uint8_t *)"  \\/  ");
    lcd.DisplayStringAtLine(16, (uint8_t *)"[ C O N F I R M ]");

    //*************main loop*****************
    while (1){

      //**************detect touch****************
      ts.GetState(&TS_State);      
      
      //**************detect start button****************
      if (TS_State.TouchDetected && start_flag == false)
      {
        x = TS_State.X;
        y = TS_State.Y;
        sprintf((char*)text, "x=%d y=%d    ", x, y);
        lcd.DisplayStringAt(1, LINE(18), (uint8_t *)&text, LEFT_MODE);
        //up button is pressed
        if ( (y<250 && y>200) && (x<50) ){
          userheight = userheight + 6;
          sprintf((char*)userheighttext, "%d inches", userheight);
          lcd.DisplayStringAt(1, LINE(8), (uint8_t *)&userheighttext, CENTER_MODE);
        }
        //down button is pressed
        if ( (y<135 && y>95) && (x<50) ){
          userheight = userheight - 6;
          sprintf((char*)userheighttext, "%d inches", userheight);
          lcd.DisplayStringAt(1, LINE(8), (uint8_t *)&userheighttext, CENTER_MODE);
        }
        //start button is pressed
        if (y < 80 && y > 30){
          start_flag = true;
          lcd.Clear(LCD_COLOR_LIGHTGREEN);
          //intialize timer to track when 20s have passed
          t.start();
          stept.start();
        }
      }
      
      if(start_flag == true){
        //*************gyroscope readings*****************
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
        step_time = stept.read_ms();
        stept.reset();

        //printf("RAW|\tgx: %d \t gy: %d \t gz: %d\t",raw_gx,raw_gy,raw_gz);
        //unit conversion to radians per second
        gx=((float)raw_gx)*(17.5f*0.017453292519943295769236907684886f / 1000.0f);
        gy=((float)raw_gy)*(17.5f*0.017453292519943295769236907684886f / 1000.0f);
        gz=((float)raw_gz)*(17.5f*0.017453292519943295769236907684886f / 1000.0f);

        //store values of angular velocity
        ang_x[ang_ind] = gx;
        ang_y[ang_ind] = gy;
        ang_z[ang_ind] = gz;

        //prep text for LCD
        snprintf(bufx, 60, "gx: %4.5f",gx);
        snprintf(bufy, 60, "gy: %4.5f",gy);
        snprintf(bufz, 60, "gz: %4.5f",gz);
        //display text
        lcd.DisplayStringAtLine(2, (uint8_t *)bufx);
        lcd.DisplayStringAtLine(3, (uint8_t *)bufy);
        lcd.DisplayStringAtLine(4, (uint8_t *)bufz);

        //*************math for distance*****************
        //multiply each reading of z by the length of the leg (pendulum approximation)
        //also multiply by amount of time between each reading (linear velocity)
        //store results in another array to calculate the area under the curve (integration)
        lin_velocity = fabs(ang_z[ang_ind]) * float(userheight);

        //float lin_velocity_dis = lin_velocity ; 
        //snprintf(bufl, 60, "lin velocity: %f ft/s",lin_velocity);
        //lcd.DisplayStringAtLine(5, (uint8_t *)bufl);

        // linear velocity * time = distance per step * convert from in/ms to m/s
        integrate[ang_ind] = fabs(lin_velocity) * step_time * (0.0254/1000.0); 
        ang_ind = ang_ind + 1;
        if (ang_ind >= 80){
          ang_ind = 0;
        }

        //checking if twenty seconds have passed
        elapsed_time=t.read_ms();
        if (elapsed_time >= 20000){

          //sum area under the curve array
          uint32_t sum_array = 0;
          for(int i = 0; i < 80; i++){
            sum_array = sum_array + integrate[i];
          }

          //text on LCD
          snprintf(buft, 60, "distance in 20s: %f m", sum_array);
          lcd.DisplayStringAtLine(6, (uint8_t *)buft);

          //reset timer
          t.reset();
          
          //reset 
          result = result + 1;

        }
      }
      thread_sleep_for(250);
      

    }


}