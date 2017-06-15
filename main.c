#include "main.h"



///Guarda o estado da máquina de estados.
int8 estado;

///Contador para controlar a base de tempo.
int16 cont;

//Tempo em minutos para acionamento do alarme.
int8 tempo;

///Indica que houve transicao de estado.
int1 trans;

///Indica que deverá piscar o LED dentro do estado aguardando ignicao.
int1 piscar_simular_alarme;


///Estado em que o alarme está desligado e o carro pode funcionar normalmente.
#define ALARME_DESLIGADO 1

///Estado em que o alarme está piscando aguardando a ignição para acionar a sirene e o relé.
#define AGUARDANDO_IGNICAO 2

///O tempo selecionado está sendo incrementado\contado.
#define CONTANDO_TEMPO 3

///O tempo acabou, começa a piscar LED de aviso de sirene.
#define TEMPO_FINALIZADO 4

///O alarme está disparado e somente a chave secreta pode desliga-lo.
#define ALARME_DISPARADO 5

///A sirene de aviso está tocando e o relé irá bater em alguns segundos.
#define SIRENE_AVISO 6

///Valor de 1 minuto em milissegundos.
#define _1_MINUTO 60000

///Passo de quanto o contador sera incrementado. Deve ser pequeno para permitir leitura dos sensores.
#define BASE_TEMPO_CONTADOR 20

///Tempo antes de disparar o alarme que o LED ira piscar.
#define TEMPO_INICIO_PISCAGEM 5000

///Tempo em que a sirene será tocada antes de desligar o motor.
#define TEMPO_SIRENE_AVISO 3000





/**@brief Realiza a mudança de estado do sistema.
Esta função seta a variável trans que indicará que a mudança de estado ocorreu.
@param est_futuro Estado para o qual o sistema irá ao passar novamente no loop da máquina de estados.*/
void muda_estado(int8 est_futuro){
	trans = 1;
	estado = est_futuro;
}






/**@brief  o contador de tempo de 1 minuto.
Esse contador conta intervalos de 1 minuto. Ao zera-lo, está se evitando overflow aos
65535 da variável de 16bits que realiza a contagem.
A base de tempo de contagem de tempos maiores é de 1 minuto.*/
void zera_contador(void){
	//O contador conta 1 minuto.
	cont = 	_1_MINUTO / BASE_TEMPO_CONTADOR;
}





/**@brief Le a configuracao de  JUMPERS e detemina quanto tempo o alarme
ira demorar para ligar.
\n
A seguinte configuração pode ser determinada pelos jumpers: \n
- 1-ON   2-ON    = 1 minuto \n
- 1-ON   2-OFF   = 3 minutos \n
- 1-OFF  2-ON    = 5 minutos \n
- 1-OFF  2-OFF   = 10 minutos
*/
void determina_tempo(void){
	if(!input(JP1)){
		if(!input(JP2)){
			tempo = 1;
		}
		else{
			tempo = 3;
		}
	}
	else{
		if(!input(JP2)){
			tempo = 5;
		}
		else{
			tempo = 10;
		}
	}
	zera_contador();
}





/**@brief Indica que a porta esta aberta ou fechado.
@return 1 se a porta está aberta.*/
int1 porta_aberta(void){
	return (!input(SENSOR_PORTA));
}

/**@brief Indica se a chave secreta está sendo pressionada.
@return  1 se a chave está sendo pressionada.*/
int1 apertou_chave_secreta(void){
	return (!input(CHAVE_SECRETA));
}


/**@brief Indica se a ignicao está acionada.
@return  1 se a ignicao está acionada.*/
int1 ignicao_acionada(void){
	return (!input(IGNICAO));
}



/**@brief Indica que o pino de DEBUG está jumpeado.
@return 1 se o DEBUG estiver ligado.*/
int1 debug_ativado(void){
	return (!input(DEBUG));
}






/**@brief Desliga ou liga o alarme.
Liga o alarme mas somente aciona a sirene se a ignição estiver acionada.
@param onoff Se for 1, liga o alarme e se for 0 desliga.
*/
void alarme(int1 onoff) {
	if(onoff==1){
		muda_estado(ALARME_DISPARADO);
		output_low(LED);
		output_high(SIRENE);
		if(ignicao_acionada()){
			output_high(RELE);
		}
		else{
			output_low(RELE);
		}
	}
	else{
		muda_estado(ALARME_DESLIGADO);
		output_high(LED);
		output_low(SIRENE);
		output_low(RELE);
	}
}





