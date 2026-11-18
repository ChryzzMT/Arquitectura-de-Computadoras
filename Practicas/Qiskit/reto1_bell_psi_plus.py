from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from qiskit import QuantumCircuit
from qiskit.visualization import plot_histogram
from qiskit_aer import AerSimulator

# Reto 1: a partir del circuito de lab1, intercalar una Pauli-X
# para preparar |Ψ+⟩ = (|01⟩ + |10⟩)/√2.

# 1. Definición del circuito: 2 cúbits cuánticos y 2 bits clásicos
qc = QuantumCircuit(2, 2)

# 2. Creación de superposición equiprobable en el cúbit 0
qc.h(0)

# 3. Creación de entrelazamiento máximo mediante CNOT
qc.cx(0, 1)

# 4. Pauli-X sobre el cúbit 1: |Φ+⟩ pasa a |Ψ+⟩ = (|01⟩ + |10⟩)/√2
qc.x(1)

# 5. Medición de los cúbits en los registros clásicos
qc.measure([0, 1], [0, 1])

print("CIRCUITO CUÁNTICO:")
print(qc.draw(output="text"))

backend = AerSimulator()
shots = 10000
job = backend.run(qc, shots=shots)
counts = job.result().get_counts(qc)

print(f"\nRESULTADOS DE LA MEDICIÓN ({shots} disparos):")
for estado in ("00", "01", "10", "11"):
    count = counts.get(estado, 0)
    porcentaje = (count / shots) * 100
    print(f"Estado |{estado}⟩: {count} ocurrencias ({porcentaje:.2f}%)")

print("\nCompuerta intercalada: Pauli-X sobre el cúbit 1, después de la CNOT.")

salida = Path(__file__).resolve().parent / "evidencias" / "reto1"
salida.mkdir(parents=True, exist_ok=True)

fig_circ = qc.draw(output="mpl", fold=-1)
if not hasattr(fig_circ, "savefig"):
    fig_circ = plt.gcf()
fig_circ.savefig(salida / "circuito.png", dpi=150, bbox_inches="tight", facecolor="white")
plt.close(fig_circ)

fig_hist = plot_histogram(counts, title="Estado de Bell |Ψ⁺⟩  (10 000 disparos)", figsize=(6.5, 4))
fig_hist.savefig(salida / "histograma.png", dpi=150, bbox_inches="tight", facecolor="white")
plt.close(fig_hist)

print(f"\nFiguras guardadas en: {salida}")
