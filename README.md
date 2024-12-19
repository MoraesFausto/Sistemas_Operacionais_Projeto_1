# Resolvendo problemas de concorrência
## Descrição do problema
Mestre Yoda está preparando a avaliação (testes) dos novos candidatos (Padawans) a Mestre Jedi. Os testes
ocorrem em um salão especial da ordem Jedi, que possibilita a entrada de um número controlado de público.
Yoda é quem controla o número de candidatos e público no salão. A avaliação obedece a ordem de chegada
no salão. Um Padawan, ao chegar, deve cumprimentar formalmente os Mestres Avaliadores. Quando Yoda
sinaliza o início dos testes, não é mais permitida a entrada no salão. Por outro lado, qualquer espectador
pode sair a qualquer momento. Após os testes, Yoda anuncia o resultado. Se aprovado, cada Padawan terá
sua trança cortada por um Sabre de Luz e poderá sair do salão. Se não aprovado, apenas cumprimentará
Yoda respeitosamente e sairá do salão. Yoda aguardará um tempo para os novos candidatos e público
entrarem para a próxima sessão de avaliação. Após o término de todas as avaliações, Yoda fará um discurso
e finalizará oficialmente todos os testes. Faça uma implementação usando semáforos e outra usando
variáveis de condição e mutex (alternativa a monitores em C) para simular e controlar o comportamento das
entidades: Yoda, Padawans e Público.

## Executar exemplo implementado com semaforo
```shell
cd ex_semaforo && make && ./yoda_semaforo
```
## Executar exemplo implementado com monitor
```shell
cd ex_monitor && make &&./yoda_monitor
```
