---
source: https://isa262.davinsony.com/concepto/cps-nivel-medio/
title: CPS de nivel medio
---

# CPS de nivel medio

Recuerda algunas caracteristicas de los _Cyber Physical Systems_:
[Diagrama (GraphViz) generado dinámicamente en el sitio original. Fuente DOT literal:]
```dot
layout=neato;
splines=false;
node [shape=box, style=filled, color="#aed1cd", width=1.8, height=0.8];

low [label="Low level - CPS", pos="0,0!"];
mid [label="Mid level - CPS", pos="0,2!"];
high [label="High level - CPS", pos="0,4!"];

low -> mid -> high;

ComplejidadInicio [label=" - complejidad", shape=none, pos="-2,0!", color=none];
ComplejidadFin [label=" + complejidad", shape=none, pos="-2,4!", color=none];
EstrategiaInicio [label=" - estratégico 
(+ operativo)", shape=none, pos="2,0!", color=none];
EstrategiaFin [label=" + estratégico", shape=none, pos="2,4!", color=none];
```
## _CPS_ de nivel bajo.
- Cumple una tarea de regulación
- Desempeña un requerimiento local
- Poco tiempo para la toma de decisiones
- Repite siempre la misma acción
- Tiene métodos muy bien establecidos
## _CPS_ de nivel medio.
- Hace planeación de trayectoria
- Se enfoca en secuencias compleja de acciones simples
- Tiene mas tiempo para la toma de decisiones
- Puede implicar optimización o escoger a partir de un criterio
## Referencias
- [E. A. Lee and S. A. Seshia, Introduction to Embedded Systems - A Cyber-Physical Systems Approach, Second Edition, MIT Press, 2017.](https://ptolemy.berkeley.edu/books/leeseshia/)
