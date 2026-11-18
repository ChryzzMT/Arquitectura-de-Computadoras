# Programación Cuántica con Qiskit en Linux

Laboratorios de la guía *Programación Cuántica con Qiskit en Linux Ubuntu*: estado de Bell, rotaciones Ry en la esfera de Bloch, algoritmo de Deutsch y teletransportación. Los circuitos se simulan con Qiskit 1.x y Qiskit Aer.

## Entorno virtual

Ubuntu 23.04 o posterior bloquea `pip` en el Python del sistema (PEP 668). Las dependencias van en un entorno virtual:

```bash
cd Practicas/Qiskit
python3 -m venv .venv
source .venv/bin/activate
python3 -m pip install --upgrade pip
pip install -r requirements.txt
```

Comprobación:

```bash
python3 -c "import qiskit, qiskit_aer; print('Qiskit Core:', qiskit.__version__); print('Aer Simulator OK')"
```

Si `qiskit-aer` no tiene rueda precompilada e intenta compilarse, hacen falta las herramientas del sistema: `sudo apt install -y build-essential python3-dev`.

## Ejecución

Con el entorno activado, desde esta carpeta:

```bash
python3 lab1_hola_mundo.py
python3 lab2_rotaciones_bloch.py
python3 lab3_algoritmo_deutsch.py
python3 lab4_teletransportacion.py
python3 reto1_bell_psi_plus.py
python3 reto2_deutsch_sin_x.py
```

Cada script imprime el resultado en la terminal y guarda el diagrama del circuito y la gráfica en `evidencias/`. No abre ventanas: las figuras se escriben con `savefig` usando el backend Agg.

| Script | Qué comprueba |
| --- | --- |
| `lab1_hola_mundo.py` | Estado de Bell \|Φ⁺⟩ ≈ 50 % `00` y 50 % `11` |
| `lab2_rotaciones_bloch.py` | P(\|0⟩) = cos²(θ/2) al variar Ry(θ) |
| `lab3_algoritmo_deutsch.py` | Constante → 0, balanceada → 1, con una sola consulta |
| `lab4_teletransportacion.py` | Bob recupera Ry(π/3): ≈ 75 % 0 y 25 % 1 |
| `reto1_bell_psi_plus.py` | Estado \|Ψ⁺⟩ ≈ 50 % `01` y 50 % `10` |
| `reto2_deutsch_sin_x.py` | Sin la X de la ancilla el algoritmo deja de distinguir |

El informe de esta corrida está en `INFORME.md`.
