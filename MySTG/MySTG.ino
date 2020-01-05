#include <wifiboypro.h>
#include <Arduino.h>
#include "wb-sprite.h"

uint8_t starsX[50];            //宣告陣列星星位置X
uint16_t starsY[50];           //宣告陣列星星位置Y
uint8_t starsSpeed[50];        //宣告陣列星星垂直速度
uint16_t starsColor = 65535;    //宣告星星顏色並設為白色
const uint8_t buttonLeft = 33;  //宣告向左移動的按鍵腳位為常量
const uint8_t buttonRight = 35; //宣告向右移動的按鍵腳位為常量
int masterX = 112;              //宣告主角位置X
uint8_t masterStatus = 0;       //宣告主角狀態
int enemyX[10];                 //宣告敵方位置X
int enemyY[10];                 //宣告敵方位置Y
unsigned long currentTime;      //宣告現在時間的變數
unsigned long enemySpawnCD;     //宣告敵人生成時間的變數
bool enemyAlive[10];            //宣告控制敵機存活的陣列
unsigned int enemyNo = 0;       //宣告為敵機編號的變數
int score = 0;                  //宣告分數變數
uint8_t life = 3;               //宣告生命
unsigned long soundStop;        //宣告控制音樂停止的變數
uint8_t sceneStatus;            //宣告控制場景的變數

void blit_str256(const char *str, int x, int y)
{
    for (int i = 0; i < strlen(str); i++)
    {
        if (str[i] >= '@' && str[i] <= ']')
            wbpro_blitBuf8(8 * (str[i] - '@'), 0, 240, x + i * 8, y, 8, 8, (uint8_t *)sprites);
        if (str[i] >= '!' && str[i] <= '>')
            wbpro_blitBuf8(8 * (str[i] - '!'), 8, 240, x + i * 8, y, 8, 8, (uint8_t *)sprites);
        if (str[i] == '?')
            wbpro_blitBuf8(8 * 14, 16, 240, x + i * 8, y, 8, 8, (uint8_t *)sprites);
        if (str[i] == 'c')
            wbpro_blitBuf8(8 * 13, 16, 240, x + i * 8, y, 8, 8, (uint8_t *)sprites);
        if (str[i] == 'w')
            wbpro_blitBuf8(7, 16, 240, x + i * 8, y, 26, 8, (uint8_t *)sprites);
        if (str[i] == 'x')
            wbpro_blitBuf8(42, 16, 240, x + i * 8, y, 61, 8, (uint8_t *)sprites);
    }
}

void blit_num256(uint16_t num, uint16_t x, uint16_t y, uint8_t color_mode)
{
    uint16_t d[5];

    d[0] = num / 10000;
    d[1] = (num - d[0] * 10000) / 1000;
    d[2] = (num - d[0] * 10000 - d[1] * 1000) / 100;
    d[3] = (num - d[0] * 10000 - d[1] * 1000 - d[2] * 100) / 10;
    d[4] = num - d[0] * 10000 - d[1] * 1000 - d[2] * 100 - d[3] * 10;

    for (int i = 0; i < 5; i++)
    {
        wbpro_blitBuf8(d[i] * 8 + 120, color_mode * 8, 240, x + i * 8, y, 8, 8, (uint8_t *)sprites); //將d[0]~d[4]逐個顯示並排列
    }
}

void SetupSound()
{
    pinMode(17, OUTPUT);  //設定蜂鳴器的腳位為輸出
    ledcSetup(1, 400, 8); //ledcSetup(設定頻道1的頻率為400解析度為8位元)
    ledcAttachPin(25, 1); //ledcAttachPin(將腳位25定義為頻道1)
}

void MakeSound(int volume)
{
    ledcWrite(1, volume); //寫入音量
}

void SoundFreq(int freq)
{
    ledcSetup(1, freq, 8); //寫入音高
}

