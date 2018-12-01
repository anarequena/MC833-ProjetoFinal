# Projeto MC833 - Forca

Simulação do jogo da forca utilizando interação cliente/servidor e
cliente/cliente.

### Compilando

Em anexo com os arquivos do projeto encontra-se disponível um Makefile. Sendo
assim, basta o comando "make" no terminal para que a compilação dos arquivos
seja realizada.

## Testando/Jogando

Como servidor, é necessário rodar o programa do servidor através da linha de
comando "./servidor X", onde X é um valor de porta qualquer arbitrário ao
usuário. Note que este valor é importante para ser repassado aos jogadores a
fim de se encontrarem e estabelecer uma conexão. Depois disso, o servidor serve
apenas para realizar as partidas dos clientes ou fazer o intermédio de
multiplayer entre eles, então basta que o deixemos rodando, sem fazer nada.
Todas as mensagens e ações por parte do servidor já estão programados para
serem realizadas automaticamente, sem a necessidade do usuário fazer algo.  

Como cliente, é necessário rodar o programa do cliente através da linha de
comando "./cliente Y X", onde Y é o IP da máquina do servidor e X é a porta
utilizada pelo servidor. Após rodar, o cliente deve responder as mensagens do
servidor com as opções oferecidas. A aplicação simula um jogo simples de forca.

## Controle de Versões

Está disponível no seguinte repositório
(https://github.com/anarequena/MC833-ProjetoFinal)

## Autores

Ana Carolina Requena Barbosa        RA: 163755
Alex Wei                            RA: 157642

## Conceitos Abordados

* Uso de conexões com protocolo TCP e UDP
* Elaboração de um servidor multiclient
* Clientes capazes de se conectar a servidores e outros clientes
