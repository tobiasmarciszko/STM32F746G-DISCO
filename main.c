#include "main.h"
#include "math.h"

static void LCD_Config(void);
static void SystemClock_Config(void);
static void CPU_CACHE_Enable(void);
static void doFire(void);
static void spreadFire(int);

const int WIDTH  = 480;
const int HEIGHT = 272;
const int FIRE_WIDTH  = 480;
const int FIRE_HEIGHT = 120;
static int firePixels[57600];

static uint32_t palette[36] = 
{
  (uint32_t)0xFF070707,
  (uint32_t)0xFF1f0707,
  (uint32_t)0xFF2f0f07,
  (uint32_t)0xFF470f07,
  (uint32_t)0xFF571707,
  (uint32_t)0xFF671f07,
  (uint32_t)0xFF771f07,
  (uint32_t)0xFF8f2707,
  (uint32_t)0xFF9f2f07,
  (uint32_t)0xFFaf3f07,
  (uint32_t)0xFFbf4707,
  (uint32_t)0xFFc74707,
  (uint32_t)0xFFDF4F07,
  (uint32_t)0xFFDF5707,
  (uint32_t)0xFFDF5707,
  (uint32_t)0xFFD75F07,
  (uint32_t)0xFFD7670F,
  (uint32_t)0xFFcf6f0f,
  (uint32_t)0xFFcf770f,
  (uint32_t)0xFFcf7f0f,
  (uint32_t)0xFFCF8717,
  (uint32_t)0xFFC78717,
  (uint32_t)0xFFC78F17,
  (uint32_t)0xFFC7971F,
  (uint32_t)0xFFBF9F1F,
  (uint32_t)0xFFBF9F1F,
  (uint32_t)0xFFBFA727,
  (uint32_t)0xFFBFA727,
  (uint32_t)0xFFBFAF2F,
  (uint32_t)0xFFB7AF2F,
  (uint32_t)0xFFB7B72F,
  (uint32_t)0xFFB7B737,
  (uint32_t)0xFFCFCF6F,
  (uint32_t)0xFFDFDF9F,
  (uint32_t)0xFFEFEFC7,
  (uint32_t)0xFFFFFFFF 
};

static void spreadFire(int src) {
    int pixel = firePixels[src];
    if(pixel == 0) {
        firePixels[src - FIRE_WIDTH] = 0;
    } else {
        int randIdx = rand() % 4;
        int dst = src - randIdx + 1;
        firePixels[dst - FIRE_WIDTH] = pixel - (randIdx & 1);
    }
}

static void doFire() {
    for(int x = 0 ; x < FIRE_WIDTH; x++) {
        for (int y = 1; y < FIRE_HEIGHT; y++) {
            spreadFire(y * FIRE_WIDTH + x);
        }
    }
}

int main(void)
{
  CPU_CACHE_Enable();
  HAL_Init();
  SystemClock_Config();
  BSP_LED_Init(LED1);
  LCD_Config(); 

  uint16_t xpos = 0; 
  uint16_t currLayer = 0;

  BSP_LCD_SelectLayer(0);
  BSP_LCD_Clear(0xFF070707);  
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
          
  BSP_LCD_SelectLayer(1);
  BSP_LCD_Clear(0xFF070707);  
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);

  // Init fire
  for (int i = 0; i < FIRE_WIDTH*FIRE_HEIGHT; i++) {
      firePixels[i] = 0;
  }

  for(int i = 0; i < FIRE_WIDTH; i++) {
      firePixels[(FIRE_HEIGHT-1)*FIRE_WIDTH + i] = 35;
  }

  while(1)
  {
        doFire();        
        currLayer = currLayer == 0 ? 1 : 0;
        
        uint16_t ypos = 0;
        double yposf = 125 + 100 * sin(5*xpos * 3.14159 / 180);
        ypos = (uint16_t)yposf;
        ypos = ypos % HEIGHT;
        xpos = (xpos + 1) % WIDTH;

        BSP_LCD_SelectLayer(currLayer);        
        BSP_LCD_Clear(palette[0]);  
        
        for(int x = 0 ; x < FIRE_WIDTH; x++) {
          for (int y = 0; y < FIRE_HEIGHT; y++) {
            int index = firePixels[y * FIRE_WIDTH + x];
            BSP_LCD_DrawPixel(x, y+152, palette[index]);
          }
        }

        for(int x = xpos ; x < xpos + 50; x++) {
          for (int y = ypos; y < ypos + 50; y++) {
            BSP_LCD_DrawPixel(x, y, palette[35]);
          }
        }
             
        // Wait VSync to avoid tearing
        while (!(LTDC->CDSR & LTDC_CDSR_VSYNCS)) {}
        BSP_LED_Toggle(LED1);
        
        // Switch buffer
        BSP_LCD_SetLayerVisible(currLayer, ENABLE);
        BSP_LCD_SetLayerVisible(!currLayer, DISABLE);
  }
}

static void LCD_Config(void)
{
  BSP_LCD_Init();
  BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);
  BSP_LCD_LayerDefaultInit(1, LCD_FB_START_ADDRESS+(BSP_LCD_GetXSize()*BSP_LCD_GetYSize()*4));
  BSP_LCD_DisplayOn(); 
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 200000000
  *            HCLK(Hz)                       = 200000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 25
  *            PLL_N                          = 400
  *            PLL_P                          = 2
  *            PLL_Q                          = 8
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 6
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  HAL_StatusTypeDef ret = HAL_OK;

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 400;  
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 8;

  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }

  /* Activate the OverDrive to reach the 200 MHz Frequency */
  ret = HAL_PWREx_EnableOverDrive();
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6);
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }
}

/**
  * @brief  CPU L1-Cache enable.
  * @param  None
  * @retval None
  */
static void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}