void setup()
{
    currentTime = millis(); //現在時間為經過時間

    wbpro_init();
    wbpro_initBuf8();
    SetupSound();
    for(int i=0; i<256; i++)
       wbpro_setPal8(i, wbpro_color565(standardColour[i][0],standardColour[i][1],standardColour[i][2]));

    wbpro_fillScreen(wbBLACK); //設定背景(黑色)

    for (int i = 0; i < 50; i++) //宣告區域變數i = 0;當i < 100時執行;執行完畢i+1
    {
        starsX[i] = random(0, 240); //於第i個星星位置X隨機一個整數(0~240)
        starsY[i] = random(0, 320);
        starsSpeed[i] = random(2, 5);
       
        if (i < 10)       //加到10前執行以下
        {
            enemyX[i] = random(0, 229); //初始化敵機的X
            enemyY[i] = -13;            //初始化敵機的Y
        }
    }
    pinMode(22,OUTPUT);
    pinMode(buttonLeft, INPUT);
    pinMode(buttonRight, INPUT);
    pinMode(23, OUTPUT);
    // enemySpawnCD = currentTime + 1000; //讓下台敵機出生CD為1秒後
    // enemyAlive[0] = true;              //設置第一個敵機為存活
    pinMode(21, OUTPUT);
    pinMode(26, OUTPUT);
    // bulletSpawnCD = currentTime; //讓第一顆子彈馬上到出生時間
}

void loop()
{
    wbpro_clearBuf8();
    currentTime = millis(); //當前時間在每禎以毫秒更新

    blit_str256("FRUIT", 0, 0);
    blit_num256(score, 60, 0, 1);
    for (int i = life; i > 0; i--)
        wbpro_blitBuf8(0, 0, 20, 239 - i * 25, 0, 20, 20, (uint8_t *)life2); //wbpro_blitBuf8(int x:我們把最右側的生命圖示顯示出來後就能自動往左增加剩餘生命)

    for (int i = 0; i < 50; i++)
    {
        wbpro_setBuf8(starsX[i] + starsY[i] * 240, starsColor); //(於buffer設定顯示的位置(0~76799),顏色)
        starsY[i] =  starsY[i] +1.5*starsSpeed[i];                            //第i個星星位置Y以第i個星星速度增加
        starsX[i] = starsX[i] +1.5*starsSpeed[i]; 
        if (starsY[i] >= 320)                                  //當第i個星星位置Y超過320
            starsY[i] = 0;                                     //回到0
        if (starsX[i] >= 240)                                  //當第i個星星位置Y超過320
            starsX[i] = 0;
    }
    
    // if (life > 0)
    // {
    //     MasterCtrl();
    //     EnemyCtrl();
    //     Collision();
    // }
    SceneCtrl();
    if (currentTime > soundStop)
    {
        MakeSound(0); //於停止時間把音量降為0
        digitalWrite(26,LOW);
    }

    // wbpro_blitBuf8(0, 0, 240,0,0,240,320, (uint8_t *)sprites2);
    wbpro_blit8();
}

void MasterCtrl()
{
    switch (masterStatus)
    {
    case 0: //待機
        PlayerMovement();
       
        break;

    case 1: //移動
        PlayerMovement();
        
        break;

    case 2: //死亡

        break;
    }
}

void PlayerMovement()
{
    wbpro_blitBuf8(0, 0, 32,  masterX, 299, 32, 32, (uint8_t *)cha);
    //wbpro_drawImage(masterX, 299, 16, 21, (uint16_t *)img16bit); //將主角顯示出來
    if (digitalRead(buttonLeft) == 0)                                    //按下I/O33時
    {
        masterX -= 8;     //主角左移8像素
        masterStatus = 1; //主角進入移動狀態
    }
    if (masterX < 0)
        masterX = 0;

    if (digitalRead(buttonRight) == 0)
    {
        masterX += 8; //主角右移8像素
        masterStatus = 1;
    }
    if (masterX > 224)
        masterX = 224;

    if (digitalRead(buttonLeft) == 1 && digitalRead(buttonRight) == 1) //兩個按鍵都鬆開時
        masterStatus = 0;                                              //主角進入待機狀態
}

void EnemyCtrl()
{
    if (currentTime >= enemySpawnCD) //當現在時間到達敵機出生的CD時
    {
        enemyAlive[enemyNo] = true; //敵機設為存活
        if (enemyNo < 9)
            enemyNo += 1; //敵機編號+1
        else
            enemyNo = 0; //敵機編號回到0

        enemySpawnCD = currentTime + random(1000, 1500); //設置下一個敵機出生的時間
    }
    for (int i; i < 10; i++)
    {
        if (enemyAlive[i]) //當本敵機存活時
        {
            enemyY[i] += 3; //設定向下飛的速度
             wbpro_blitBuf8(0, 0, 32,  enemyX[i], enemyY[i], 32, 32, (uint8_t *)fru);
           // wbpro_drawImage( enemyX[i], enemyY[i], 22, 22, (uint16_t *)img16bit2); //水果
            if (enemyY[i] > 320) //當飛出畫面底部時
            {
                life--;
                enemyX[i] = random(0, 229); //重新設置X位置
                enemyY[i] = -13;            //回到螢幕上方
                enemyAlive[i] = false;      //設定本敵機為未存活
            }
        }
        else          //當本敵機未存活時
            continue; //繼續執行
    }
}



