#include "ILI9341.h"
#include "pwm.h"
#include "GraphicsColors.h"
#include "main.h"

extern "C"{
  #include "stm32f4xx_delay.h"
}

ILI9341::ILI9341(): TftDriver()
{
InitSysTick();
  configTftFsmc();
  fsmcSetup();
  initTft();
  configBacklightPWM();
  //setBacklightON(333);
  //fadeBacklightON();
}

ILI9341::~ILI9341()
{

}

void ILI9341::configTftFsmc()
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIO D and E clocks */
  RCC_AHB1PeriphClockCmd(
                  RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOA,
                  ENABLE);

  // Create GPIO D Init structure for used pins
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7
                  | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11| GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

  /* Initialize pins */
  GPIO_Init(GPIOD, &GPIO_InitStructure);

  /* Configure GPIO D pins as FSMC alternate functions */
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FSMC); // D0 -> D0
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FSMC); // D1 -> D1
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_FSMC); // NOE -> RD
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FSMC); // NWE -> WR
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource7, GPIO_AF_FSMC); // NE1 -> CS
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FSMC); // D13 -> D13
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FSMC); // D14 -> D14
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FSMC); // D15 -> D14
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource11, GPIO_AF_FSMC); // A16 -> RS
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FSMC); // D14 -> D14
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FSMC); // D15 -> D15

  /* Create GPIO E Init structure for used pin */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | 
                                GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

  /* Initialize GPIOE pins */
  GPIO_Init(GPIOE, &GPIO_InitStructure);

  /* Configure GPIO E pins as FSMC alternate functions */
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource7, GPIO_AF_FSMC); // D4  -> D4
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource8, GPIO_AF_FSMC); // D5  -> D5
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_FSMC); // D6  -> D6
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource10, GPIO_AF_FSMC); // D7  -> D7
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_FSMC); // D8  -> D8
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource12, GPIO_AF_FSMC); // D9  -> D9
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_FSMC); // D10 -> D10
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_FSMC); // D11 -> D11
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource15, GPIO_AF_FSMC); // D12 -> D12

  // Configure RESET pin for TFT
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

  GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void ILI9341::fsmcSetup()
{
  FSMC_NORSRAMInitTypeDef FSMC_NORSRAMInitStructure;
  FSMC_NORSRAMTimingInitTypeDef FSMC_NORSRAMTimingInitStructureRead;
  FSMC_NORSRAMTimingInitTypeDef FSMC_NORSRAMTimingInitStructureWrite;

  /* Enable FSMC Clock */
  RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE);

  /* Define Read timing parameters */
  FSMC_NORSRAMTimingInitStructureRead.FSMC_AddressSetupTime = 1;
  FSMC_NORSRAMTimingInitStructureRead.FSMC_AddressHoldTime = 0;
  FSMC_NORSRAMTimingInitStructureRead.FSMC_DataSetupTime = 15;
  FSMC_NORSRAMTimingInitStructureRead.FSMC_BusTurnAroundDuration = 0;
  FSMC_NORSRAMTimingInitStructureRead.FSMC_CLKDivision = 1;
  FSMC_NORSRAMTimingInitStructureRead.FSMC_DataLatency = 0;
  FSMC_NORSRAMTimingInitStructureRead.FSMC_AccessMode = FSMC_AccessMode_A;

  /* Define Write Timing parameters */
  FSMC_NORSRAMTimingInitStructureWrite.FSMC_AddressSetupTime = 2; // 2
  FSMC_NORSRAMTimingInitStructureWrite.FSMC_AddressHoldTime = 0; // 0
  FSMC_NORSRAMTimingInitStructureWrite.FSMC_DataSetupTime = 5; // 5
  FSMC_NORSRAMTimingInitStructureWrite.FSMC_BusTurnAroundDuration = 0;
  FSMC_NORSRAMTimingInitStructureWrite.FSMC_CLKDivision = 1;
  FSMC_NORSRAMTimingInitStructureWrite.FSMC_DataLatency = 0;
  FSMC_NORSRAMTimingInitStructureWrite.FSMC_AccessMode = FSMC_AccessMode_A;

  /* Define protocol type */
  FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM1; //Bank1
  FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable; //No mux
  FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM; //SRAM type
  FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b; //16 bits wide
  FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode =
                  FSMC_BurstAccessMode_Disable; //No Burst
  FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait =
                  FSMC_AsynchronousWait_Disable; // No wait
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity =
                  FSMC_WaitSignalPolarity_Low; //Don'tcare
  FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable; //No wrap mode
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive =
                  FSMC_WaitSignalActive_BeforeWaitState; //Don't care
  FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable; //Don't care
  FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Enable; //Allow distinct Read/Write parameters
  FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable; //Don't care

  // Set read timing structure
  FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct =
                  &FSMC_NORSRAMTimingInitStructureRead;

  // Set write timing structure
  FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct =
                  &FSMC_NORSRAMTimingInitStructureWrite;

  // Initialize FSMC for read and write
  FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);

  // Enable FSMC
  FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);
}

