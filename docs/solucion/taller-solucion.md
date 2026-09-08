---
source: https://isa262.davinsony.com/solucion/taller-solucion/
title: Solución
---

# Solución

Una posible solución:
### Propuesta MEF del esclavo
- **ESLEEP** : Estado del esclavo en espera
- **EWARN** : Estado del esclavo en alerta y emitiendo datos por el maestro o con el maestro
- **ROFF** : Estado del led rojo apagado
- **RON** : Estado del led rojo encendido
[Diagrama (GraphViz) generado dinámicamente en el sitio original. Fuente DOT literal:]
```dot
rankdir = "LR";
node[shape="circle", width = 1.5];
iR[label="",shape=none, width = 0];
R0[label="ROFF

LR=0"];
R1[label="RON

LR=1"];

iR->R0;
R0->R1[label=" (tr >= p) && P
/ tr = 0;"];
R1->R0[label=" (tr >= p) || !P
/ tr = 0;"];

iE[label="",shape=none, width = 0];
ES[label="ESLEEP

P = 0;"];
EW[label="EWARN

P = 1;
readDHT();
post(T,H,B);"];

iE->ES;
ES->EW[label="msg == 'W'"];
EW->ES[label="msg == 'O'"];
```
donde $p$ es la constante de tiempo del parpadeo (`500 ms`), $msg$ es la variable donde se guarda el mensaje que llega del maestro, $tr$ es la variable de tiempo relativo para realizar el parpadeo, $P$ es la variable para controlar una MEF desde la otra, $LR$ es el LED Rojo.
💾 [Código para el esclavo](../../assets/code/arduino/taller/mef_esclavo.ino)
### Propuesta MEF del maestro
- **EOK** : Estado del maestro en estado ok, tomando y emitiendo datos
- **EWARN** : Estado del maestro en alerta, tomando y emitiendo datos
- **ROFF** : Estado del led rojo apagado
- **RON** : Estado del led rojo encendido
[Diagrama (GraphViz) generado dinámicamente en el sitio original. Fuente DOT literal:]
```dot
rankdir = "LR";
node[shape="circle", width = 1.5];
iR[label="",shape=none, width = 0];
R0[label="ROFF

LR=0"];
R1[label="RON

LR=1"];

iR->R0;
R0->R1[label=" (tr >= p) && P
/ tr = 0;"];
R1->R0[label=" (tr >= p) || !P
/ tr = 0;"];

iE[label="",shape=none, width = 0];
ES[label="EOK

LG = 1;
P = 0;
readDHT();
post(T,H,B);"];
EW[label="EWARN

LG = 0;
P = 1;
readDHT();
post(T,H,B);"];

iE->ES;
ES->EW[label="T > Tmax
/ C = 0;
/ te = 0;"];
EW->ES[label="T < Tmin
/ C = 0;
/ te = 0;"];
ES->ES[label="(te >= e) && C<3
/ enviar('O');
/ te = 0;
/ C++;"];
EW->EW[label="(te >= e) && C<3
/ enviar('W');
/ te = 0;
/ C++;"];
```
donde $p$ es la constante de tiempo del parpadeo (`500 ms`), $tr$ es la variable de tiempo relativo para realizar el parpadeo, $P$ es la variable para controlar una MEF desde la otra, $LR$ es el LED Rojo, $LG$ es el LED Verde, $te$ tiempo entre emisiones, $e$ es la constante de tiempo entre emisiones (`3000 ms`), $C$ es el contador del número de emisiones, $Tmax$ temperatura que generá alerta (`30`), $Tmim$ temperatura que regresa el sistema a la normalidad (`27`).
💾 [Código para el maestro](../../assets/code/arduino/taller/mef_maestro.ino)
