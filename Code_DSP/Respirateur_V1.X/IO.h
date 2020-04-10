
#ifndef IO_H
#define	IO_H

/*******************************************************************************
 * LED PORT MAPPING
 ******************************************************************************/
#define LED_BLANCHE _LATA7                         //Led Blanche
#define LED_ROUGE _LATA10                        //Led Orange
#define LED_EMETTEUR _LATC7
#define STEP _LATC6
#define DIR _LATA9
#define ELECTROVANNE _LATB5
#define FIN_COURSE1 PORTBbits.RB3
#define FIN_COURSE2 PORTBbits.RB2

/*******************************************************************************
 * Prototypes fonctions
 ******************************************************************************/
void InitIO();
void LockIO();
void UnlockIO();
void InitCN(void);
#endif	/* IO_H */