/**@brief Pisca o LED baseado na  variavel contador, que é incrementada a cada  x ms.
@param tempo Intervalo entre cada piscada do LED.
O tempo é dividido de maneira a dixar o LED menos tempo acesos quando ele estiver piscando para
economizar energia.*/
void pisca_led(int16 tempo){

	//Se o tempo nao for dividido por 2, o LED ficara aceso o tempo determinado, e nao piscando.
	tempo/=4;

	if(cont%(tempo/BASE_TEMPO_CONTADOR)){

		tempo = cont/(tempo/BASE_TEMPO_CONTADOR);

		//Se o modulo der 0, acende o led, caso contrario, deixa apagado (1,2 3).
		//Assim , o duty cicle será de 1/4.
	   if(tempo%4==0){
	      output_high(LED);
	   }
	   else{
	      output_low(LED);
	   }
	}

}







/**@brief Aciona todas as saída do circuito.
Essa função é utilizada no DEBUG do alarme.
@param onoff Indica se as saídas devem ser ligadas ou desligadas. 1 para ligar e 0 para desligar.*/
void aciona_tudo(int1 onoff){
   if(onoff){
		output_high(SIRENE);
		output_high(LED);
		output_high(RELE);
	}
	else{
		output_low(SIRENE);
		output_low(LED);
		output_low(RELE);
	}
}








/**@brief Função principal do programa.\n
Tem as incializações a máquina de estados. */
void main() {
   setup_counters(RTCC_INTERNAL,RTCC_DIV_2);

	#ifndef PIC_505
	port_b_pullups(TRUE);
	#endif

	if(debug_ativado()){
		while(1){
			aciona_tudo(0);
			while(porta_aberta()){
			   aciona_tudo(1);
			}
			while(apertou_chave_secreta()){
			   aciona_tudo(1);
			}
			while(ignicao_acionada()){
			   aciona_tudo(1);
			}
		}
	}


	determina_tempo();
	muda_estado(ALARME_DISPARADO);

	while(1){

		restart_wdt();

		switch(estado){

			case SIRENE_AVISO:
				if(trans){
					trans = 0;
					zera_contador();
					cont = TEMPO_SIRENE_AVISO;
				}
				if(cont){
					output_high(SIRENE);
					output_low(LED);
					cont -= BASE_TEMPO_CONTADOR;
					//Se acabar o tempo, corta o motor.
				}
				else{
					//Liga o alarme e vai para o estado ALARME_DISPARADO
					alarme(1);
					muda_estado(ALARME_DISPARADO);
				}
			break;



			//Indica que acabou o tempo e espera a ignicao ser ligada para acionar o alarme.
			case ALARME_DISPARADO:
				alarme(1);
			break;


			//Indica que acabou o tempo e espera a ignicao ser ligada para acionar o alarme.
			case TEMPO_FINALIZADO:

				cont++;
				pisca_led(500);

				//Se ligar a ignicao, dispara o alarme.
				if(ignicao_acionada()){
					muda_estado(SIRENE_AVISO);
				}

			break;


			//Se o alarme estiver desligado e abrir a porta, começa a contar o tempo.
			case ALARME_DESLIGADO:
				if(porta_aberta()){
					output_low(LED);
					determina_tempo();
					muda_estado(CONTANDO_TEMPO);
 				}
			break;

			//Indica que esta contando o tempo para disparar o alarme.
			case CONTANDO_TEMPO:

				//Se acabar o tempo, dispara o alarme.
				if(cont==0){
					--tempo;
					if(tempo!=0){
						zera_contador();
					}
					else{
						muda_estado(TEMPO_FINALIZADO);
					}
				}
				//Se faltar pouco tempo para disparar, pisca o LED de 1 em 1 segundo.
				else{
					//se estiver nos ultimos x segundos...
					if(cont<(TEMPO_INICIO_PISCAGEM/BASE_TEMPO_CONTADOR)){
						// ... do ultimo minuto...
						if(tempo==1){
							// .. pisca o LED para avisar que o tempo esta acabando.
							pisca_led(1000);
						}
					}
				}

				if(cont)cont--;

			break;

			default:
				muda_estado(ALARME_DISPARADO);
			break;
		}



		//Zera tudo se a chave for apertada em qualquer momento.
      if(apertou_chave_secreta()){
         alarme(0);
      }


		//Faz o delay do circuito.
		delay_ms(BASE_TEMPO_CONTADOR);


	}
}
