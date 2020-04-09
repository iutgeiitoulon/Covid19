#include <stdio.h>
#include <stdlib.h>
#include "p33Fxxxx.h"
#include <libpic30.h>
#include "pragma_config.h"
#include "main.h"
#include "oscillator.h"
#include "IO.h"
#include "timer.h"
#include "PWM.h"

int main(void)
{
    InitOscillator();

    /****************************************************************************************************/
    // Configuration des entrées sorties
    /****************************************************************************************************/
    InitIO();
    InitTimer1(); //Génération des pulses de l'émetteur infrarouge
    InitTimer2(); //affichage des leds de la balise
    
    InitPWM();
    InitPWM2();
    
    LED_BLANCHE = 1;
    LED_ROUGE = 1;

    /****************************************************************************************************/
    // Boucle Principale
    /****************************************************************************************************/
    for (;;)
    {

            } // fin main
}

void OperatingSystemLoop(void)
{

}