void Collision()
{
  
  for (int j = 0; j < 10; j++){
     if (enemyY[j] >= 288 && enemyX[j] + 10 >= masterX && enemyX[j] <= masterX + 15)
     {
      score+=10;
      enemyAlive[j] = false;
       enemyX[j] = random(0, 229);
       enemyY[j] = -13;
       SoundFreq(200);
       MakeSound(30);
       soundStop = currentTime + 100;
     }
   
  }
}
void SceneCtrl()
{
    switch (sceneStatus)
    {
    case 0: //遊戲開始
        if (digitalRead(buttonLeft) == 0 || digitalRead(buttonRight) == 0)
        {
            enemySpawnCD = currentTime + 1000;
            enemyAlive[0] = true;
           
            sceneStatus++;
        }
        wbpro_blitBuf8(0, 0, 32,  105, 100, 32, 32, (uint8_t *)cha);
        wbpro_blitBuf8(0, 0, 32,  105, 80, 32, 32, (uint8_t *)fru);
        blit_str256("FRUIT CATCHER",70, 150);
        blit_str256("PRESS L OR A", 71, 200);
        digitalWrite(23,LOW);
        digitalWrite(22,LOW);
        digitalWrite(21,HIGH);
        break;

    case 1: //遊戲中
        blit_str256("FRUIT", 0, 0);
        blit_num256(score, 60, 0, 1);
        currentTime = millis();
        MasterCtrl();
        Collision();
        EnemyCtrl();
        digitalWrite(23,HIGH);
        digitalWrite(22,LOW);
        digitalWrite(21,0);
        if (life <= 0)
        { //當life到達0時先將所有敵機與子彈從場上清除再到下一個場景
            enemyNo = 0;
           
            for (int i = 0; i < 100; i++)
            {
                if (i < 10)
                {
                    enemyAlive[i] = false;
                    enemyY[i] = -13;
                }
               
            }
            sceneStatus++;
            delay(500); //給一個小延遲產生結束的感受
        }
        if(score==100){
          enemyNo = 0;
           
            for (int i = 0; i < 100; i++)
            {
                if (i < 10)
                {
                    enemyAlive[i] = false;
                    enemyY[i] = -13;
                }
               
            }
          sceneStatus+=2;
            delay(1000); //給一個小延遲產生結束的感受
        }
        break;

    case 2: //遊戲結束
        if (digitalRead(buttonLeft) == 0 || digitalRead(buttonRight) == 0)
        { //當回到遊戲場景時要將數值初始化
            life = 3;
            enemySpawnCD = currentTime + 1000;
            enemyAlive[0] = true;
           
            sceneStatus = 1;
            score = 0;
        }
        MakeSound(0);
        wbpro_blitBuf8(0, 0, 32,  105, 70, 32, 32, (uint8_t *)cha);
        wbpro_blitBuf8(0, 0, 32,  105, 50, 32, 32, (uint8_t *)fru);
        blit_str256("FRUIT CATCHER",70, 120);
        blit_str256("PRESS L OR A", 71, 180);
        blit_str256("GAMEOVER", 87, 160);
        blit_str256("YOUR FRUIT", 79, 230);
        blit_num256(score, 99, 250, 1);
        digitalWrite(23,LOW);
        digitalWrite(22,LOW);
        digitalWrite(21,0);
        break;
   
    case 3:
        if (digitalRead(buttonLeft) == 0 || digitalRead(buttonRight) == 0)
        { //當回到遊戲場景時要將數值初始化
            life = 3;
            enemySpawnCD = currentTime + 1000;
            enemyAlive[0] = true;
           
            sceneStatus = 1;
            score = 0;
        }
        MakeSound(0);
        wbpro_blitBuf8(0, 0, 32,  105, 70, 32, 32, (uint8_t *)cha);
        wbpro_blitBuf8(0, 0, 32,  105, 50, 32, 32, (uint8_t *)fru);
        blit_str256("FRUIT CATCHER",70, 120);
        blit_str256("PRESS L OR A", 71, 180);
        blit_str256("YOU WIN", 87, 160);
        blit_str256("YOUR FRUIT", 79, 230);
        blit_num256(score, 99, 250, 1);
        digitalWrite(23,LOW);
        digitalWrite(22,LOW);
        digitalWrite(21,0);
        break;
     }
}
