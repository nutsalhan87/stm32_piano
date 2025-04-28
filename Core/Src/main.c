/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "stdio.h"
#include "inttypes.h"
#include "string.h"
#include "math.h"
#include "user/async.h"
#include "user/messenger.h"
#include "user/util.h"
#include "user/sound.h"
#include "user/tracks.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define NEWLINE '\r'

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

const char *hello_msg = "\r\nWelcome to the programmable musical box!\r\nWrite 1-4 to play built-in tracks. Write 5 to play your custom track. Write Enter to create custom track.\r\n";
const char *track_choosed_msg = "You choosed track: ";
const char *creation_mode_msg = "You've opened creation mode. Write enter to finish creation. Write 0-8 to choose octave. Write space to make pause. Write to append note at the track:\r\n"
  "a to C\r\n"
  "w to C#\r\n"
  "s to D\r\n"
  "e to C#\r\n"
  "d to E\r\n"
  "f to F\r\n"
  "t to F#\r\n"
  "g to G\r\n"
  "y to G#\r\n"
  "h to A\r\n"
  "u to A#\r\n"
  "j to B\r\n";
const char *track_created_msg = "You created a track!\r\n";
const char *octave_changed_msg = "Base octave changed to ";
const char *newline_msg = "\r\n";

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

extern void initialise_monitor_handles(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

async_def(create_track, {
  uint8_t base_octave;
  struct Sample current_sample;
  struct AsyncState_messenger_read m;
  struct AsyncState_messenger_write w;
  struct AsyncState_sound_play_pitch s;
});
async_impl(create_track) async_body({
  start_fn();
  await(1, w, messenger_write, creation_mode_msg, strlen(creation_mode_msg));
  track_custom.length = 0;
  fn_state->base_octave = 4;
  do {
    await(2, m, messenger_read);
    if (fn_state->m.ans >= '0' && fn_state->m.ans <= '8') {
      fn_state->base_octave = fn_state->m.ans - '0';
      await(3, w, messenger_write, octave_changed_msg, strlen(octave_changed_msg));
      await(4, w, messenger_write, &fn_state->m.ans, 1);
      await(5, w, messenger_write, newline_msg, strlen(newline_msg));
    } else if (fn_state->m.ans == NEWLINE) {
      end_fn(6);
    } else if (fn_state->m.ans == ' ') {
      fn_state->current_sample.is_zero = true;
      fn_state->current_sample.pitch.note = A;
      fn_state->current_sample.pitch.octave = 4;
      fn_state->current_sample.duration_modifier = 1.;
      track_custom.length++;
    } else {
      enum Note note;
      switch (fn_state->m.ans) {
      case 'a':
        note = C;
        break;
      case 'w':
        note = Cd;
        break;
      case 's':
        note = D;
        break;
      case 'e':
        note = Dd;
        break;
      case 'd':
        note = E;
        break;
      case 'f':
        note = F;
        break;
      case 't':
        note = Fd;
        break;
      case 'g':
        note = G;
        break;
      case 'y':
        note = Gd;
        break;
      case 'h':
        note = A;
        break;
      case 'u':
        note = Ad;
        break;
      case 'j':      
        note = B;
      }
      fn_state->current_sample.is_zero = false;
      fn_state->current_sample.pitch.note = note;
      fn_state->current_sample.pitch.octave = fn_state->base_octave;
      fn_state->current_sample.duration_modifier = 1.;
      track_custom.samples[track_custom.length] = fn_state->current_sample;
      track_custom.length++;
      await(7, s, sound_play_pitch, track_custom.adsr, fn_state->current_sample.pitch, track_custom.base_note_duration);
    }
  } while (fn_state->m.ans != NEWLINE);
  end_fn(8);
})

async_def(player_control_routine, {
  struct Track **track;
  struct AsyncState_messenger_read m;
  struct AsyncState_messenger_write w;
  struct AsyncState_create_track c;
});
async_impl(player_control_routine) async_body({
  start_fn();
  while (1) {
    await(1, m, messenger_read);
    if (fn_state->m.ans >= '1' && fn_state->m.ans <= '5') {
      if (fn_state->m.ans == '1') {
        *fn_state->track = &track_fade;
      } else if (fn_state->m.ans == '2') {
        *fn_state->track = &track_kuznechik;
      } else if (fn_state->m.ans == '3') {
        *fn_state->track = &track_megalovania;
      } else if (fn_state->m.ans == '4') {
        *fn_state->track = &track_white_roses;
      } else if (fn_state->m.ans == '5') {
        *fn_state->track = &track_custom;
      }
      await(2, w, messenger_write, track_choosed_msg, strlen(track_choosed_msg));
      await(3, w, messenger_write, (*fn_state->track)->name, strlen((*fn_state->track)->name));
      await(4, w, messenger_write, newline_msg, strlen(newline_msg));
    } else if (fn_state->m.ans == NEWLINE) {
      *fn_state->track = NULL;
      await(5, c, create_track);
      *fn_state->track = &track_custom;
      await(6, w, messenger_write, track_created_msg, strlen(track_created_msg));
    }
    yield(7);
  }
})

async_def(player_routine, {
  struct Track **track;
  struct Track *current_track;
  struct AsyncState_sound_play_samples s;
});
async_impl(player_routine) async_body({
  start_fn();
  while (1) {
    while (*fn_state->track == NULL) {
      yield(1);
    }
    fn_state->current_track = *fn_state->track;
    await_until(
      2, 
      *fn_state->track != fn_state->current_track,
      s, 
      sound_play_samples, 
      fn_state->current_track->adsr, 
      fn_state->current_track->samples, 
      fn_state->current_track->length
    );
    sound_volume_off();
    yield(3);
  }
})

async_def(async_main, {
  struct Track *track;
  struct AsyncState_player_routine p;
  struct AsyncState_player_control_routine c;
});
async_impl(async_main) async_body({
  start_fn();
  sound_volume_off();
  fn_state->track = NULL;
  fn_state->p.track = &fn_state->track;
  fn_state->c.track = &fn_state->track;
  while (1) {
    await_two_routines(1, p, player_routine, c, player_control_routine);
  }
})

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  initialise_monitor_handles();

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART6_UART_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */

  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

  printf("Program started!\n");
  
  block_on(m, messenger_write, hello_msg, strlen(hello_msg));

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    block_on(m, async_main);
  }
  printf("Program end!\n");
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 50;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart == &huart6) {
    messenger_read_signal = true;
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart == &huart6) {
	}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
