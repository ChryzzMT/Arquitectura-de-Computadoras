import math
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from qiskit import QuantumCircuit
from qiskit.visualization import plot_histogram
from qiskit_aer import AerSimulator

# Circuito con 3 cúbits y 3 bits clásicos:
# q0 = Cúbit a teletransportar (Alice)
# q1 = Cúbit del par de Bell de Alice
# q2 = Cúbit del par de Bell de Bob
qc = QuantumCircuit(3, 3)

# 1. Preparar un estado cuántico arbitrario en q0 mediante Ry(pi/3)
#    Estado teórico: (sqrt(3)/2)|0> + (1/2)|1> -> P(|0>) = 75%, P(|1>) = 25%
qc.ry(math.pi / 3, 0)
qc.barrier()

# 2. Generar el par entrelazado de Bell entre Alice (q1) y Bob (q2)
qc.h(1)
qc.cx(1, 2)
qc.barrier()

# 3. Operaciones de Alice: Medición en la base de Bell
qc.cx(0, 1)
qc.h(0)
qc.barrier()
qc.measure(0, 0)
qc.measure(1, 1)

# 4. Operaciones de Corrección en Bob condicionadas por los bits recibidos
qc.barrier()
qc.cx(1, 2)  # Si bit 1 es 1 -> aplicar Pauli-X
qc.cz(0, 2)  # Si bit 0 es 1 -> aplicar Pauli-Z

# 5. Medición final del cúbit q2 en Bob para verificar coincidencia
qc.barrier()
qc.measure(2, 2)

print("CIRCUITO CUÁNTICO:")
print(qc.draw(output="text"))

shots = 10000
counts = AerSimulator().run(qc, shots=shots).result().get_counts()

print(f"\nCONTEOS COMPLETOS ({shots} disparos, orden c2 c1 c0):")
for bitstring, n in sorted(counts.items()):
    print(f"  {bitstring}: {n}")

# Contabilizar el resultado medido en el bit más significativo (cúbit de Bob q2):
bob = {"0": 0, "1": 0}
for bitstring, n in counts.items():
    bob[bitstring[0]] += n

print("\nVERIFICACIÓN EN EL CÚBIT DE BOB (q2):")
print(f"P(|0⟩): {bob['0'] / 100:.2f}% (Teórico: 75.00%)")
print(f"P(|1⟩): {bob['1'] / 100:.2f}% (Teórico: 25.00%)")

salida = Path(__file__).resolve().parent / "evidencias" / "lab4"
salida.mkdir(parents=True, exist_ok=True)

fig_circ = qc.draw(output="mpl", fold=-1)
if not hasattr(fig_circ, "savefig"):
    fig_circ = plt.gcf()
fig_circ.savefig(salida / "circuito.png", dpi=150, bbox_inches="tight", facecolor="white")
plt.close(fig_circ)

fig_hist = plot_histogram(
    {"0": bob["0"], "1": bob["1"]},
    title="Cúbit de Bob tras teletransportar Ry(π/3)",
    figsize=(6.2, 4.2),
)
fig_hist.savefig(salida / "histograma.png", dpi=150, bbox_inches="tight", facecolor="white")
plt.close(fig_hist)

print(f"\nFiguras guardadas en: {salida}")
