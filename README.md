# stm32f107vct6_chinese

![Image of stm32f107vct6 chinese](https://github.com/peakhunt/stm32f107vct6_chinese/blob/master/imgs/stm32f107vct6_chinese.jpg)

if you got hold of this board like me, you are unlucky. there is no board schematic available.
Here are the list of things I found during some sort of hobby grade reverse engineering.

# LED
* D2    - PE9
* D3    - PE11
* D4    - PE13
* D5    - PE15

LED is connecte to 3.3V source via current limiting resistor. 
Setting GPIO LOW turns on the LED.

# Ethernet
I believe most people bought this board because of ethernet interface.
Basically it works quite okay except one glitch.

The glitch is MDIO isn't connected to PA2. This won't normally prevent you from using ethernet but it's quite odd that MDIO is not connected while MDC(PC1) is. If you are like me, you won't like the situation and wanna fix it. Actually the fix is quite simple. Take a look at the picture below.

![Image of MDIO fix](https://github.com/peakhunt/stm32f107vct6_chinese/blob/master/imgs/IMG_20170916_191742.jpg)

As shown in the picture, just connect R42 ( MDIO Pull UP) to PA2 pin header.

One funny thing was there was a STM32CubeMX problem with properly working PHY. With original CubeMX code (latest version as of Sep/16/2017), if the boards boots up with ethernet link down, the ethernet simple doesn't work. It turned out that the problem was caused by not calling netif_set_up()/netif_set_down() when handling link up/down events. Again fix is quite simple. Just take a look at ethernetif_set_link() in Src/ethernetif.c

The PHY(DP83848IVV) is connected via RMII. 50Mhz clock to the PHY is provided via MCO pin of STM32F107VCT6.
And here is the list of pin assignments for ethernet connection

* MDC     - PC1
* MDIO    - PA2
* REF_CLK - PA1
* TX_EN   - PB11
* CRS_DV  - PD8
* RXD0    - PD9
* RXD1    - PD10
* TXD0    - PB12
* TXD1    - PB13

Just don't forget about 50 Mhz clock output from MCO (PA8)

# More on later!