void ILI9341::initTft()
{
  tftReset();

  sendCommand(0x01);
  sendCommand(0x28);
  
  Delay(5);
  
  sendCommand(0xcf);
  sendRegister(0x0000);
  sendRegister(0x0083);
  sendRegister(0x0030);

  sendCommand(0xed);
  sendRegister(0x0064);
  sendRegister(0x0003);
  sendRegister(0x0012);
  sendRegister(0x0081);

  sendCommand(0xe8);
  sendRegister(0x0085);
  sendRegister(0x0001);
  sendRegister(0x0079);

  sendCommand(0xcb);
  sendRegister(0x0039);
  sendRegister(0x002c);
  sendRegister(0x0000);
  sendRegister(0x0034);
  sendRegister(0x0002);

  sendCommand(0xf7);
  sendRegister(0x0020);

  sendCommand(0xea);
  sendRegister(0x0000);
  sendRegister(0x0000);

  sendCommand(0xc0);
  sendRegister(0x0026);

  sendCommand(0xc1);
  sendRegister(0x0011);

  sendCommand(0xc5);
  sendRegister(0x0035);
  sendRegister(0x003e);

  sendCommand(0xc7);
  sendRegister(0x00be);

  sendCommand(0x36);
  sendRegister(0x0048);//48

  sendCommand(0x3a);
  sendRegister(0x0055);

  sendCommand(0xb1);
  sendRegister(0x0000);
  sendRegister(0x0010); // default 0x1B

  sendCommand(0xF2);
  sendRegister(0x0008);

  sendCommand(0x26);
  sendRegister(0x0001);

  sendCommand(TFT_REG_GAMMA_1);
  sendRegister(0x001f);
  sendRegister(0x001a);
  sendRegister(0x0018);
  sendRegister(0x000a);
  sendRegister(0x000f);
  sendRegister(0x0006);
  sendRegister(0x0045);
  sendRegister(0x0087);
  sendRegister(0x0032);
  sendRegister(0x000a);
  sendRegister(0x0007);
  sendRegister(0x0002);
  sendRegister(0x0007);
  sendRegister(0x0005);
  sendRegister(0x0000);

  sendCommand(TFT_REG_GAMMA_1);
  sendRegister(0x0000);
  sendRegister(0x0025);
  sendRegister(0x0027);
  sendRegister(0x0005);
  sendRegister(0x0010);
  sendRegister(0x0009);
  sendRegister(0x003a);
  sendRegister(0x0078);
  sendRegister(0x004d);
  sendRegister(0x0005);
  sendRegister(0x0018);
  sendRegister(0x000d);
  sendRegister(0x0038);
  sendRegister(0x003a);
  sendRegister(0x001f);

  sendCommand(TFT_REG_COL);
  sendRegister(0x0000);
  sendRegister(0x0000);
  sendRegister(0x0000);
  sendRegister(0x00ef);

  sendCommand(TFT_REG_PAGE);
  sendRegister(0x0000);
  sendRegister(0x0000);
  sendRegister(0x0001);
  sendRegister(0x003f);

  sendCommand(TFT_REG_TE_ON);
  sendRegister(0x0000);

  sendCommand(0xb7);
  sendRegister(0x0007);

  sendCommand(0xb6);
  sendRegister(0x000a);
  sendRegister(0x0082);
  sendRegister(0x0027);
  sendRegister(0x0000);

  sendCommand(0x11);

  Delay(100);

  sendCommand(0x29);
  
  Delay(100);

  sendCommand(TFT_REG_MEM_WRITE);
  /*
  unsigned int colorGrad = 0xFFFF;

  for(int i = 0; i < 76800; i++)
  {
    if( colorGrad == 0x0000 )
    {
      colorGrad = 0xFFFF;
    }
    sendData( --colorGrad );
  }*/
  
  // LATEST EXAMPLE CODE FOR WINDOWING
  /*
  for (int i = 0; i < 76800; i++)
  {
    sendData(0xFFFF);
  }

  setWindow( 
    50, //0x000A, 
    200, //0x0014, 
    50, //0x000A, 
    100 //0x0014 // sets the page 
  ); 

  for (int i = 0; i < 76800; i++)
  {
    sendData(PINK);
  }*/
}

void ILI9341::configBacklightPWM()
{
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  TIM_OCInitTypeDef TIM_OCInitStructure;

  /* TIM Config */

  GPIO_InitTypeDef GPIO_InitStructure;

  /* TIM4 Clock Enable */

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

  /* LEDs and GPIO */

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

  /*  TODO: Find the correct PIN for PWM */

  GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_TIM4);

  /* PWM Setup */

  // Compute the Prescaler Value
  PrescalerValue = (uint16_t) ((SystemCoreClock / 2) / 21000000)
    - 1;
  
  /* Time base config */

  TIM_TimeBaseStructure.TIM_Period = 665;
  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

  /* PWM1 Mode configuration: Channel 1 */

  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 0;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  TIM_OC1Init(TIM4, &TIM_OCInitStructure);

  TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);

  /* PWM1 Mode Configuration: Channel 2 */

  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 0;

  TIM_ARRPreloadConfig(TIM4, ENABLE);

  /* TIM4 Enable Counter */

  TIM_Cmd(TIM4, ENABLE);
}

