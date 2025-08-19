# Delta Radio Control

Este repositório contém o firmware para o rádio controle baseado no protocolo ESP-NOW, projetado para uso com aeronaves e veículos de controle remoto de curto alcance e o software de controle de voo DeltaFlight. 

## Calibração do Controle

Na primeira vez que o controle é ligado, será necessário realizar a calibração dos sticks. O processo funciona assim:  

1. Ao ligar o controle, será emitido **um beep**, mova todos os sticks para os **extremos** (quatro cantos).  
2. Quando ouvir o **segundo beep**, posicione todos os sticks no **centro** (incluindo o stick de throttle).  
3. Ao final da calibração, serão emitidos **dois beeps em sequência**, indicando que os valores foram salvos.  

A calibração é armazenada automaticamente na memória flash do ESP32.  

- Se, ao ligar, a calibração for carregada com sucesso, o controle tocará a **melodia do Mario**, indicando que não é necessário calibrar novamente.  

## Compilação

1. Abra o VS Code.
2. Abra o terminal do PlatformIO (na barra inferior do VS Code).
3. Desconecte todos os controladores do computador, deixando conectado apenas aquele em que deseja enviar o código.
4. Execute `pio run -e esp32 --target upload`

## Parear com Delta-FC

Para usar [Delta-FC](https://github.com/Delta-Flight/DeltaFC) como receptor, selecione `SPI Rx (e.g. built-in Rx)` no campo "Receiver mode". O campo "Receiver provider" não faz diferença.

![ESP-FC receiver](/docs/img/espfc_receiver.png)

O transmissor e o receptor se conectam automaticamente após serem ligados, sem necessidade de nenhuma ação. O procedimento de inicialização recomendado é:
1. Ligue primeiro o Rádio Controle (transmissor).
2. Em seguida, ligue o DeltaFC (receptor).
