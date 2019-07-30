#include <DNNRT.h>
#include <SDHCI.h>
#include <Camera.h>

#include <stdio.h>

#include "grayscale.h"

#define WHITE_BALANCE_FRAMES 24
int nframes;
int result;
DNNRT dnnrt;
SDClass SD;

uint8_t buffer[28 * 28] __attribute__((aligned(16)));
DNNVariable input(28 * 28);

void CamCB(CamImage img)
{
  float *input_buffer;
  int label;
  unsigned int i;

  /* Check the img instance is available or not. */

  if (img.isAvailable())
  {
    // Make a gray scale 
    grayscale.shrink(img, buffer);

    // Get buffer
    input_buffer = input.data();

    // Convert  to float
    for (i = 0; i < 28 * 28; i++)
    {
      input_buffer[i] = (float)buffer[i];
    }

    // Set converted input data to DNNRT
    dnnrt.inputVariable(input, 0);

    // Do 
    dnnrt.forward();

    //  result
    DNNVariable output = dnnrt.outputVariable(0);
    // 0: rock
    // 1: scissor
    // 2: paper
    // 3: unknown
    // 4: love
    // 5: six
    Serial.println("rock");
    Serial.println(output[0]);
    Serial.println("scissor");
    Serial.println(output[1]);
    Serial.println("paper");
    Serial.println(output[2]);
    Serial.println("nothing");
    Serial.println(output[3]);
    Serial.println("love");
    Serial.println(output[4]);
    Serial.println("six");
    Serial.println(output[5]);

    label = 3;
    for (i = 0; i < output.size(); i++)
    {
      if (output[i] > 0.8)
      {
        label = i;
        
      }
    }
    result = label;

    Serial.print("RESULT IS ");
    if (result == 0) {
      Serial.println("rock,0000");
      digitalWrite(14, LOW);
      digitalWrite(13, LOW);
      digitalWrite(12, LOW);
      digitalWrite(11, LOW);
    }
    if (result == 1) {
      Serial.println("scissor,0001");
      digitalWrite(14, LOW);
      digitalWrite(13, LOW);
      digitalWrite(12, LOW);
      digitalWrite(11, HIGH);
    }
    if (result == 2) {
      Serial.println("paper,0010");
      digitalWrite(14, LOW);
      digitalWrite(13, LOW);
      digitalWrite(12, HIGH);
      digitalWrite(11, LOW);
    }

    if (result == 3) {
      Serial.println("nothing,0011");
      digitalWrite(14, LOW);
      digitalWrite(13, LOW);
      digitalWrite(12, HIGH);
      digitalWrite(11, HIGH);
    }
    if (result == 4) {
      Serial.println("love,0100");
      digitalWrite(14, LOW);
      digitalWrite(13, HIGH);
      digitalWrite(12, LOW);
      digitalWrite(11, LOW);
    }
    if (result == 5) {
      Serial.println("six,0101");
      digitalWrite(14, LOW);
      digitalWrite(13, HIGH);
      digitalWrite(12, LOW);
      digitalWrite(11, HIGH);
    }
    delay(1000);
    digitalWrite(14, HIGH);
    digitalWrite(13, HIGH);
    digitalWrite(12, HIGH);
    digitalWrite(11, HIGH);


    // Check frame counts for stop auto white balance
    if (nframes < WHITE_BALANCE_FRAMES)
    {
      nframes++;
    }
    else if (nframes == WHITE_BALANCE_FRAMES)
    {
      // Disable auto white balance

      theCamera.setAutoWhiteBalance(false);
    }
  }
}

void setup() {
  // Initialize variables

  nframes = 0;

  /* Open serial communications and wait for port to open */
  digitalWrite(15, HIGH);
  digitalWrite(14, HIGH);
  digitalWrite(13, HIGH);
  digitalWrite(12, HIGH);
  digitalWrite(11, HIGH);
  delay(200);
  pinMode(15, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(11, OUTPUT);
  Serial.begin(115200);
  while (!Serial)
  {
    ; /* wait for serial port to connect. Needed for native USB port only */
  }

  /* Start USB Mass Storage */

  Serial.println("USB Mass Storage start");
  SD.begin();
  SD.beginUsbMsc();

  /* Initialize DNNRT */

  Serial.println("Loading network model");

  File nnbfile("network.nnb");
  if (!nnbfile) {
    Serial.println("nnb not found");
    return;
  }

  Serial.println("Initialize DNNRT");
  int ret = dnnrt.begin(nnbfile);
  Serial.println(ret);
  if (ret < 0)
  {
    Serial.println("DNNRT initialize error.");
  }



  /* begin() without parameters means that
     number of buffers = 1, 30FPS, QVGA, YUV 4:2:2 format */

  Serial.println("Prepare camera");
  theCamera.begin(2);

  /* Start video stream.
     If received video stream data from camera device,
      camera library call CamCB.
  */

  Serial.println("Start streaming");
  theCamera.startStreaming(true, CamCB);

  /* Auto white balance configuration */

  Serial.println("Set Auto white balance parameter");
  theCamera.setAutoWhiteBalanceMode(CAM_WHITE_BALANCE_DAYLIGHT);

}

void loop() {
  // put your main code here, to run repeatedly:

}