void ILI9341::tftReset()
{
  RESET_PORT->BSRRL = RESET_PIN;
  Delay(10);
  RESET_PORT->BSRRH = RESET_PIN;
  Delay(15);
  RESET_PORT->BSRRL = RESET_PIN;
  Delay(120);
}

void ILI9341::sendRegister(unsigned int reg)
{
  pushDataRam(reg);
}

void ILI9341::sendCommand(unsigned int data)
{
  pushDataReg(data);
}

void ILI9341::sendData(unsigned int data)
{
  pushDataRam(data);
}

void ILI9341::pushDataReg(unsigned int data)
{
  LCD_REG = data;
}

void ILI9341::pushDataRam(unsigned int data)
{
  LCD_RAM = data;
}

void ILI9341::enterSleep()
{
  sendCommand(0x28);
  Delay(20);
  sendCommand(0x10);
}

void ILI9341::exitSleep()
{
  sendCommand(0x11);
  Delay(120);
  sendCommand(0x29);
}

void ILI9341::setOrientation(unsigned int HV)
{
  sendCommand(0x03);
  if(HV==1)//vertical
  {
    sendData(0x5038);
  }
  else//horizontal
  {
    sendData(0x5030);
  }
  sendCommand(0x0022); //Start to write to display RAM
}

void ILI9341::setCol(uint16_t startX, uint16_t startY)
{
  sendCommand(0x2A);
  sendRegister(startX);
  sendRegister(startY);
}

void ILI9341::setPage(uint16_t endX, uint16_t endY)
{
  sendCommand(0x2B);
  sendRegister(endX);
  sendRegister(endY);
}

void ILI9341::setXY(uint16_t poX, uint16_t poY)
{
  //setCol(poX, poX);
  setPage(poY, poY);
  sendCommand(TFT_REG_MEM_WRITE);
}

void ILI9341::setWindow(uint16_t startX, uint16_t endX, uint16_t startY, uint16_t endY)
 {
    /*
    sendCommand(0x2a);
    
    sendRegister((startX >> 8) & 0xFF);
    sendRegister(startX & 0xFF);
    sendRegister((endX >> 8) & 0xFF);
    sendRegister(startY & 0xFF);
  
    sendCommand(0x2b);
    sendRegister((startY >> 8) & 0xFF);
    sendRegister(endX & 0xFF);
    sendRegister((endY >> 8) & 0xFF);
    sendRegister(endY >> 8);
    */
    sendCommand(TFT_REG_COL);
    sendRegister(startX >> 8);
    sendRegister(startX);
    sendRegister(endX >> 8);
    sendRegister(endX);
    sendCommand(TFT_REG_PAGE);
    sendRegister(startY >> 8);
    sendRegister(startY);
    sendRegister(endY >> 8);
    sendRegister(endY);


    sendCommand(TFT_REG_MEM_WRITE);
 }

void ILI9341::fillRectangle(uint16_t poX, uint16_t poY,
                                uint16_t width, uint16_t length,
                                uint16_t color)
{
  uint32_t windowSize = length * width;
  uint32_t count = 0;

  setWindow(poX, (poX + width), poY, (poY + length));
  /*for( uint32_t i = 0; i <= windowSize; i++ )
  {
    sendData(color);
    count++;
  }
  sendData(color);  */
  int i = 0;
  int j = 0;
  for( i = 0; i < length; i++ )
  {
    for( j = 0; j < width + 100; j++ )
    {
      sendData(color);
    }
  }
}

void ILI9341::drawHorizontalLine(uint16_t poX, uint16_t poY, uint16_t length,
                        uint16_t color)
{
  unsigned int i;
  if(poX <0)
    poX = 0;
  if(poX>239)
    poX = 239;
 
  if(poY <0)
    poY = 0;
  if(poY>319)
    poY = 319;
 
  setXY(poX,poY);
  //tft.setOrientation(0);
  
  if(length+poX>MAX_X)
  {
    length=MAX_X-poX;
  }
  for(i=0;i<length;i++)
  {
    sendData(color);
  }
}

void ILI9341::setBacklightON()
{
  TIM4->CCR1 = 333;
}

void ILI9341::setBacklightOff()
{
  TIM4->CCR1 = 0;
}

void ILI9341::fadeBacklightON(uint16_t duration)
{
  int brightness = 333;

  while(brightness > 0)
  {
    brightness--;

    TIM4->CCR1 = 333 - (brightness + 0) % 333;

    Delay(20);
  }
}

void ILI9341::fadeBacklightOFF(uint16_t duration)
{
  int brightness = 0;

  while(brightness < PWM_DUTY_CYLCE_MAX - 1)
  {
    brightness++;

    TIM4->CCR1 = 333 - (brightness + 0) % 333;

    Delay(20);
  }

  TIM4->CCR1 = 0;
}

void ILI9341::fadeBacklight_DownHalf()
{
  int brightness = 0;
}

void ILI9341::fadeBacklight_UpHalf()
{
  int brightness = 0;
}