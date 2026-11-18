from qiskit import QuantumCircuit
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

print(f"\nESPERADO: |01⟩ y |10⟩ cerca del 50 %; |00⟩ y |11⟩ en 0 %.")
print(f"RESULTADOS DE LA MEDICIÓN ({shots} disparos):")
for estado in ("00", "01", "10", "11"):
    count = counts.get(estado, 0)
    porcentaje = (count / shots) * 100
    print(f"Estado |{estado}⟩: {count} ocurrencias ({porcentaje:.2f}%)")

print()
print("Justificación: la Pauli-X en el cúbit 1, después de la CNOT, cambia")
print("|0⟩ por |1⟩ en q1. (|00⟩+|11⟩)/√2 pasa a (|10⟩+|01⟩)/√2, el estado |Ψ+⟩.")
print("Por eso solo aparecen 01 y 10, cada uno con probabilidad 1/2.")
